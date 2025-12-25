#pragma once

#include <distingnt/api.h>
#include "common.h"
#include "gridInfo.h"
#include "cellDefinition.h"
#include "timeKeeper.h"
#include "stepDataRegion.h"


// this class represents a single playhead moving thru the sequencer field
struct Playhead {
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

	_NT_algorithm* Algorithm = nullptr;
	StepDataRegion* StepData = nullptr;
	TimeKeeper* Timer = nullptr;
	RandomGenerator* Random = nullptr;
	int ParamOffset;

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
	void QuantizeValue();
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

	OutputInfo Outputs;

	bool QuantReturnSupplied;
	float QuantReturn;

	Trigger ResetTrigger;
	Trigger ClockTrigger;

	Playhead();
	void InjectDependencies(_NT_algorithm* alg, size_t idx, TimeKeeper* timer, RandomGenerator* rnd, StepDataRegion* stepData);

	void ProcessClockTrigger();
	void ProcessResetTrigger();
	void Process();
};


struct PlayheadList {
private:
	Playhead* Playheads;
public:
	int Count;

	void Init(int cnt, Playhead* arr);
	Playhead& operator[](size_t index) const;
};