#include <distingnt/slot.h>
#include "ntCommon.h"
#include "dirSeqAlg.h"
#include "dirSeqMidiController.h"


namespace DirSeqMidi {
	static constexpr uint8_t ManufacturerId = 0x7D;
	static constexpr uint8_t ProtocolVersion = 0x01;
	static constexpr uint8_t Product0 = 'A';
	static constexpr uint8_t Product1 = 'T';
	static constexpr uint8_t Product2 = 'D';
	static constexpr uint8_t Product3 = 'S';
	static constexpr uint32_t AdvertiseIntervalMs = 2000;
	static constexpr uint32_t KeepAliveIntervalMs = 2000;
	static constexpr uint32_t KeepAliveAckTimeoutMs = 1000;
	static constexpr uint32_t SendIntervalMs = 0;
	static constexpr uint32_t ChangedCellsIntervalMs = 42;
	static constexpr uint32_t WriterCapacity = 144;
	static constexpr uint8_t CellCount = GridSizeX * GridSizeY;

	enum ValueKind : uint8_t {
		Adjusted = 0,
		Base = 1,
		ValueKindCount = 2,
	};

	static constexpr uint8_t ChangedCellsIncludeBase = 0x01;
	static constexpr uint8_t ChangedCellsIncludeAdjusted = 0x02;

	enum Command : uint8_t {
		Advertise        = 0x01,
		Register         = 0x02,
		Registered       = 0x03,
		KeepAlive        = 0x04,
		Unregister       = 0x05,
		Error            = 0x06,
		KeepAliveAck     = 0x07,
		RequestIdentity  = 0x10,
		Identity         = 0x11,
		SubscribeAttrs   = 0x12,
		RequestSnapshot  = 0x13,
		AttrSnapshot     = 0x14,
		ChangedCells     = 0x15,
		SetCell          = 0x20,
		AdjustCell       = 0x21,
		SetInitialCell   = 0x22,
		RequestPlayheads = 0x23,
		PlayheadsChanged = 0x24,
		Discover         = 0x25,
	};

	enum ErrorCode : uint8_t {
		BadHeader            = 1,
		UnsupportedVersion   = 2,
		BadLength            = 3,
		UnknownCommand       = 4,
		TargetNotFound       = 5,
		TokenMismatch        = 6,
		InvalidValue         = 7,
		NotRegistered        = 8,
		UnsupportedOperation = 9,
	};

	struct Reader {
		const uint8_t* Data;
		uint32_t Count;
		uint32_t Pos;

		bool ReadU7(uint8_t& value) {
			if (Pos >= Count || Data[Pos] > 0x7F) {
				return false;
			}
			value = Data[Pos++];
			return true;
		}

		bool ReadU14(uint16_t& value) {
			uint8_t hi;
			uint8_t lo;
			if (!ReadU7(hi) || !ReadU7(lo)) {
				return false;
			}
			value = (hi << 7) | lo;
			return true;
		}

		bool ReadS14(int16_t& value) {
			uint16_t encoded;
			if (!ReadU14(encoded)) {
				return false;
			}
			value = static_cast<int16_t>(encoded) - 8192;
			return true;
		}

		bool ReadCellValue(CellDataType ct, int16_t& value) {
			auto& cd = CellDefinition::All[static_cast<size_t>(ct)];
			if (cd.Min < 0) {
				return ReadS14(value);
			}
			if (cd.Max <= 0x7F) {
				uint8_t encoded;
				if (!ReadU7(encoded)) {
					return false;
				}
				value = encoded;
				return true;
			}
			uint16_t encoded;
			if (!ReadU14(encoded)) {
				return false;
			}
			value = static_cast<int16_t>(encoded);
			return true;
		}

		bool ReadU21(uint32_t& value) {
			uint8_t b0;
			uint8_t b1;
			uint8_t b2;
			if (!ReadU7(b0) || !ReadU7(b1) || !ReadU7(b2)) {
				return false;
			}
			value = (b0 << 14) | (b1 << 7) | b2;
			return true;
		}
	};

	struct Writer {
		uint8_t Data[WriterCapacity];
		uint32_t Len;

