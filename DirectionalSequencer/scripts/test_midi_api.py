#!/usr/bin/env python3
"""
Directional Sequencer MIDI SysEx smoke test.

Requirements:
    pip install mido python-rtmidi

Example:
    python DirectionalSequencer/scripts/test_midi_api.py
    python DirectionalSequencer/scripts/test_midi_api.py --port "disting NT"

The script uses the same Python MIDI mechanism as the Expert Sleepers
push_plugin_to_device.py tool: mido + python-rtmidi, SysEx byte arrays, and
ports selected by name.

It discovers an advertised Directional Sequencer instance, registers, tests the
implemented SysEx API, and restores the cells/playhead initial cell that it
mutates during the test.
"""

import argparse
import queue
import sys
import threading
import time
from dataclasses import dataclass

try:
    import mido
except ImportError as exc:
    raise SystemExit("Missing dependency: pip install mido python-rtmidi") from exc

MANUFACTURER_ID = 0x7D
PRODUCT = [0x41, 0x54, 0x44, 0x53]  # ATDS
VERSION = 0x01
HEADER = [MANUFACTURER_ID, *PRODUCT, VERSION]

CMD_ADVERTISE = 0x01
CMD_REGISTER = 0x02
CMD_REGISTERED = 0x03
CMD_KEEP_ALIVE = 0x04
CMD_UNREGISTER = 0x05
CMD_ERROR = 0x06
CMD_KEEP_ALIVE_ACK = 0x07
CMD_REQUEST_IDENTITY = 0x10
CMD_IDENTITY = 0x11
CMD_SUBSCRIBE_ATTRS = 0x12
CMD_REQUEST_SNAPSHOT = 0x13
CMD_ATTR_SNAPSHOT = 0x14
CMD_CHANGED_CELLS = 0x15
CMD_SET_CELL = 0x20
CMD_ADJUST_CELL = 0x21
CMD_SET_INITIAL_CELL = 0x22
CMD_REQUEST_PLAYHEADS = 0x23
CMD_PLAYHEADS_CHANGED = 0x24
CMD_DISCOVER = 0x25

ERR_INVALID_VALUE = 0x07
ERR_NOT_REGISTERED = 0x08

ATTR_DIRECTION = 0
ATTR_VALUE = 1
ATTR_GLIDE = 3
ATTR_ACCUM_ADD = 12
ATTR_MUTE = 14
ATTR_COUNT = 15
CELL_COUNT = 32
INVALID_CELL = 0x7F
VALUE_KIND_ADJUSTED = 0
VALUE_KIND_BASE = 1
CHANGED_CELLS_INCLUDE_BASE = 0x01
CHANGED_CELLS_INCLUDE_ADJUSTED = 0x02

# Stored bounds, default, and absolute encoding style. These mirror CellDefinition::All.
ATTR_DEFS = {
    0: ("Direction", 0, 8, 0, "u7"),
    1: ("Value", 0, 10000, 5000, "u14"),
    2: ("Velocity", 1, 127, 64, "u7"),
    3: ("Glide", 0, 100, 0, "u7"),
    4: ("Gate Len", 0, 100, 75, "u7"),
    5: ("Ratchets", 0, 7, 0, "u7"),
    6: ("Tie Steps", 0, 7, 0, "u7"),
    7: ("Probability", 0, 100, 100, "u7"),
    8: ("Rest After", 0, 7, 0, "u7"),
    9: ("Repeats", 0, 7, 0, "u7"),
    10: ("Drift Prob", 0, 100, 0, "u7"),
    11: ("Max Drift", 0, 10000, 0, "u14"),
    12: ("Accum Add", -4000, 4000, 0, "s14"),
    13: ("Accum Times", 0, 7, 0, "u7"),
    14: ("Mute", 0, 1, 0, "u7"),
}

INTERACTIVE_ATTR_ORDER = [
    0, 1, 2, 3, 4,
    5, 6, 7, 8, 9,
    10, 11, 12, 13, 14,
]


@dataclass(frozen=True)
class Target:
    alg_index: int
    token: int


@dataclass
class Frame:
    command: int
    payload: list[int]
    raw: list[int]


def hex_bytes(values):
    return " ".join(f"{b:02x}" for b in values)


def u14(value):
    if not 0 <= value <= 0x3FFF:
        raise ValueError(f"U14 out of range: {value}")
    return [(value >> 7) & 0x7F, value & 0x7F]


