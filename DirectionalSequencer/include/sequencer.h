#pragma once

#include "ownedBase.h"
#include "gridInfo.h"
#include "cellData.h"


struct DirectionalSequencer;


struct Sequencer : OwnedBase<DirectionalSequencer> {
private:
	struct RatchetInfo {
		bool Active;
		uint8_t Count;
		float Substep;
		uint32_t GateLen;
		float RemainingDuration;
		float AccumulatedTime;
	};

	struct GlideInfo {
		float OldStepVal;
		float NewStepVal;
		float Delta;
		uint32_t Duration;
	};

	struct OutputInfo {
		float Value;
		float Gate;
		float Velocity;
		float PreQuantStepVal;
	};

	static constexpr uint16_t InactiveTime = 10000;
	static constexpr float GateHigh = 5.0;
	static constexpr float GateLow = 0.0;

	uint32_t LastClock;
	uint32_t ClockRate = 500; // 500ms = 120BPM
	uint32_t PrevClockRate = 500; // 500ms = 120BPM
	bool StableClock = true;

	uint32_t AdvanceCount;
	uint32_t UniqueAdvanceCount;
	uint8_t Direction;
	uint8_t RepeatCount;
	bool IsRepeat;
	RatchetInfo Ratchets;
	bool ResetQueued;
	uint32_t LastAdvanceTime;
	uint32_t GateStart;
	uint32_t GateEnd;
	GlideInfo Glide;
	uint8_t Dip;
	bool EmitGate;
	uint8_t GatePct;
	uint8_t LastGatePct;
	float PreQuantStepVal;
	float StepVal;
	uint32_t CellVisitCounts[GridSizeX][GridSizeY];
	uint32_t GateLen;
	uint8_t Velocity = 127;

	void Advance();
	void Reset();
	void InitVisitCounts();
	void ResetIfNecessary();
	void MoveToNextCell();
	void CalculateStepValue();
	void ProcessAccumulator();
	void ProcessDrift();
	void AttenuateValue();
	void OffsetValue();
	void QuantizeValue()	;
	void ProcessRest();
	void ProcessProbability();
	void ProcessRepeats();
	void RecordCellVisit();
	void CalculateGateLength();
	void SetupStepValue();
	void DipIfNeccessary();
	void ProcessGlide();
	void ProcessRatchets();
	void AttenuateGateLength();
	void CalculateGate();
	void HumanizeGate();
	void CalculateVelocity();
	void HumanizeVelocity();
	float NormalizeVelocityForOutput();

public:
	CellCoords InitialStep { 0, 0 };
	CellCoords CurrentStep = { -1, -1 };

	CellData Cells[GridSizeX][GridSizeY];	
	OutputInfo Outputs;

	bool QuantReturnSupplied;
	float QuantReturn;

	void ProcessClockTrigger();
	void ProcessResetTrigger();
	void Process();

	Sequencer();
};