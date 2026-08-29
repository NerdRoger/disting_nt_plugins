#pragma once
#include <stddef.h>
#include <stdint.h>
#include "cellDefinition.h"
#include "gridInfo.h"

#ifndef DIRSEQ_ENABLE_CUSTOM_MIDI
#define DIRSEQ_ENABLE_CUSTOM_MIDI 0
#endif

struct DirSeqAlg;

class DirSeqMidiController {
public:
	static constexpr size_t AttrCount = static_cast<size_t>(CellDataType::NumCellDataTypes);
	static constexpr size_t MaxPlayheads = 8;
	static constexpr uint8_t InvalidCell = 0x7F;

#if DIRSEQ_ENABLE_CUSTOM_MIDI
	void Init(DirSeqAlg& alg);
	void SetInstanceToken(uint16_t token);
	void MarkCellChanged(DirSeqAlg& alg, uint8_t x, uint8_t y, CellDataType ct, bool force = false);
	void MarkPlayheadsDirty();
	void RequestAdvertise() { pendingAdvertise = true; }
	void CheckPlayheadChanges(DirSeqAlg& alg);
	void Process(DirSeqAlg& alg);

	static void HandleSysEx(const uint8_t* message, uint32_t count);

	uint16_t InstanceToken() const { return instanceToken; }

private:
	uint16_t instanceToken = 0;
	bool controllerRegistered = false;
	uint16_t controllerId = 0;
	uint32_t lastAdvertiseMs = 0;
	uint32_t lastSendMs = 0;
	uint32_t lastChangedCellsMs = 0;
	uint32_t lastKeepAliveSentMs = 0;
	uint32_t subscribedAttrMask = 0;
	uint32_t dirtyBaseCellMasks[AttrCount] = { };
	uint32_t dirtyAdjustedCellMasks[AttrCount] = { };
	uint32_t snapshotRequestMasks[2] = { };
	uint32_t adjustedCacheValidMasks[AttrCount] = { };
	int16_t lastAdjustedCellValues[AttrCount][GridSizeX * GridSizeY] = { };
	bool pendingRegistered = false;
	bool pendingIdentity = false;
	bool pendingPlayheads = false;
	bool pendingAdvertise = false;
	bool pendingError = false;
	bool keepAliveAwaitingAck = false;
	uint8_t errorCommand = 0;
	uint8_t errorCode = 0;
	CellCoords lastCurrentCells[MaxPlayheads] = { };
	CellCoords lastInitialCells[MaxPlayheads] = { };

	void QueueError(uint8_t originalCommand, uint8_t code);
	void ClearRegistration();
	void SendAdvertise(DirSeqAlg& alg);
	void SendRegistered(DirSeqAlg& alg, uint8_t command);
	void SendError(DirSeqAlg& alg);
	void SendKeepAlive(DirSeqAlg& alg);
	void SendSnapshot(DirSeqAlg& alg, uint8_t attr, uint8_t valueKind);
	void CheckAdjustedCellChanges(DirSeqAlg& alg);
	void SendChangedCells(DirSeqAlg& alg);
	void SendPlayheadsChanged(DirSeqAlg& alg);
#else
	void Init(DirSeqAlg&) {}
	void SetInstanceToken(uint16_t) {}
	void MarkCellChanged(DirSeqAlg&, uint8_t, uint8_t, CellDataType, bool = false) {}
	void MarkPlayheadsDirty() {}
	void RequestAdvertise() {}
	void CheckPlayheadChanges(DirSeqAlg&) {}
	void Process(DirSeqAlg&) {}

	static void HandleSysEx(const uint8_t*, uint32_t) {}

	uint16_t InstanceToken() const { return 0; }
#endif
};