		void Begin(uint8_t cmd) {
			Len = 0;
			WriteRaw(0xF0);
			WriteRaw(ManufacturerId);
			WriteRaw(Product0);
			WriteRaw(Product1);
			WriteRaw(Product2);
			WriteRaw(Product3);
			WriteRaw(ProtocolVersion);
			WriteU7(cmd);
		}

		bool CanWrite(uint32_t count) const {
			return Len + count <= WriterCapacity;
		}

		void WriteRaw(uint8_t value) {
			Data[Len++] = value;
		}

		void WriteU7(uint8_t value) {
			Data[Len++] = value & 0x7F;
		}

		void WriteU14(uint16_t value) {
			WriteU7((value >> 7) & 0x7F);
			WriteU7(value & 0x7F);
		}

		void WriteS14(int16_t value) {
			WriteU14(static_cast<uint16_t>(value + 8192));
		}

		void WriteCellValue(CellDataType ct, int16_t value) {
			auto& cd = CellDefinition::All[static_cast<size_t>(ct)];
			if (cd.Min < 0) {
				WriteS14(value);
			} else if (cd.Max <= 0x7F) {
				WriteU7(static_cast<uint8_t>(value));
			} else {
				WriteU14(static_cast<uint16_t>(value));
			}
		}

		void WriteTarget(DirSeqAlg& alg) {
			auto algIndex = NT_algorithmIndex(&alg);
			WriteU14(static_cast<uint16_t>(algIndex < 0 ? 0 : algIndex));
			WriteU14(alg.Midi.InstanceToken());
		}

		void Send() {
			NT_sendMidiSysEx(kNT_destinationBreakout | kNT_destinationUSB, Data, Len, true);
//			NT_sendMidiSysEx(kNT_destinationBreakout, Data, Len, true);
		}
	};

	static uint8_t CellIndex(uint8_t x, uint8_t y) {
		return y * GridSizeX + x;
	}

	static uint8_t CellIndex(const CellCoords& coords) {
		if (coords.x < 0 || coords.y < 0 || coords.x >= GridSizeX || coords.y >= GridSizeY) {
			return DirSeqMidiController::InvalidCell;
		}
		return CellIndex(coords.x, coords.y);
	}

	static bool CellCoordsFromIndex(uint8_t cell, CellCoords& coords) {
		if (cell >= CellCount) {
			return false;
		}
		coords.x = cell % GridSizeX;
		coords.y = cell / GridSizeX;
		return true;
	}

	static int16_t StoredBaseCellValue(DirSeqAlg& alg, uint8_t x, uint8_t y, CellDataType ct) {
		auto& cd = CellDefinition::All[static_cast<size_t>(ct)];
		return cd.CellValueToCellStorage(alg.StepData.GetBaseCellValue(x, y, ct));
	}

	static int16_t StoredAdjustedCellValue(DirSeqAlg& alg, uint8_t x, uint8_t y, CellDataType ct) {
		auto& cd = CellDefinition::All[static_cast<size_t>(ct)];
		return cd.CellValueToCellStorage(alg.StepData.GetAdjustedCellValue(x, y, ct));
	}

	static int16_t StoredCellValue(DirSeqAlg& alg, uint8_t x, uint8_t y, CellDataType ct, uint8_t valueKind) {
		return valueKind == Base ? StoredBaseCellValue(alg, x, y, ct) : StoredAdjustedCellValue(alg, x, y, ct);
	}

	static uint32_t CellValueWidth(CellDataType ct) {
		auto& cd = CellDefinition::All[static_cast<size_t>(ct)];
		return (cd.Min < 0 || cd.Max > 0x7F) ? 2 : 1;
	}

	static bool DecodeFrame(const uint8_t* message, uint32_t count, Reader& reader, uint8_t& command) {
		if (message == nullptr || count < 7) {
			return false;
		}

		uint32_t start = 0;
		uint32_t end = count;
		if (message[start] == 0xF0) {
			start++;
		}
		if (end > start && message[end - 1] == 0xF7) {
			end--;
		}
		if (end - start < 7) {
			return false;
		}
		if (message[start] != ManufacturerId ||
			message[start + 1] != Product0 ||
			message[start + 2] != Product1 ||
			message[start + 3] != Product2 ||
			message[start + 4] != Product3 ||
			message[start + 5] != ProtocolVersion) {
			return false;
		}

		command = message[start + 6];
		reader = { .Data = message, .Count = end, .Pos = start + 7 };
		return true;
	}