def read_u14(data, pos):
    return (data[pos] << 7) | data[pos + 1], pos + 2


def s14(value):
    if not -8192 <= value <= 8191:
        raise ValueError(f"S14 out of range: {value}")
    return u14(value + 8192)


def read_s14(data, pos):
    encoded, pos = read_u14(data, pos)
    return encoded - 8192, pos


def u21(value):
    if not 0 <= value <= 0x1FFFFF:
        raise ValueError(f"U21 out of range: {value}")
    return [(value >> 14) & 0x7F, (value >> 7) & 0x7F, value & 0x7F]


def cell_value(attr, value):
    _, min_value, max_value, _, enc = ATTR_DEFS[attr]
    if enc == "s14" or min_value < 0:
        return s14(value)
    if max_value <= 0x7F:
        if not 0 <= value <= 0x7F:
            raise ValueError(f"U7 out of range: {value}")
        return [value & 0x7F]
    return u14(value)


def read_cell_value(attr, data, pos):
    _, min_value, max_value, _, enc = ATTR_DEFS[attr]
    if enc == "s14" or min_value < 0:
        return read_s14(data, pos)
    if max_value <= 0x7F:
        return data[pos], pos + 1
    return read_u14(data, pos)


def target_bytes(target):
    return u14(target.alg_index) + u14(target.token)


def frame_bytes(command, payload=None):
    if payload is None:
        payload = []
    return [0xF0, *HEADER, command, *payload, 0xF7]


def parse_frame(msg):
    raw = msg.bytes()
    if not raw:
        return None
    if raw[0] == 0xF0:
        raw = raw[1:]
    if raw and raw[-1] == 0xF7:
        raw = raw[:-1]
    if len(raw) < len(HEADER) + 1:
        return None
    if raw[: len(HEADER)] != HEADER:
        return None
    return Frame(command=raw[len(HEADER)], payload=raw[len(HEADER) + 1 :], raw=msg.bytes())


def decode_target(payload, pos=0):
    alg_index, pos = read_u14(payload, pos)
    token, pos = read_u14(payload, pos)
    return Target(alg_index, token), pos


def targets_equal(a, b):
    return a.alg_index == b.alg_index and a.token == b.token


