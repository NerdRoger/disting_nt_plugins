# Directional Sequencer MIDI SysEx API

This document describes the SysEx protocol used by the Directional Sequencer plugin to communicate with an external hardware controller.

The current implementation is intended for a controller with a fixed 8 x 4 grid, but the protocol also includes discovery, registration, snapshots, live cell updates, and playhead state updates.

## Transport

The plugin sends outgoing SysEx to both:

- disting NT breakout MIDI
- USB MIDI

Incoming SysEx is handled by the plugin through the disting NT MIDI SysEx callback.

All message bytes must be 7-bit clean inside the SysEx payload. Multi-byte numeric fields are encoded most-significant 7-bit chunk first.

## SysEx Frame

Every message uses this frame:

```text
f0 7d 41 54 44 53 01 <command> <payload...> f7
```

Fields:

```text
f0                SysEx start
7d                manufacturer ID, non-commercial/development
41 54 44 53       product ID, ASCII "ATDS"
01                protocol version
<command>         command ID
<payload...>      command-specific payload
f7                SysEx end
```

The plugin parser accepts messages with or without the outer `f0` / `f7`, but controllers should send both.

## Numeric Encoding

### U7

Unsigned 7-bit integer.

```text
range: 0..127
bytes: 1
```

### U14

Unsigned 14-bit integer, encoded most-significant 7-bit chunk first.

```text
range: 0..16383
bytes: hi7 lo7
value = (hi7 << 7) | lo7
```

Example, `10000`:

```text
10000 decimal = 0x2710
hi7 = 0x4e
lo7 = 0x10
encoded: 4e 10
```

### S14

Signed 14-bit integer encoded as biased U14.

```text
range: -8192..8191
encoded = value + 8192
bytes: U14(encoded)
```

Example, `+100`:

```text
100 + 8192 = 8292
8292 = 0x2064
encoded: 40 64
```

Example, `-100`:

```text
-100 + 8192 = 8092
encoded: 3f 1c
```

### U21

Unsigned 21-bit integer, encoded most-significant 7-bit chunk first.

```text
range: 0..2097151
bytes: hi7 mid7 lo7
value = (hi7 << 14) | (mid7 << 7) | lo7
```

Used for attribute subscription bitmasks.

### CellValue

Absolute cell values are encoded according to the selected attribute's bounds.

Rules:

```text
if attribute min < 0:      S14
else if attribute max <= 127: U7
else:                      U14
```

Current wide/signed absolute attributes:

```text
Value,    attr 1,  range 0..10000,     U14
MaxDrift, attr 11, range 0..10000,     U14
AccumAdd, attr 12, range -4000..4000,  S14
```

All other current attributes use U7.

`AdjustCell` deltas are always S14, regardless of attribute, because deltas may be negative.

### ValueKind

Snapshots can request either effective display values or stored edit values.

```text
0 = adjusted
1 = base
```

`ChangedCells` reports whichever live value kinds changed, using per-group value flags. `SetCell` and `AdjustCell` always modify base values.

Examples:

```text
Glide attr 3, value 100, U7:      64
Value attr 1, value 10000, U14:   4e 10
AccumAdd attr 12, value -500, S14: 3c 0c
```

## Target Identity

Most commands target one specific Directional Sequencer instance using:

```text
<alg-index:u14> <instance-token:u14>
```

`alg-index` is the current disting NT algorithm index. It can change if algorithms are reordered.

`instance-token` is generated when the plugin instance is first constructed and is persisted in the Directional Sequencer custom serialization. It disambiguates stale advertisements and stale controller state, and it remains stable across Disting NT respecify/reconstruct/deserialize cycles for the same serialized algorithm state.

The controller should not hard-code either value. It should normally learn them from `Advertise` messages. `Registered` and `Identity` responses echo the same target for an already-known instance. If a known token starts advertising again, the previous MIDI registration session should be considered gone and the controller should send `Register` again.

If a command targets the wrong index/token pair, the plugin ignores it.

## Grid And Cell Indexing

The grid is fixed at 8 columns x 4 rows.

Cells are encoded row-major:

```text
cell = y * 8 + x
x = cell % 8
y = cell / 8
```

Valid cell indexes:

```text
0..31
```

`0x7f` is used only in playhead reports to mean "invalid/no current cell".

## Attribute IDs