	static DirSeqAlg* FindTarget(uint16_t algIndex, uint16_t token) {
		_NT_slot slot;
		if (!NT_getSlot(slot, algIndex) || slot.guid() != DirSeqAlg::Guid) {
			return nullptr;
		}
		auto alg = static_cast<DirSeqAlg*>(slot.plugin());
		if (alg == nullptr || alg->Midi.InstanceToken() != token) {
			return nullptr;
		}
		return alg;
	}

	static void MarkAllSequencersForAdvertise() {
		_NT_slot slot;
		for (uint32_t idx = 0; idx < NT_algorithmCount(); idx++) {
			if (!NT_getSlot(slot, idx) || slot.guid() != DirSeqAlg::Guid) {
				continue;
			}
			auto alg = static_cast<DirSeqAlg*>(slot.plugin());
			if (alg != nullptr) {
				alg->Midi.RequestAdvertise();
			}
		}
	}

	static bool ReadTarget(Reader& reader, uint16_t& algIndex, uint16_t& token) {
		return reader.ReadU14(algIndex) && reader.ReadU14(token);
	}

	static bool RequiresRegistration(uint8_t cmd) {
		return cmd != Register && cmd != RequestIdentity && cmd != KeepAliveAck;
	}
}


void DirSeqMidiController::Init(DirSeqAlg& alg) {
	if (instanceToken == 0) {
		SetInstanceToken(static_cast<uint16_t>((NT_getCpuCycleCount() ^ (reinterpret_cast<uintptr_t>(&alg) >> 2)) & 0x3FFF));
	}
	for (int h = 0; h < alg.Playheads.Count; h++) {
		lastCurrentCells[h] = alg.Playheads[h].CurrentStep;
		lastInitialCells[h] = alg.Playheads[h].InitialStep;
	}
}


void DirSeqMidiController::SetInstanceToken(uint16_t token) {
	token &= 0x3FFF;
	if (token == 0) {
		token = 1;
	}
	instanceToken = token;
}


void DirSeqMidiController::MarkCellChanged(DirSeqAlg&, uint8_t x, uint8_t y, CellDataType ct, bool force) {
	if (!controllerRegistered) {
		return;
	}

	auto attr = static_cast<size_t>(ct);
	if (attr >= AttrCount) {
		return;
	}
	if (!force && ((subscribedAttrMask & (1u << attr)) == 0)) {
		return;
	}

	auto cellMask = 1u << DirSeqMidi::CellIndex(x, y);
	dirtyBaseCellMasks[attr] |= cellMask;
	dirtyAdjustedCellMasks[attr] |= cellMask;
}


void DirSeqMidiController::MarkPlayheadsDirty() {
	if (controllerRegistered) {
		pendingPlayheads = true;
	}
}


void DirSeqMidiController::CheckPlayheadChanges(DirSeqAlg& alg) {
	for (int h = 0; h < alg.Playheads.Count; h++) {
		if (alg.Playheads[h].CurrentStep != lastCurrentCells[h] || alg.Playheads[h].InitialStep != lastInitialCells[h]) {
			MarkPlayheadsDirty();
		}
	}
}


void DirSeqMidiController::QueueError(uint8_t originalCommand, uint8_t code) {
	pendingError = true;
	errorCommand = originalCommand;
	errorCode = code;
}


void DirSeqMidiController::ClearRegistration() {
	controllerRegistered = false;
	subscribedAttrMask = 0;
	pendingRegistered = false;
	pendingIdentity = false;
	pendingPlayheads = false;
	keepAliveAwaitingAck = false;
	lastKeepAliveSentMs = 0;
	for (size_t kind = 0; kind < DirSeqMidi::ValueKindCount; kind++) {
		snapshotRequestMasks[kind] = 0;
	}
	for (size_t i = 0; i < AttrCount; i++) {
		dirtyBaseCellMasks[i] = 0;
		dirtyAdjustedCellMasks[i] = 0;
		adjustedCacheValidMasks[i] = 0;
	}
}