class Tester:
    def __init__(self, in_port, out_port, timeout, interactive=False, interactive_timeout=60.0):
        self.in_port = in_port
        self.out_port = out_port
        self.timeout = timeout
        self.interactive = interactive
        self.interactive_timeout = interactive_timeout
        self.target = None
        self.frames = queue.Queue()
        self.reader_stop = threading.Event()
        self.reader_thread = None

    def log(self, message):
        print(message, flush=True)

    def start_reader(self):
        self.reader_thread = threading.Thread(target=self.reader_loop, daemon=True)
        self.reader_thread.start()

    def stop_reader(self):
        self.reader_stop.set()
        if self.reader_thread is not None:
            self.reader_thread.join(timeout=1.0)

    def reader_loop(self):
        while not self.reader_stop.is_set():
            msg = self.in_port.poll()
            if msg is None:
                time.sleep(0.005)
                continue
            if msg.type != "sysex":
                continue
            frame = parse_frame(msg)
            if frame is None:
                continue
            if frame.command == CMD_KEEP_ALIVE and self.is_for_target(frame):
                self.send(CMD_KEEP_ALIVE_ACK, target_bytes(self.target))
                continue
            self.log(f"< {hex_bytes(frame.raw)}")
            self.frames.put(frame)

    def send(self, command, payload=None, label=None):
        arr = frame_bytes(command, payload)
        if label:
            self.log(f"> {label}: {hex_bytes(arr)}")
        self.out_port.send(mido.Message.from_bytes(arr))

    def drain(self, duration=0.15):
        end = time.monotonic() + duration
        while time.monotonic() < end:
            try:
                self.frames.get(timeout=0.01)
            except queue.Empty:
                pass

    def receive_matching(self, predicate, description, timeout=None):
        if timeout is None:
            timeout = self.timeout
        end = time.monotonic() + timeout
        while time.monotonic() < end:
            remaining = max(0.0, end - time.monotonic())
            try:
                frame = self.frames.get(timeout=min(0.05, remaining))
            except queue.Empty:
                continue
            if predicate(frame):
                return frame
        raise TimeoutError(f"Timed out waiting for {description}")

    def discover(self):
        self.log("Testing discovery: requesting advertisements and waiting for Advertise")
        self.send(CMD_DISCOVER, label="Discover")

        def is_advertise(frame):
            return frame.command == CMD_ADVERTISE and len(frame.payload) >= 5

        frame = self.receive_matching(is_advertise, "Advertise", timeout=max(self.timeout, 8.0))
        target, pos = decode_target(frame.payload)
        playheads = frame.payload[pos]
        self.target = target
        self.log(f"  Found Directional Sequencer: algIndex={target.alg_index}, token=0x{target.token:04x}, playheads={playheads}")
        return playheads

    def is_for_target(self, frame):
        if len(frame.payload) < 4 or self.target is None:
            return False
        target, _ = decode_target(frame.payload)
        return targets_equal(target, self.target)

    def wait_command(self, command, description):
        return self.receive_matching(lambda f: f.command == command and self.is_for_target(f), description)

    def register(self):
        self.log("Testing registration")
        payload = target_bytes(self.target) + u14(1)
        self.send(CMD_REGISTER, payload, "Register controller 1")
        frame = self.wait_command(CMD_REGISTERED, "Registered")
        _, pos = decode_target(frame.payload)
        playheads = frame.payload[pos]
        attr_count = frame.payload[pos + 1]
        flags = frame.payload[pos + 2]
        assert attr_count == ATTR_COUNT, f"Expected attr count {ATTR_COUNT}, got {attr_count}"
        self.log(f"  Registered: playheads={playheads}, attrCount={attr_count}, flags={flags}")
        return playheads

    def request_identity(self):
        self.log("Testing RequestIdentity / Identity")
        self.send(CMD_REQUEST_IDENTITY, target_bytes(self.target), "RequestIdentity")
        frame = self.wait_command(CMD_IDENTITY, "Identity")
        _, pos = decode_target(frame.payload)
        self.log(f"  Identity: playheads={frame.payload[pos]}, attrs={frame.payload[pos + 1]}, flags={frame.payload[pos + 2]}")

    def request_playheads(self):
        self.log("Testing RequestPlayheads / PlayheadsChanged")
        self.send(CMD_REQUEST_PLAYHEADS, target_bytes(self.target), "RequestPlayheads")
        frame = self.wait_command(CMD_PLAYHEADS_CHANGED, "PlayheadsChanged")
        return self.parse_playheads(frame)

    def parse_playheads(self, frame):
        _, pos = decode_target(frame.payload)
        count = frame.payload[pos]
        pos += 1
        records = []
        for _ in range(count):
            playhead = frame.payload[pos]
            current_cell = frame.payload[pos + 1]
            initial_cell = frame.payload[pos + 2]
            pos += 3
            records.append((playhead, current_cell, initial_cell))
        self.log(f"  Playheads: {records}")
        return records

    def request_snapshot(self, attr, value_kind=VALUE_KIND_ADJUSTED):
        name = ATTR_DEFS[attr][0]
        kind_name = "adjusted" if value_kind == VALUE_KIND_ADJUSTED else "base"
        self.log(f"Testing RequestSnapshot / AttrSnapshot for {name} attr {attr} ({kind_name})")
        self.send(CMD_REQUEST_SNAPSHOT, target_bytes(self.target) + [attr, value_kind], f"RequestSnapshot {name} {kind_name}")
        frame = self.wait_command(CMD_ATTR_SNAPSHOT, f"AttrSnapshot {name} {kind_name}")
        _, pos = decode_target(frame.payload)
        got_attr = frame.payload[pos]
        got_kind = frame.payload[pos + 1]
        pos += 2
        assert got_attr == attr, f"Expected snapshot attr {attr}, got {got_attr}"
        assert got_kind == value_kind, f"Expected snapshot valueKind {value_kind}, got {got_kind}"
        values = []
        for _ in range(CELL_COUNT):
            value, pos = read_cell_value(attr, frame.payload, pos)
            values.append(value)
        assert len(values) == CELL_COUNT
        self.log(f"  Snapshot {name} {kind_name}: first 8 values {values[:8]}")
        return values

    def subscribe(self, mask, label):
        self.log(f"Testing SubscribeAttrs: {label}, mask=0x{mask:05x}")
        self.send(CMD_SUBSCRIBE_ATTRS, target_bytes(self.target) + u21(mask), f"SubscribeAttrs {label}")
        time.sleep(0.05)

    def parse_changed_cells(self, frame):
        _, pos = decode_target(frame.payload)
        group_count = frame.payload[pos]
        pos += 1
        records = []
        for _ in range(group_count):
            attr = frame.payload[pos]
            value_flags = frame.payload[pos + 1]
            cell_count = frame.payload[pos + 2]
            pos += 3
            for _ in range(cell_count):
                cell = frame.payload[pos]
                pos += 1
                if value_flags & CHANGED_CELLS_INCLUDE_BASE:
                    value, pos = read_cell_value(attr, frame.payload, pos)
                    records.append((attr, cell, VALUE_KIND_BASE, value))
                if value_flags & CHANGED_CELLS_INCLUDE_ADJUSTED:
                    value, pos = read_cell_value(attr, frame.payload, pos)
                    records.append((attr, cell, VALUE_KIND_ADJUSTED, value))
        return records

    def wait_changed_cell(self, attr, cell, expected_value, description):
        def matches(frame):
            if frame.command != CMD_CHANGED_CELLS or not self.is_for_target(frame):
                return False
            return any(got_attr == attr and got_cell == cell and value == expected_value for got_attr, got_cell, _, value in self.parse_changed_cells(frame))

        frame = self.receive_matching(matches, description)
        records = self.parse_changed_cells(frame)
        self.log(f"  ChangedCells verified: attr={attr}, cell={cell}, value={expected_value}, records={records}")
        return frame

    def wait_any_changed_cell(self, allowed_attrs, description, timeout=None):
        def matches(frame):
            if frame.command != CMD_CHANGED_CELLS or not self.is_for_target(frame):
                return False
            return any(attr in allowed_attrs for attr, _, _, _ in self.parse_changed_cells(frame))

        frame = self.receive_matching(matches, description, timeout=timeout)
        records = self.parse_changed_cells(frame)
        for attr, cell, kind, value in records:
            if attr in allowed_attrs:
                name = ATTR_DEFS[attr][0]
                kind_name = "adjusted" if kind == VALUE_KIND_ADJUSTED else "base"
                self.log(f"  Live ChangedCells received: attr={attr} ({name}), cell={cell}, kind={kind_name}, value={value}, records={records}")
                return attr, cell, value
        raise AssertionError("Matching ChangedCells record was not found")

    def wait_cell31_boundary_change(self, attr, expected_value, label):
        name = ATTR_DEFS[attr][0]

        def matches(frame):
            if frame.command != CMD_CHANGED_CELLS or not self.is_for_target(frame):
                return False
            return any(got_attr == attr and got_cell == 31 and value == expected_value for got_attr, got_cell, _, value in self.parse_changed_cells(frame))

        frame = self.receive_matching(matches, f"interactive {name} cell 31 {label}", timeout=self.interactive_timeout)
        records = self.parse_changed_cells(frame)
        self.log(f"  Verified {name} cell 31 {label}: {expected_value}, records={records}")

    def set_cell(self, attr, cell, value):
        name = ATTR_DEFS[attr][0]
        self.log(f"Testing SetCell {name} cell {cell} -> {value}")
        self.send(CMD_SET_CELL, target_bytes(self.target) + [attr, cell] + cell_value(attr, value), f"SetCell {name}")
        self.wait_changed_cell(attr, cell, value, f"ChangedCells {name}={value}")

    def adjust_cell(self, attr, cell, delta, expected):
        name = ATTR_DEFS[attr][0]
        self.log(f"Testing AdjustCell {name} cell {cell} by {delta}")
        self.send(CMD_ADJUST_CELL, target_bytes(self.target) + [attr, cell] + s14(delta), f"AdjustCell {name}")
        self.wait_changed_cell(attr, cell, expected, f"ChangedCells {name} adjusted to {expected}")

    def wait_playhead_initial(self, playhead, expected_cell=None, description="PlayheadsChanged", timeout=None):
        def matches(frame):
            if frame.command != CMD_PLAYHEADS_CHANGED or not self.is_for_target(frame):
                return False
            _, pos = decode_target(frame.payload)
            count = frame.payload[pos]
            pos += 1
            for _ in range(count):
                ph = frame.payload[pos]
                initial = frame.payload[pos + 2]
                pos += 3
                if ph == playhead and (expected_cell is None or initial == expected_cell):
                    return True
            return False

        frame = self.receive_matching(matches, description, timeout=timeout)
        records = self.parse_playheads(frame)
        for ph, _, initial in records:
            if ph == playhead:
                self.log(f"  Playhead {playhead} initial cell update received: {initial}")
                return initial
        raise AssertionError(f"Playhead {playhead} not found in PlayheadsChanged")

    def set_initial_cell(self, playhead, cell):
        self.log(f"Testing SetInitialCell playhead {playhead} -> cell {cell}")
        self.send(CMD_SET_INITIAL_CELL, target_bytes(self.target) + [playhead, cell], "SetInitialCell")
        self.wait_playhead_initial(playhead, cell, "PlayheadsChanged with requested initial cell")
        self.log("  Initial cell update verified")

    def expect_error(self, sent_command, expected_error, send_command, payload, label):
        self.log(f"Testing error path: {label}")
        self.send(send_command, payload, label)

        def matches(frame):
            if frame.command != CMD_ERROR or not self.is_for_target(frame):
                return False
            _, pos = decode_target(frame.payload)
            return frame.payload[pos] == sent_command and frame.payload[pos + 1] == expected_error

        self.receive_matching(matches, f"Error {expected_error} for command {sent_command}")
        self.log("  Error response verified")

    def unregister(self):
        self.log("Testing Unregister")
        self.send(CMD_UNREGISTER, target_bytes(self.target), "Unregister")
        time.sleep(0.1)

    def interactive_checks(self, playheads):
        if not self.interactive:
            return

        self.log("")
        self.log("Interactive NT-side tests")
        self.log("These prompts verify updates caused directly on the disting NT UI.")
        self.log("For each attribute, set cell 31 to max, min, then default when prompted.")

        for attr in INTERACTIVE_ATTR_ORDER:
            name, min_value, max_value, default_value, _ = ATTR_DEFS[attr]
            self.subscribe(1 << attr, f"interactive {name} only")

            for label, expected in (("maximum", max_value), ("minimum", min_value), ("default", default_value)):
                input(f"Press Enter, then set cell 31 {name} to its {label} stored value ({expected}) on the disting NT UI...")
                self.wait_cell31_boundary_change(attr, expected, label)

        self.subscribe(0, "interactive none, unsubscribe all")

        if playheads > 0:
            input("Press Enter, then change playhead 0's initial cell on the disting NT UI...")
            self.wait_playhead_initial(0, None, "interactive PlayheadsChanged", timeout=self.interactive_timeout)

        self.log("Interactive NT-side tests passed")

    def run(self):
        self.start_reader()
        try:
            self.drain()
            playheads = self.discover()
            self.register()
            self.request_identity()
            original_playheads = self.request_playheads()

            original_value = self.request_snapshot(ATTR_VALUE, VALUE_KIND_BASE)
            original_glide = self.request_snapshot(ATTR_GLIDE, VALUE_KIND_BASE)
            original_accum = self.request_snapshot(ATTR_ACCUM_ADD, VALUE_KIND_BASE)

            original_initial = None
            if original_playheads:
                original_initial = original_playheads[0][2]
                if original_initial == INVALID_CELL:
                    original_initial = 0

            try:
                adjusted_value_snapshot = self.request_snapshot(ATTR_VALUE, VALUE_KIND_ADJUSTED)
                base_value_snapshot = self.request_snapshot(ATTR_VALUE, VALUE_KIND_BASE)
                assert len(adjusted_value_snapshot) == CELL_COUNT and len(base_value_snapshot) == CELL_COUNT

                self.subscribe(1 << ATTR_VALUE, "Value only")
                self.set_cell(ATTR_VALUE, 1, 10000)
                snapshot = self.request_snapshot(ATTR_VALUE, VALUE_KIND_BASE)
                assert snapshot[1] == 10000, f"Snapshot did not reflect Value cell 1 = 10000: {snapshot[1]}"
                self.adjust_cell(ATTR_VALUE, 1, -100, 9900)
                snapshot = self.request_snapshot(ATTR_VALUE, VALUE_KIND_BASE)
                assert snapshot[1] == 9900, f"Snapshot did not reflect Value cell 1 = 9900: {snapshot[1]}"

                self.subscribe(1 << ATTR_GLIDE, "Glide only, replacing Value subscription")
                self.set_cell(ATTR_GLIDE, 2, 77)
                snapshot = self.request_snapshot(ATTR_GLIDE, VALUE_KIND_BASE)
                assert snapshot[2] == 77, f"Snapshot did not reflect Glide cell 2 = 77: {snapshot[2]}"

                self.subscribe((1 << ATTR_VALUE) | (1 << ATTR_GLIDE), "Value + Glide")
                self.set_cell(ATTR_VALUE, 1, 5000)
                self.set_cell(ATTR_GLIDE, 2, 12)

                self.set_cell(ATTR_ACCUM_ADD, 3, -500)
                snapshot = self.request_snapshot(ATTR_ACCUM_ADD, VALUE_KIND_BASE)
                assert snapshot[3] == -500, f"Snapshot did not reflect AccumAdd cell 3 = -500: {snapshot[3]}"
                self.adjust_cell(ATTR_ACCUM_ADD, 3, 250, -250)
                snapshot = self.request_snapshot(ATTR_ACCUM_ADD, VALUE_KIND_BASE)
                assert snapshot[3] == -250, f"Snapshot did not reflect AccumAdd cell 3 = -250: {snapshot[3]}"

                if playheads > 0:
                    self.set_initial_cell(0, 9)
                    records = self.request_playheads()
                    assert records and records[0][2] == 9, "RequestPlayheads did not return initial cell 9"

                self.expect_error(
                    sent_command=CMD_REQUEST_SNAPSHOT,
                    expected_error=ERR_INVALID_VALUE,
                    send_command=CMD_REQUEST_SNAPSHOT,
                    payload=target_bytes(self.target) + [0x0F, VALUE_KIND_ADJUSTED],
                    label="RequestSnapshot invalid attr 0x0f",
                )

                self.subscribe(0, "none, unsubscribe all")
                self.interactive_checks(playheads)
            finally:
                self.log("Restoring mutated values")
                self.set_cell(ATTR_VALUE, 1, original_value[1])
                self.set_cell(ATTR_GLIDE, 2, original_glide[2])
                self.set_cell(ATTR_ACCUM_ADD, 3, original_accum[3])
                if playheads > 0 and original_initial is not None:
                    self.set_initial_cell(0, original_initial)
                self.unregister()

            self.log("All MIDI API smoke tests passed")
        finally:
            self.stop_reader()