Attribute IDs match the `CellDataType` enum and the `CellDefinition::All` table.

| ID | Hex | Name | Stored Range | Scaling | Absolute Encoding |
|---:|----:|------|-------------:|--------:|-------------------|
| 0 | 00 | Direction | 0..8 | none | U7 |
| 1 | 01 | Value | 0..10000 | 1000 | U14 |
| 2 | 02 | Velocity | 1..127 | none | U7 |
| 3 | 03 | Glide | 0..100 | none | U7 |
| 4 | 04 | GateLen | 0..100 | none | U7 |
| 5 | 05 | Ratchets | 0..7 | none | U7 |
| 6 | 06 | TieSteps | 0..7 | none | U7 |
| 7 | 07 | Probability | 0..100 | none | U7 |
| 8 | 08 | RestAfter | 0..7 | none | U7 |
| 9 | 09 | Repeats | 0..7 | none | U7 |
| 10 | 0a | DriftProb | 0..100 | none | U7 |
| 11 | 0b | MaxDrift | 0..10000 | 1000 | U14 |
| 12 | 0c | AccumAdd | -4000..4000 | 1000 | S14 |
| 13 | 0d | AccumTimes | 0..7 | none | U7 |
| 14 | 0e | Mute | 0..1 | none | U7 |

Scaled values are stored as integers. For example:

```text
Value 10.000V -> stored 10000
Value 5.000V  -> stored 5000
MaxDrift 1.250V -> stored 1250
AccumAdd -0.500V -> stored -500
```

## Timing And Session Behavior

When no controller is registered, each Directional Sequencer instance advertises periodically.

```text
advertise interval: 2000 ms
```

After registration, the plugin stops periodic advertisements for that instance. Registration state is not serialized; after a respecify/reconstruct/deserialize cycle, the plugin starts unregistered and advertises using the restored `instance-token`.

Once registered, the plugin sends `KeepAlive` to the controller every:

```text
2000 ms
```

The controller should respond with `KeepAliveAck`. If the plugin does not receive the ACK within this timeout measured from the actual `KeepAlive` send time, it clears registration and resumes advertising:

```text
1000 ms
```

The controller should also track plugin keepalives locally. If it does not receive a plugin `KeepAlive` within this interval, it should forget the plugin/session and wait for advertisements before registering again:

```text
3000 ms
```

Outgoing messages are not artificially rate-limited in the current implementation:

```text
minimum spacing: 0 ms
```

If multiple updates are pending, the plugin sends one message per processing interval in priority order.

Current outgoing priority:

1. Error
2. Registered
3. KeepAlive
4. Identity
5. Attribute snapshots
6. Playheads changed
7. Changed cells batches

## Command Summary

| ID | Hex | Direction | Name |
|---:|----:|-----------|------|
| 1 | 01 | plugin -> controller | Advertise |
| 2 | 02 | controller -> plugin | Register |
| 3 | 03 | plugin -> controller | Registered |
| 4 | 04 | plugin -> controller | KeepAlive |
| 5 | 05 | controller -> plugin | Unregister |
| 6 | 06 | plugin -> controller | Error |
| 7 | 07 | controller -> plugin | KeepAliveAck |
| 16 | 10 | controller -> plugin | RequestIdentity |
| 17 | 11 | plugin -> controller | Identity |
| 18 | 12 | controller -> plugin | SubscribeAttrs |
| 19 | 13 | controller -> plugin | RequestSnapshot |
| 20 | 14 | plugin -> controller | AttrSnapshot |
| 21 | 15 | plugin -> controller | ChangedCells |
| 32 | 20 | controller -> plugin | SetCell |
| 33 | 21 | controller -> plugin | AdjustCell |
| 34 | 22 | controller -> plugin | SetInitialCell |
| 35 | 23 | controller -> plugin | RequestPlayheads |
| 36 | 24 | plugin -> controller | PlayheadsChanged |
| 37 | 25 | controller -> plugin | Discover |

## Registration Flow

Recommended controller startup:

1. Listen for `Advertise` messages.
2. Choose a Directional Sequencer instance.
3. Send `Register` with the advertised target.
4. Wait for `Registered`.
5. Send adjusted `RequestSnapshot` for each attribute the controller needs to display.
6. Send `RequestPlayheads`.
7. Send `SubscribeAttrs` for attributes that should produce live updates.
8. Respond to each plugin `KeepAlive` with `KeepAliveAck`.