void DirSeqMidiController::SendAdvertise(DirSeqAlg& alg) {
	DirSeqMidi::Writer writer;
	writer.Begin(DirSeqMidi::Advertise);
	writer.WriteTarget(alg);
	writer.WriteU7(alg.Playheads.Count);
	writer.Send();
	lastAdvertiseMs = alg.Timer.TotalMs;
	lastSendMs = alg.Timer.TotalMs;
	pendingAdvertise = false;
}


void DirSeqMidiController::SendRegistered(DirSeqAlg& alg, uint8_t command) {
	DirSeqMidi::Writer writer;
	writer.Begin(command);
	writer.WriteTarget(alg);
	writer.WriteU7(alg.Playheads.Count);
	writer.WriteU7(static_cast<uint8_t>(CellDataType::NumCellDataTypes));
	writer.WriteU7(0);
	writer.Send();
	lastSendMs = alg.Timer.TotalMs;
}


void DirSeqMidiController::SendError(DirSeqAlg& alg) {
	DirSeqMidi::Writer writer;
	writer.Begin(DirSeqMidi::Error);
	writer.WriteTarget(alg);
	writer.WriteU7(errorCommand);
	writer.WriteU7(errorCode);
	writer.Send();
	lastSendMs = alg.Timer.TotalMs;
	pendingError = false;
}


void DirSeqMidiController::SendKeepAlive(DirSeqAlg& alg) {
	DirSeqMidi::Writer writer;
	writer.Begin(DirSeqMidi::KeepAlive);
	writer.WriteTarget(alg);
	writer.Send();
	lastSendMs = alg.Timer.TotalMs;
	lastKeepAliveSentMs = alg.Timer.TotalMs;
	keepAliveAwaitingAck = true;
}


void DirSeqMidiController::SendSnapshot(DirSeqAlg& alg, uint8_t attr, uint8_t valueKind) {
	DirSeqMidi::Writer writer;
	writer.Begin(DirSeqMidi::AttrSnapshot);
	writer.WriteTarget(alg);
	writer.WriteU7(attr);
	writer.WriteU7(valueKind);
	auto ct = static_cast<CellDataType>(attr);
	uint32_t sentMask = 0;
	for (uint8_t y = 0; y < GridSizeY; y++) {
		for (uint8_t x = 0; x < GridSizeX; x++) {
			auto cell = DirSeqMidi::CellIndex(x, y);
			auto value = DirSeqMidi::StoredCellValue(alg, x, y, ct, valueKind);
			writer.WriteCellValue(ct, value);
			if (valueKind == DirSeqMidi::Adjusted) {
				lastAdjustedCellValues[attr][cell] = value;
				sentMask |= 1u << cell;
			}
		}
	}
	if (valueKind == DirSeqMidi::Adjusted) {
		adjustedCacheValidMasks[attr] |= sentMask;
	}
	writer.Send();
	lastSendMs = alg.Timer.TotalMs;
	snapshotRequestMasks[valueKind] &= ~(1u << attr);
}


void DirSeqMidiController::CheckAdjustedCellChanges(DirSeqAlg& alg) {
	for (uint8_t attr = 0; attr < AttrCount; attr++) {
		if ((subscribedAttrMask & (1u << attr)) == 0) {
			continue;
		}
		auto ct = static_cast<CellDataType>(attr);
		for (uint8_t cell = 0; cell < DirSeqMidi::CellCount; cell++) {
			CellCoords coords;
			if (!DirSeqMidi::CellCoordsFromIndex(cell, coords)) {
				continue;
			}
			auto cellMask = 1u << cell;
			auto value = DirSeqMidi::StoredAdjustedCellValue(alg, coords.x, coords.y, ct);
			if ((adjustedCacheValidMasks[attr] & cellMask) == 0) {
				lastAdjustedCellValues[attr][cell] = value;
				adjustedCacheValidMasks[attr] |= cellMask;
				continue;
			}
			if (lastAdjustedCellValues[attr][cell] != value) {
				dirtyAdjustedCellMasks[attr] |= cellMask;
			}
		}
	}
}