def choose_port(names, requested, kind):
    if requested:
        matches = [name for name in names if requested in name]
        if not matches:
            raise SystemExit(f"No {kind} port containing {requested!r}. Available: {names}")
        return matches[0]
    matches = [name for name in names if "disting NT" in name]
    if not matches:
        raise SystemExit(f"No {kind} port containing 'disting NT'. Available: {names}")
    return matches[0]


def main(argv):
    parser = argparse.ArgumentParser(description="Smoke-test Directional Sequencer MIDI SysEx API")
    parser.add_argument("--port", help="substring to match for both input and output MIDI ports; defaults to 'disting NT'")
    parser.add_argument("--input", help="substring to match input MIDI port")
    parser.add_argument("--output", help="substring to match output MIDI port")
    parser.add_argument("--timeout", type=float, default=5.0, help="seconds to wait for each expected response")
    parser.add_argument("--interactive", action="store_true", help="prompt for manual NT-side edits and verify live updates")
    parser.add_argument("--interactive-timeout", type=float, default=60.0, help="seconds to wait for each interactive NT-side update")
    parser.add_argument("--list-ports", action="store_true", help="list MIDI ports and exit")
    args = parser.parse_args(argv)

    input_names = mido.get_input_names()
    output_names = mido.get_output_names()

    if args.list_ports:
        print("Input ports:")
        for name in input_names:
            print(f"  {name}")
        print("Output ports:")
        for name in output_names:
            print(f"  {name}")
        return 0

    in_match = args.input or args.port
    out_match = args.output or args.port
    in_name = choose_port(input_names, in_match, "input")
    out_name = choose_port(output_names, out_match, "output")

    print(f"Opening input:  {in_name}")
    print(f"Opening output: {out_name}")

    with mido.open_input(in_name) as in_port, mido.open_output(out_name) as out_port:
        Tester(in_port, out_port, args.timeout, interactive=args.interactive, interactive_timeout=args.interactive_timeout).run()

    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