The controller may also send `Discover` to ask all Directional Sequencer instances to advertise promptly.

## Commands

### Advertise, 0x01

Direction: plugin -> controller

Sent periodically while no controller is registered. `Discover` causes unregistered instances to advertise promptly; registered instances remain quiet.

Payload:

```text
<target> <playhead-count:u7>
```

Full layout:

```text
f0 7d 41 54 44 53 01 01 <alg-index:u14> <instance-token:u14> <playhead-count:u7> f7
```

Example:

```text
f0 7d 41 54 44 53 01 01 00 00 08 6c 01 f7
```

Decoded:

```text
alg index: 0
token: 0x046c / 1132
playheads: 1
```

### Discover, 0x25

Direction: controller -> plugin

Requests Directional Sequencer instances to advertise. In the current implementation, this prompts unregistered instances; registered instances do not advertise while a controller session is active.

Payload: none

```text
f0 7d 41 54 44 53 01 25 f7
```

### Register, 0x02

Direction: controller -> plugin

Registers a controller with one specific plugin instance.

Payload:

```text
<target> <controller-id:u14>
```

Full layout:

```text
f0 7d 41 54 44 53 01 02 <alg-index:u14> <instance-token:u14> <controller-id:u14> f7
```

Example, register controller `1` for advertised target index `0`, token `0x046c`:

```text
f0 7d 41 54 44 53 01 02 00 00 08 6c 00 01 f7
```

On success, plugin queues a `Registered` response.

### Registered, 0x03

Direction: plugin -> controller

Confirms registration.

Payload:

```text
<target> <playhead-count:u7> <attr-count:u7> <protocol-flags:u7>
```

Current `attr-count` is `15` (`0x0f`).

Current `protocol-flags` is `0`.

Example:

```text
f0 7d 41 54 44 53 01 03 00 00 08 6c 01 0f 00 f7
```

### KeepAlive, 0x04

Direction: plugin -> controller

Sent by the plugin every 2000 ms while a controller is registered. The controller should reply with `KeepAliveAck` for the same target.

Payload:

```text
<target>
```

Example:

```text
f0 7d 41 54 44 53 01 04 00 00 08 6c f7
```

If the plugin does not receive `KeepAliveAck` within 1000 ms of sending `KeepAlive`, it clears registration and resumes advertising.

### KeepAliveAck, 0x07

Direction: controller -> plugin

Acknowledges a plugin `KeepAlive`.

Payload:

```text
<target>
```

Example:

```text
f0 7d 41 54 44 53 01 07 00 00 08 6c f7
```

### Unregister, 0x05

Direction: controller -> plugin

Clears the registration session for the targeted plugin instance.

Payload:

```text
<target>
```

Example:

```text
f0 7d 41 54 44 53 01 05 00 00 08 6c f7
```

After unregistering, the plugin resumes advertisements.

### RequestIdentity, 0x10

Direction: controller -> plugin

Requests identity details for a target. This command does not require registration.

Payload:

```text
<target>
```

Example:

```text
f0 7d 41 54 44 53 01 10 00 00 08 6c f7
```

### Identity, 0x11

Direction: plugin -> controller

Same payload shape as `Registered`.

Payload:

```text
<target> <playhead-count:u7> <attr-count:u7> <protocol-flags:u7>
```

Example:

```text
f0 7d 41 54 44 53 01 11 00 00 08 6c 01 0f 00 f7
```

### SubscribeAttrs, 0x12

Direction: controller -> plugin

Sets the live-update subscription mask for cell attributes.

Payload:

```text
<target> <attr-mask:u21>
```

Each bit corresponds to one attribute ID.

`SubscribeAttrs` replaces the full subscription mask. It is not additive. To unsubscribe from an attribute, send a new mask that omits that attribute. To unsubscribe from all attributes, send mask `0`.

Examples:

Subscribe to Value only, attr `1`, mask `1 << 1 = 0x000002`:

```text
f0 7d 41 54 44 53 01 12 00 00 08 6c 00 00 02 f7
```

Switch from Value to Glide. Glide is attr `3`, mask `1 << 3 = 0x000008`. This unsubscribes from Value because the new mask does not include bit `1`:

```text
f0 7d 41 54 44 53 01 12 00 00 08 6c 00 00 08 f7
```

Subscribe to Value and Glide, attrs `1` and `3`, mask `0x00000a`:

```text
f0 7d 41 54 44 53 01 12 00 00 08 6c 00 00 0a f7
```

Unsubscribe from all attribute updates, mask `0`:

```text
f0 7d 41 54 44 53 01 12 00 00 08 6c 00 00 00 f7
```

Subscribe to all current attrs, mask `0x00007fff`:

```text
f0 7d 41 54 44 53 01 12 00 00 08 6c 00 7f 7f f7
```

The plugin masks off unknown/future bits above the current attr count.

### RequestSnapshot, 0x13

Direction: controller -> plugin

Requests all 32 cell values for one attribute and value kind.

Payload:

```text
<target> <attr:u7> <valueKind:u7>
```

Use `valueKind = 0` for adjusted/effective display values. Use `valueKind = 1` for base/stored edit values.

Example, request adjusted Value snapshot:

```text
f0 7d 41 54 44 53 01 13 00 00 08 6c 01 00 f7
```

Example, request base Value snapshot:

```text
f0 7d 41 54 44 53 01 13 00 00 08 6c 01 01 f7
```

### AttrSnapshot, 0x14

Direction: plugin -> controller

Reports all 32 cell values for one attribute and value kind.

Payload:

```text
<target> <attr:u7> <valueKind:u7> <cell-value[0]> ... <cell-value[31]>
```

Each `<cell-value>` uses the attribute's variable-width absolute CellValue encoding: U7, U14, or S14.

Cell order is row-major:

```text
0, 1, 2, 3, 4, 5, 6, 7,
8, 9, 10, 11, 12, 13, 14, 15,
16, 17, 18, 19, 20, 21, 22, 23,
24, 25, 26, 27, 28, 29, 30, 31
```

Full message length for current snapshots is:

```text
8 byte header through command + 4 byte target + 1 attr + 1 valueKind + value bytes + f7. U7 attributes are 47 bytes total; U14/S14 attributes are 79 bytes total.
```

Example prefix for an adjusted Value snapshot:

```text
f0 7d 41 54 44 53 01 14 00 00 08 6c 01 00 ... f7
```

### ChangedCells, 0x15

Direction: plugin -> controller

Reports one or more changed live cell attribute values. Changes are batched on a 42 ms cadence and grouped by attribute to reduce DIN MIDI traffic. Each group declares whether it contains base values, adjusted values, or both.

Generated when:

- a subscribed attribute's base value changes inside the plugin,
- a subscribed attribute's adjusted value changes inside the plugin, including modulation-only changes, or
- the controller sends `SetCell` / `AdjustCell`, in which case the resulting base and adjusted values are echoed even if that attribute is not subscribed.

Repeated changes to the same cell/attribute before a batch is sent collapse to the latest value for each included value kind. Controller-originated `SetCell` / `AdjustCell` commands still receive an authoritative echo, but that echo may arrive in a `ChangedCells` batch with other changed values.

Payload:

```text
<target> <groupCount:u7>

repeated groupCount times:
  <attr:u7> <valueFlags:u7> <cellCount:u7>

  repeated cellCount times:
    <cell:u7>
    if valueFlags & 0x01: <baseValue:CellValue>
    if valueFlags & 0x02: <adjustedValue:CellValue>
```

`valueFlags`:

```text
0x01 = base value included
0x02 = adjusted value included
0x03 = base and adjusted values included
```

`SubscribeAttrs` remains attribute-based. A subscribed attribute can produce base-only, adjusted-only, or base+adjusted `ChangedCells` groups depending on what changed. Every cell record in a group has exactly the value kinds declared by that group's `valueFlags`; if one attribute has a mix of base-only, adjusted-only, and base+adjusted cells, the same `attr` may appear in multiple groups with different `valueFlags`.

Example, one adjusted-only group for Value attr `1` (U14), containing cell `1` with adjusted value `10000`:

```text
f0 7d 41 54 44 53 01 15 00 00 08 6c 01 01 02 01 01 4e 10 f7
```

Example, one base+adjusted group for Value attr `1` (U14), containing cell `1` with base value `5000` and adjusted value `10000`:

```text
f0 7d 41 54 44 53 01 15 00 00 08 6c 01 01 03 01 01 27 08 4e 10 f7
```