void DirSeqMidiController::SendChangedCells(DirSeqAlg& alg) {
	DirSeqMidi::Writer writer;
	writer.Begin(DirSeqMidi::ChangedCells);
	writer.WriteTarget(alg);
	uint32_t groupCountPos = writer.Len;
	writer.WriteU7(0);

	uint8_t groupCount = 0;
	for (uint8_t attr = 0; attr < AttrCount; attr++) {
		auto baseMask = dirtyBaseCellMasks[attr];
		auto adjustedMask = dirtyAdjustedCellMasks[attr];
		if ((baseMask | adjustedMask) == 0) {
			continue;
		}

		auto ct = static_cast<CellDataType>(attr);
		uint32_t valueWidth = DirSeqMidi::CellValueWidth(ct);
		uint32_t groupMasks[3] = { baseMask & adjustedMask, baseMask & ~adjustedMask, adjustedMask & ~baseMask };
		uint8_t groupFlags[3] = {
			static_cast<uint8_t>(DirSeqMidi::ChangedCellsIncludeBase | DirSeqMidi::ChangedCellsIncludeAdjusted),
			DirSeqMidi::ChangedCellsIncludeBase,
			DirSeqMidi::ChangedCellsIncludeAdjusted,
		};

		for (uint8_t group = 0; group < 3; group++) {
			auto mask = groupMasks[group];
			if (mask == 0) {
				continue;
			}

			uint8_t valueFlags = groupFlags[group];
			uint32_t recordWidth = 1;
			if (valueFlags & DirSeqMidi::ChangedCellsIncludeBase) {
				recordWidth += valueWidth;
			}
			if (valueFlags & DirSeqMidi::ChangedCellsIncludeAdjusted) {
				recordWidth += valueWidth;
			}
			if (!writer.CanWrite(3 + recordWidth)) {
				break;
			}

			uint32_t groupStart = writer.Len;
			writer.WriteU7(attr);
			writer.WriteU7(valueFlags);
			uint32_t cellCountPos = writer.Len;
			writer.WriteU7(0);

			uint8_t cellCount = 0;
			uint32_t sentMask = 0;
			for (uint8_t cell = 0; cell < DirSeqMidi::CellCount; cell++) {
				uint32_t cellMask = 1u << cell;
				if ((mask & cellMask) == 0) {
					continue;
				}
				if (!writer.CanWrite(recordWidth)) {
					break;
				}

				CellCoords coords;
				if (!DirSeqMidi::CellCoordsFromIndex(cell, coords)) {
					continue;
				}

				writer.WriteU7(cell);
				if (valueFlags & DirSeqMidi::ChangedCellsIncludeBase) {
					writer.WriteCellValue(ct, DirSeqMidi::StoredBaseCellValue(alg, coords.x, coords.y, ct));
				}
				if (valueFlags & DirSeqMidi::ChangedCellsIncludeAdjusted) {
					auto value = DirSeqMidi::StoredAdjustedCellValue(alg, coords.x, coords.y, ct);
					writer.WriteCellValue(ct, value);
					lastAdjustedCellValues[attr][cell] = value;
					adjustedCacheValidMasks[attr] |= cellMask;
				}
				cellCount++;
				sentMask |= cellMask;
			}

			if (cellCount == 0) {
				writer.Len = groupStart;
				break;
			}

			writer.Data[cellCountPos] = cellCount;
			if (valueFlags & DirSeqMidi::ChangedCellsIncludeBase) {
				dirtyBaseCellMasks[attr] &= ~sentMask;
			}
			if (valueFlags & DirSeqMidi::ChangedCellsIncludeAdjusted) {
				dirtyAdjustedCellMasks[attr] &= ~sentMask;
			}
			groupCount++;
		}
	}

	if (groupCount == 0) {
		return;
	}

	writer.Data[groupCountPos] = groupCount;
	writer.Send();
	lastSendMs = alg.Timer.TotalMs;
	lastChangedCellsMs = alg.Timer.TotalMs;
}


void DirSeqMidiController::SendPlayheadsChanged(DirSeqAlg& alg) {
	DirSeqMidi::Writer writer;
	writer.Begin(DirSeqMidi::PlayheadsChanged);
	writer.WriteTarget(alg);
	writer.WriteU7(alg.Playheads.Count);
	for (int h = 0; h < alg.Playheads.Count; h++) {
		writer.WriteU7(h);
		writer.WriteU7(DirSeqMidi::CellIndex(alg.Playheads[h].CurrentStep));
		writer.WriteU7(DirSeqMidi::CellIndex(alg.Playheads[h].InitialStep));
		lastCurrentCells[h] = alg.Playheads[h].CurrentStep;
		lastInitialCells[h] = alg.Playheads[h].InitialStep;
	}
	writer.Send();
	lastSendMs = alg.Timer.TotalMs;
	pendingPlayheads = false;
}


void DirSeqMidiController::Process(DirSeqAlg& alg) {
	if (controllerRegistered
		&& keepAliveAwaitingAck
		&& alg.Timer.TotalMs - lastKeepAliveSentMs > DirSeqMidi::KeepAliveAckTimeoutMs) {
		ClearRegistration();
		return;
	}

	if (alg.Timer.TotalMs - lastSendMs < DirSeqMidi::SendIntervalMs) {
		return;
	}

	if (!controllerRegistered) {
		if (pendingAdvertise || alg.Timer.TotalMs - lastAdvertiseMs >= DirSeqMidi::AdvertiseIntervalMs) {
			SendAdvertise(alg);
		}
		return;
	}

	if (pendingError) {
		SendError(alg);
		return;
	}
	if (pendingRegistered) {
		SendRegistered(alg, DirSeqMidi::Registered);
		pendingRegistered = false;
		return;
	}
	if (!keepAliveAwaitingAck
		&& alg.Timer.TotalMs - lastKeepAliveSentMs >= DirSeqMidi::KeepAliveIntervalMs) {
		SendKeepAlive(alg);
		return;
	}
	if (pendingIdentity) {
		SendRegistered(alg, DirSeqMidi::Identity);
		pendingIdentity = false;
		return;
	}

	for (uint8_t valueKind = 0; valueKind < DirSeqMidi::ValueKindCount; valueKind++) {
		for (uint8_t attr = 0; attr < AttrCount; attr++) {
			if (snapshotRequestMasks[valueKind] & (1u << attr)) {
				SendSnapshot(alg, attr, valueKind);
				return;
			}
		}
	}

	if (pendingPlayheads) {
		SendPlayheadsChanged(alg);
		return;
	}

	if (alg.Timer.TotalMs - lastChangedCellsMs >= DirSeqMidi::ChangedCellsIntervalMs) {
		CheckAdjustedCellChanges(alg);
		SendChangedCells(alg);
		lastChangedCellsMs = alg.Timer.TotalMs;
	}
}