Example, one adjusted-only group for Glide attr `3` (U7), containing cell `2` with adjusted value `100`:

```text
f0 7d 41 54 44 53 01 15 00 00 08 6c 01 03 02 01 02 64 f7
```

The plugin uses a 144 byte outgoing buffer and packs as many dirty cell values as fit. If dirty values remain after one batch, they stay queued for a later `ChangedCells` message.

### SetCell, 0x20

Direction: controller -> plugin

Sets one cell attribute to an absolute stored value.

Payload:

```text
<target> <attr:u7> <cell:u7> <value:CellValue>
```

The plugin clamps the value to the attribute's stored min/max before applying it.

Example, set Value attr `1`, cell `1`, to `10000` / 10.000V:

```text
f0 7d 41 54 44 53 01 20 00 00 08 6c 01 01 4e 10 f7
```

Expected echo:

```text
f0 7d 41 54 44 53 01 15 00 00 08 6c 01 01 4e 10 f7
```

Example, set AccumAdd attr `12`, cell `1`, to `-500` / -0.500V:

```text
-500 + 8192 = 7692
7692 encoded S14 = 3c 0c
```

```text
f0 7d 41 54 44 53 01 20 00 00 08 6c 0c 01 3c 0c f7
```

### AdjustCell, 0x21

Direction: controller -> plugin

Adds a signed delta to one cell attribute's current stored value.

Payload:

```text
<target> <attr:u7> <cell:u7> <delta:s14>
```

The delta is always S14. The result is clamped to the attribute's stored min/max.

Example, add `+100` to Value attr `1`, cell `0`:

```text
f0 7d 41 54 44 53 01 21 00 00 08 6c 01 00 40 64 f7
```

Example, add `-100` to Value attr `1`, cell `0`:

```text
f0 7d 41 54 44 53 01 21 00 00 08 6c 01 00 3f 1c f7
```

### SetInitialCell, 0x22

Direction: controller -> plugin

Sets the initial cell for one playhead.

Payload:

```text
<target> <playhead:u7> <cell:u7>
```

Example, set playhead `0` initial cell to cell `9`:

```text
f0 7d 41 54 44 53 01 22 00 00 08 6c 00 09 f7
```

Expected `PlayheadsChanged` response includes initial cell `09`.

### RequestPlayheads, 0x23

Direction: controller -> plugin

Requests current playhead positions and initial cells.

Payload:

```text
<target>
```

Example:

```text
f0 7d 41 54 44 53 01 23 00 00 08 6c f7
```

### PlayheadsChanged, 0x24

Direction: plugin -> controller

Reports all playheads.

Payload:

```text
<target> <record-count:u7> <record...>
```

Each record:

```text
<playhead:u7> <current-cell:u7> <initial-cell:u7>
```

`current-cell` may be `0x7f` to mean invalid/no current cell.

Example:

```text
f0 7d 41 54 44 53 01 24 00 00 08 6c 01 00 7f 09 f7
```

Decoded:

```text
one playhead record
playhead 0
current cell invalid
initial cell 9
```

The plugin sends this when requested or when playhead current/initial cells change.

### Error, 0x06

Direction: plugin -> controller

Reports an error for a targetable command.

Payload:

```text
<target> <original-command:u7> <error-code:u7>
```

Error codes:

| Code | Hex | Name | Meaning |
|----:|----:|------|---------|
| 1 | 01 | BadHeader | Header/version/prefix invalid. Currently not targetable, usually no response. |
| 2 | 02 | UnsupportedVersion | Unsupported protocol version. Currently not targetable, usually no response. |
| 3 | 03 | BadLength | Payload too short or required field missing. |
| 4 | 04 | UnknownCommand | Command ID is not recognized. |
| 5 | 05 | TargetNotFound | Reserved; current implementation generally ignores wrong targets. |
| 6 | 06 | TokenMismatch | Reserved; current implementation generally ignores token mismatch. |
| 7 | 07 | InvalidValue | Attribute, cell, playhead, or value is invalid. |
| 8 | 08 | NotRegistered | Command requires registration and no controller is registered. |
| 9 | 09 | UnsupportedOperation | Reserved. |

Example, requesting invalid attr `0x0f`:

Controller sends:

```text
f0 7d 41 54 44 53 01 13 00 00 08 6c 0f f7
```