void DirSeqMidiController::HandleSysEx(const uint8_t* message, uint32_t count) {
	DirSeqMidi::Reader reader;
	uint8_t cmd;
	if (!DirSeqMidi::DecodeFrame(message, count, reader, cmd)) {
		return;
	}

	if (cmd == DirSeqMidi::Discover) {
		DirSeqMidi::MarkAllSequencersForAdvertise();
		return;
	}

	uint16_t algIndex;
	uint16_t token;
	if (!DirSeqMidi::ReadTarget(reader, algIndex, token)) {
		return;
	}

	auto alg = DirSeqMidi::FindTarget(algIndex, token);
	if (alg == nullptr) {
		return;
	}

	if (DirSeqMidi::RequiresRegistration(cmd) && !alg->Midi.controllerRegistered) {
		alg->Midi.QueueError(cmd, DirSeqMidi::NotRegistered);
		return;
	}

	switch (cmd) {
		case DirSeqMidi::Register: {
			uint16_t newControllerId;
			if (!reader.ReadU14(newControllerId)) {
				alg->Midi.QueueError(cmd, DirSeqMidi::BadLength);
				return;
			}
			alg->Midi.controllerRegistered = true;
			alg->Midi.controllerId = newControllerId;
			alg->Midi.subscribedAttrMask = 0;
			for (size_t kind = 0; kind < DirSeqMidi::ValueKindCount; kind++) {
				alg->Midi.snapshotRequestMasks[kind] = 0;
			}
			for (size_t i = 0; i < AttrCount; i++) {
				alg->Midi.dirtyBaseCellMasks[i] = 0;
				alg->Midi.dirtyAdjustedCellMasks[i] = 0;
				alg->Midi.adjustedCacheValidMasks[i] = 0;
			}
			alg->Midi.pendingRegistered = true;
			alg->Midi.pendingError = false;
			alg->Midi.keepAliveAwaitingAck = false;
			alg->Midi.lastKeepAliveSentMs = alg->Timer.TotalMs;
			alg->Midi.lastSendMs = 0;
			alg->Midi.lastChangedCellsMs = 0;
			return;
		}
		case DirSeqMidi::KeepAlive:
			alg->Midi.QueueError(cmd, DirSeqMidi::UnsupportedOperation);
			return;
		case DirSeqMidi::KeepAliveAck:
			if (alg->Midi.controllerRegistered) {
				alg->Midi.keepAliveAwaitingAck = false;
			}
			return;
		case DirSeqMidi::Unregister:
			alg->Midi.ClearRegistration();
			return;
		case DirSeqMidi::RequestIdentity:
			alg->Midi.pendingIdentity = true;
			return;
		case DirSeqMidi::SubscribeAttrs: {
			uint32_t mask;
			if (!reader.ReadU21(mask)) {
				alg->Midi.QueueError(cmd, DirSeqMidi::BadLength);
				return;
			}
			alg->Midi.subscribedAttrMask = mask & ((1u << AttrCount) - 1);
			return;
		}
		case DirSeqMidi::RequestSnapshot: {
			uint8_t attr;
			uint8_t valueKind;
			if (!reader.ReadU7(attr) || !reader.ReadU7(valueKind)) {
				alg->Midi.QueueError(cmd, DirSeqMidi::BadLength);
				return;
			}
			if (attr >= AttrCount || valueKind >= DirSeqMidi::ValueKindCount) {
				alg->Midi.QueueError(cmd, DirSeqMidi::InvalidValue);
				return;
			}
			alg->Midi.snapshotRequestMasks[valueKind] |= 1u << attr;
			return;
		}
		case DirSeqMidi::SetCell:
		case DirSeqMidi::AdjustCell: {
			uint8_t attr;
			uint8_t cell;
			if (!reader.ReadU7(attr) || !reader.ReadU7(cell) || attr >= AttrCount) {
				alg->Midi.QueueError(cmd, DirSeqMidi::BadLength);
				return;
			}
			CellCoords coords;
			if (!DirSeqMidi::CellCoordsFromIndex(cell, coords)) {
				alg->Midi.QueueError(cmd, DirSeqMidi::InvalidValue);
				return;
			}
			auto ct = static_cast<CellDataType>(attr);
			auto& cd = CellDefinition::All[attr];
			int16_t value;
			if (cmd == DirSeqMidi::AdjustCell) {
				if (!reader.ReadS14(value)) {
					alg->Midi.QueueError(cmd, DirSeqMidi::BadLength);
					return;
				}
			} else if (!reader.ReadCellValue(ct, value)) {
				alg->Midi.QueueError(cmd, DirSeqMidi::BadLength);
				return;
			}
			int16_t stored = value;
			if (cmd == DirSeqMidi::AdjustCell) {
				stored = DirSeqMidi::StoredBaseCellValue(*alg, coords.x, coords.y, ct) + value;
			}
			stored = clamp(stored, cd.Min, cd.Max);
			alg->StepData.SetBaseCellValue(coords.x, coords.y, ct, cd.CellStorageToCellValue(stored), true, CallingContext::UiThread);
			alg->Grid.LoadParamForEditingIfSelected(coords, ct);
			alg->Midi.MarkCellChanged(*alg, coords.x, coords.y, ct, true);
			return;
		}
		case DirSeqMidi::SetInitialCell: {
			uint8_t playhead;
			uint8_t cell;
			if (!reader.ReadU7(playhead) || !reader.ReadU7(cell) || playhead >= alg->Playheads.Count) {
				alg->Midi.QueueError(cmd, DirSeqMidi::InvalidValue);
				return;
			}
			CellCoords coords;
			if (!DirSeqMidi::CellCoordsFromIndex(cell, coords)) {
				alg->Midi.QueueError(cmd, DirSeqMidi::InvalidValue);
				return;
			}
			alg->Playheads[playhead].InitialStep = coords;
			alg->Midi.MarkPlayheadsDirty();
			return;
		}
		case DirSeqMidi::RequestPlayheads:
			alg->Midi.pendingPlayheads = true;
			return;
		default:
			alg->Midi.QueueError(cmd, DirSeqMidi::UnknownCommand);
			return;
	}
}