Plugin responds:

```text
f0 7d 41 54 44 53 01 06 00 00 08 6c 13 07 f7
```

## Commands That Require Registration

These commands do not require registration:

- `Register`, 0x02
- `KeepAliveAck`, 0x07
- `RequestIdentity`, 0x10

All other target commands require registration.

`Discover` is global and does not use a target.

## Controller State Model

A controller should maintain, per plugin instance:

```text
algIndex
instanceToken
playheadCount
attrCount
registered/unregistered state
subscription mask
32 adjusted values per displayed/subscribed attr
32 base values per displayed/subscribed or edited attr
current and initial cell per playhead
```

Recommended controller behavior:

- Treat `algIndex` and `instanceToken` as dynamic.
- If a new advertisement appears with a different token, consider it a different plugin instance/session.
- If a known token advertises again, treat the previous registration as lost and re-register with that target.
- Ignore `ChangedCells`, `AttrSnapshot`, and `PlayheadsChanged` messages whose target does not match the selected instance.
- After registration, request adjusted snapshots for any attributes the controller displays.
- Use `SubscribeAttrs` for attributes that should update live with base and/or adjusted values from NT-side edits or modulation-side edits.
- Respond to each plugin `KeepAlive` with `KeepAliveAck`; if no plugin `KeepAlive` arrives for 3000 ms, forget the session locally and wait for advertisements.
- Optimistically updating controller UI after sending `SetCell` is optional; the plugin echoes authoritative base and adjusted values in `ChangedCells`.
- For rotary/relative controls, prefer `AdjustCell` so the controller does not need the latest value before sending a change.

## Example Manual Session

Given advertisement:

```text
f0 7d 41 54 44 53 01 01 00 00 08 6c 01 f7
```

Register controller `1`:

```text
f0 7d 41 54 44 53 01 02 00 00 08 6c 00 01 f7
```

Registered response:

```text
f0 7d 41 54 44 53 01 03 00 00 08 6c 01 0f 00 f7
```

Request playheads:

```text
f0 7d 41 54 44 53 01 23 00 00 08 6c f7
```

Subscribe to Value updates:

```text
f0 7d 41 54 44 53 01 12 00 00 08 6c 00 00 02 f7
```

Request adjusted Value snapshot:

```text
f0 7d 41 54 44 53 01 13 00 00 08 6c 01 00 f7
```

Set cell `1` Value to 10.000V:

```text
f0 7d 41 54 44 53 01 20 00 00 08 6c 01 01 4e 10 f7
```

Set playhead `0` initial cell to `9`:

```text
f0 7d 41 54 44 53 01 22 00 00 08 6c 00 09 f7
```

Acknowledge a plugin keepalive:

```text
f0 7d 41 54 44 53 01 07 00 00 08 6c f7
```

Unregister:

```text
f0 7d 41 54 44 53 01 05 00 00 08 6c f7
```

## Implementation Notes And Current Limitations

- The manufacturer ID `0x7d` is for development/non-commercial use.
- The controller should not assume only one Directional Sequencer instance exists.
- The controller should not assume algorithm indexes are stable.
- Grid size is not advertised because this protocol currently assumes fixed 8 x 4 grid hardware.
- Snapshots are per-attribute and per-value-kind to keep DIN MIDI bandwidth manageable.
- The plugin batches changed cells every 42 ms and groups them by attribute and value flags.
- If multiple cells change quickly, updates are queued as bitmasks and drained in `ChangedCells` batches. Multiple changes to the same cell/attribute before it is sent collapse to the latest value for each included value kind.
- Bad headers, unsupported versions, wrong algorithm indexes, and wrong tokens are generally ignored because the plugin cannot always identify a valid target to send an error from.
- The current plugin applies incoming `SetCell`, `AdjustCell`, and `SetInitialCell` from the SysEx callback path and queues outbound confirmation for the processing path.
- Each Directional Sequencer instance tracks one controller session. A new `Register` for the same target replaces the previous controller ID; subsequent commands are not authenticated by controller ID.
- `instance-token` is persisted with custom algorithm data, but MIDI registration state, subscriptions, dirty queues, and pending responses are not persisted.
- `Register`, `KeepAliveAck`, and `RequestIdentity` are accepted without an existing registration, but still require a valid target except for global `Discover`.
