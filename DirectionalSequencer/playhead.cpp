#include <math.h>
#include "playhead.h"
#include "stepDataRegion.h"
#include "timeKeeper.h"


static constexpr uint16_t InactiveTime = 10000;
static constexpr float GateHigh = 5.0;
static constexpr float GateLow = 0.0;
// Timing margins used to preserve retriggers and ties despite ms-level timing, rounding, and small clock-rate fluctuations.
static constexpr uint8_t NearLegatoGatePctMax = 95;
static constexpr uint8_t TieMaxGateOvershootPct = 105;
static constexpr uint8_t TieClockGatePct = 101;
static constexpr uint8_t ClockGateRoundingLeewayMs = 2;
static constexpr uint8_t GateRetriggerDipMs = 2;
static constexpr float NearFullAttenuationPctMax = 98.0f;


Playhead::Playhead() {

}


void Playhead::InjectDependencies(size_t idx, const Dependencies& dependencies) {
	Index = idx;
	Deps = dependencies;
}


void Playhead::SetConfig(const PlayheadConfig& config) {
	Config = config;
}


void Playhead::Reset() {
	CurrentStep = { -1, -1 };
	ClockCount = 0;
	AdvanceCount = 0;
	UniqueAdvanceCount = 0;
	// if there is no direction on the initial step, set us moving to the right
	auto dir = Deps.StepData->GetAdjustedCellValue(InitialStep.x, InitialStep.y, CellDataType::Direction);
	dir = (dir == 0 ? 3 : dir);
	Direction = dir;
	RepeatCount = 0;
	IsRepeat = false;
	Ratchets.Active = false;
	TieCount = 0;
	Tie = TieMode::None;
	InitVisitCounts();
	// TODO:  anything else need to happen when resetting the sequencer?
}


void Playhead::InitVisitCounts() {
	for (int x = 0; x < GridSizeX; x++) {
		for (int y = 0; y < GridSizeY; y++) {
			CellVisitCounts[x][y] = 0;
		}
	}
}


void Playhead::ProcessClockTrigger() {

	LastClockTriggerTime = Deps.Timer->TotalMs;
	InactiveResetApplied = false;

	// we can't calculate the clock rate without knowing the time of the last clock
	if (LastClock > 0) {
		// if the clock rate has been stable, but we are now longer by more than 10x, assume this is because the clock was stopped,
		// and continue to use the old clock rate instead of calculating a new one.
		auto newRate = Deps.Timer->TotalMs - LastClock;
		bool wayOff = newRate > (ClockRate * 10);
		if (StableClock && wayOff) {
			// don't change the clock rate
		} else {
			ClockRate = Deps.Timer->TotalMs - LastClock;
		}
	}

	ClockCount++;
	
	if (ShouldAdvance()) {
		Advance();
	}

	// calculate if the clock rate is stable...  only then can we ratchet
	// we define stable to mean within 5% of the previous rate
	float delta = static_cast<float>(PrevClockRate) - static_cast<float>(ClockRate);
	float percentOff = fabsf(delta) / static_cast<float>(PrevClockRate);
	StableClock = (percentOff <= 0.05f);

	LastClock = Deps.Timer->TotalMs;
	PrevClockRate = ClockRate;
}


void Playhead::ProcessResetTrigger() {
	ResetQueued = true;
}


void Playhead::Process() {
	// check to see if we have been inactive long enough to reset the sequencer
	if (Config.ResetWhenInactive && !InactiveResetApplied) {
		auto inactiveFor = Deps.Timer->TotalMs - LastClockTriggerTime;
		if (inactiveFor > InactiveTime) {
			Reset();
			InactiveResetApplied = true;
		}
	}

	if (Ratchets.Active) {
		if (Ratchets.RemainingDuration > 0) {
			Ratchets.RemainingDuration--;
		} else {
			if (Ratchets.Count <= 0) {
				// this was our last ratchet, let's clean up
				Ratchets.Active = false;
			} else {
				// schedule the next ratchet with accumulated timing
				Ratchets.Count--;
				Ratchets.AccumulatedTime += Ratchets.Substep;
				Ratchets.RemainingDuration = static_cast<int>(Ratchets.AccumulatedTime);
				Ratchets.AccumulatedTime -= static_cast<int>(Ratchets.AccumulatedTime);
				GateStart = Deps.Timer->TotalMs;
				GateEnd = GateStart + Ratchets.GateLen;
			}
		}
	}


	if (QuantReturnSupplied) {
		Glide.NewStepVal = QuantReturn;
	}

	auto val = Glide.NewStepVal;
	if (Glide.Duration > 0) {
		Glide.Delta = (Glide.NewStepVal - Glide.OldStepVal) / Glide.Duration;
		val = Glide.OldStepVal + Glide.Delta;
		Glide.OldStepVal = val;
		Glide.Duration -= 1;
	}


	auto velocity = NormalizeVelocityForOutput();


	// velocity range is 0-10, velo gate range is x-5
	auto veloGateMin = Config.VelocityGateMin;
	auto veloGate = veloGateMin + (velocity * (GateHigh - veloGateMin) / 10.0f);


	float gate = ((Deps.Timer->TotalMs >= GateStart) && (Deps.Timer->TotalMs < GateEnd)) ? veloGate : GateLow;

	if (Dip > 0) {
		Dip -= 1;
		gate = GateLow;
	}

	Outputs.Value = val;
	Outputs.Gate = gate;
	Outputs.Velocity = velocity;
	Outputs.PreQuantStepVal = PreQuantStepVal;
}


uint32_t Playhead::EffectiveClockRate(){
	uint32_t divisor = Config.ClockDivisor;
	return ClockRate * divisor;
}


void Playhead::Advance() {

	ResetIfNecessary();

	AdvanceCount++;
	if (RepeatCount <= 0) {
		UniqueAdvanceCount++;
	}

	MoveToNextCell();

	// start off emitting a gate...  further processing may change this
	EmitGate = true;
	ProcessMute();
	ProcessRest();
	ProcessProbability();
	ProcessTies();
	ProcessRepeats();
	RecordCellVisit();

	CalculateGateLength();

	if (EmitGate) {
		// only change the value when we are emitting a gate
		CalculateStepValue();
		ProcessAccumulator();
		ProcessDrift();
		AttenuateValue();
		OffsetValue();
		QuantizeValue();

		SetupStepValue();
		DipIfNeccessary();
		ProcessGlide();
		ProcessRatchets();
		AttenuateGateLength();
		CalculateGate();
		HumanizeGate();
		CalculateVelocity();
		HumanizeVelocity();
		
		LastGatePct = GatePct;
	} else {
		LastGatePct = 0;
	}
}


void Playhead::ProcessTies() {
	if (TieCount == 0) {
		// only start a tie if we are not resting or muted
		if (EmitGate) {
			TieCount = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::TieSteps);
			Tie = (TieCount > 0) ? TieMode::Start : TieMode::None;
		}
	} else {
		TieCount--;
		Tie = TieMode::Continuation;
	}
}


bool Playhead::ShouldAdvance() {
	uint32_t divisor = Config.ClockDivisor;
	uint32_t offset = Config.ClockOffset;
	return (((ClockCount - 1) % divisor) == offset);
}


void Playhead::ResetIfNecessary() {
	if (ResetQueued) {
		Reset();
		ResetQueued = false;
		ClockCount++;
		return;
	}

	uint32_t resetAfter = Config.ResetAfterNSteps;
	if (resetAfter > 0) {
		if (AdvanceCount >= resetAfter) {
			Reset();
			ClockCount++;
		}
	}
}


void Playhead::MoveToNextCell() {
	IsRepeat = false;
	// if we don't have a current step, start at the initial step, otherwise, move to the next step
	if (CurrentStep.x == -1 && CurrentStep.y == -1) {
		CurrentStep = InitialStep;
	} else if (RepeatCount > 0) {
		// don't change the step if we need to repeat
		IsRepeat = true;
		RepeatCount--;
	} else {
		// if we have a new direction, change to that, otherwise keep going in the direction we had, or right if we have no initial direction
		uint8_t dir = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::Direction);
		if (dir > 0) {
			Direction = dir;
		} else if (Direction == 0) {
			Direction = 3;
		}

		// we want to move forward in the given direction this many times
		auto nSteps = Config.MoveNCells;

		// also we want to skip forward in the sequence, in the direction of travel, by one step every so often, but only counting non-repeated steps
		auto skipAfterN = Config.SkipAfterNSteps;

		uint8_t skip = 0;
		if (skipAfterN > 0) {
			if (static_cast<uint16_t>((UniqueAdvanceCount - 1) % skipAfterN) == 0) {
				skip = 1;
			}
		}

		// change the selected position, and wrap it to the other side if it goes off the grid
		int8_t xOffset = (Direction == 6 || Direction == 7 || Direction == 8) ? -1 : 0;
		xOffset = (Direction == 2 || Direction == 3 || Direction == 4) ? 1 : xOffset;
		xOffset = (xOffset * nSteps) + (xOffset * skip);
		CurrentStep.x = wrap(CurrentStep.x + xOffset, 0, GridSizeX - 1);

		int8_t yOffset = (Direction == 1 || Direction == 2 || Direction == 8) ? -1 : 0;
		yOffset = (Direction == 4 || Direction == 5 || Direction == 6) ? 1 : yOffset;
		yOffset = (yOffset * nSteps) + (yOffset * skip);
		CurrentStep.y = wrap(CurrentStep.y + yOffset, 0, GridSizeY - 1);
	}
}


void Playhead::CalculateStepValue() {
	// don't change the value if we are continuing a tie
	if (Tie == TieMode::Continuation)
		return;
	
	StepVal = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::Value);
}


void Playhead::ProcessAccumulator() {
	// don't change the value if we are continuing a tie
	if (Tie == TieMode::Continuation)
		return;
	
	auto accumAdd = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::AccumAdd);
	auto accumTimes = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::AccumTimes);

	// add the accumulation amount as many times as specified
	if (accumTimes > 0) {
		// the visit for this iteration has already been incremented by this point, so subtract 1 to make it zero-based
		auto visit = CellVisitCounts[CurrentStep.x][CurrentStep.y] - 1;
		auto times = visit % static_cast<uint32_t>(accumTimes + 1);
		StepVal += (accumAdd * times);
	}
}


void Playhead::ProcessDrift() {
	// don't change the value if we are continuing a tie
	if (Tie == TieMode::Continuation)
		return;
	
	auto driftProb = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::DriftProb);
	if (Deps.Random->Next(1, 100) <= driftProb) {
		// we are going to drift the value, now let's figure out by how much
		// we want a much higher likelihood of the drift amount being small vs. large, so use an exponential scale
		// this gives us a scaling factor from 0 to 1
		// don't use UINT32_MAX here because I'm not sure of the uniformity of the randomness distribution.
		// Instead use a smaller number, which gives us less resolution, but hopefully better uniformity due to modulo distribution
		static constexpr uint32_t res = 987654;
		auto driftScale = static_cast<float>(Deps.Random->Next(0, res)) / res;
		driftScale = sqrt(driftScale);
		// calculate the drift range from the max
		auto driftRange = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::MaxDrift);
		// scale the drift range to get the actual drift, and make it negative half of the time
		auto actualDrift = driftRange * driftScale * (Deps.Random->Next(0, 1) == 1 ? -1 : 1);
		StepVal += actualDrift;
	}
}


void Playhead::AttenuateValue() {
	// don't change the value if we are continuing a tie
	if (Tie == TieMode::Continuation)
		return;
	
	auto atten = Config.AttenValue;
	StepVal *= (atten / 100.0f);
}


void Playhead::OffsetValue() {
	// don't change the value if we are continuing a tie
	if (Tie == TieMode::Continuation)
		return;
	
	auto offset = Config.OffsetValue;
	StepVal += offset;
}


void Playhead::QuantizeValue()	{
	// don't change the value if we are continuing a tie
	if (Tie == TieMode::Continuation)
		return;
	
	if (QuantReturnSupplied) {
		PreQuantStepVal = StepVal;
		StepVal = QuantReturn;
	}
}


void Playhead::ProcessMute() {
	auto mute = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::Mute);
	if (mute) {
		EmitGate = false;
	}
}


void Playhead::ProcessRest() {
	// calculate if we should rest this cell this time around
	uint8_t restAfter = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::RestAfter);
	if (restAfter > 0) {
		if (CellVisitCounts[CurrentStep.x][CurrentStep.y] % (restAfter + 1) == restAfter) {
			EmitGate = false;
		}
	}

	// calculate if we should apply the global rest
	auto restAfterN = Config.RestAfterNSteps;
	if (restAfterN > 0) {
		if (AdvanceCount % (restAfterN + 1) == 0) {
			EmitGate = false;
		}
	}
}


void Playhead::ProcessProbability() {
	// apply probability to see if we should emit a gate for this step
	auto prob = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::Probability);
	if (Deps.Random->Next(1, 100) > prob) {
		EmitGate = false;
	}
}


void Playhead::ProcessRepeats() {
	// if we're not already playing a repeated cell, set the repeat counter.  If we are repeating, this will tick down above
	if (!IsRepeat) {
		RepeatCount = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::Repeats);
		if (Tie != TieMode::None)
			TieCount += RepeatCount;
	}
}


void Playhead::RecordCellVisit() {
	CellVisitCounts[CurrentStep.x][CurrentStep.y]++;
}


void Playhead::CalculateGateLength() {
	GatePct = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::GateLength);
	auto gateLengthSource = Config.GateSource;

	// if we are processing a tie, always play legato.  100% if we are using clocked gates, otherwise calc the percentage

	if (GatePct == 0) {
		EmitGate = false;
		return;
	}

	// if we are not playing legato, cheat a little bit to make sure there is always a gap, to make sure gates don't overlap
	if (GatePct > NearLegatoGatePctMax && GatePct < 100) {
		GatePct = NearLegatoGatePctMax;
	}

	// calculate the gate length for the step
  // if we are processing a tie, always play legato.  100% if we are using clocked gates, otherwise calc the percentage
	if (gateLengthSource == GateLengthSource::MaxGateLength) {
		auto maxLen = Config.MaxGateLength;
		// if this is a tie step, play legato.  calc the percentage, using about 5% leeway.
		// It's ok to overshoot because it will be recalculated next step.  But wa don't want to overshoot by much, so the gate stops if we stop the clock
		// We don't, however, want to undershoot, because then we will not play legato
		GatePct = (Tie != TieMode::None) ? EffectiveClockRate() * TieMaxGateOvershootPct / maxLen : GatePct;
		GateLen = maxLen * GatePct / 100;
	} else if (gateLengthSource == GateLengthSource::Clock) {
		// if this is a tie step, play legato, again overshooting by a small amount to account for rounding errors
		GatePct = (Tie != TieMode::None) ? TieClockGatePct : GatePct;
		GateLen = EffectiveClockRate() * GatePct / 100;
		// this cheat helps ensure that the gate will play legato if the gatelen is 100%
		// floating point math and/or slight clock fluctuations could make it not work out that way otherwise
		if (GateLen > 0) {
			GateLen += ClockGateRoundingLeewayMs;
		}
	}
}


void Playhead::SetupStepValue() {
	// don't change the glide values if we are continuing a tie
	if (Tie == TieMode::Continuation)
		return;
	
	// a duration of zero means no glide, just a normal step.  We will adjust duration below, if required
	Glide.OldStepVal = Glide.NewStepVal;
	Glide.NewStepVal = StepVal;
	Glide.Duration = 0;
	Glide.Delta = 0;
}


void Playhead::DipIfNeccessary() {
	// don't dip if we are processing a tie
	if (Tie != TieMode::None)
		return;
	
	// unless we are playing legato (last step gate len = 100), we have to briefly dip to end the previous gate,
	// no matter the calculated length, unless of course we are already low
	if (LastGatePct < 100 && Outputs.Gate > 0.0) {
		Dip = GateRetriggerDipMs;
	}
}


void Playhead::ProcessGlide() {
	// Don't recalculate glide on continuation steps - use what was set on the start step
	if (Tie == TieMode::Continuation)
		return;
	
	auto glidePct = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::Glide);
	if (glidePct > 0) {
		// If we're starting a tie, spread the glide across all tied steps
		auto totalGateLen = GateLen;
		if (Tie == TieMode::Start && TieCount > 0) {
			// Total duration spans all tied steps (current step + remaining TieCount steps)
			totalGateLen = GateLen * (TieCount + 1);
		}
		
		// calculate the glide as a percentage of the total gate length
		Glide.Duration = totalGateLen * glidePct / 100;
		Glide.Delta = (Glide.NewStepVal - Glide.OldStepVal) / Glide.Duration;
	} else {
		Glide.Duration = 0;
		Glide.Delta = 0;
	}
}


void Playhead::ProcessRatchets() {
	// don't ratchet if we are pard of a tie
	if (Tie != TieMode::None)
		return;
	
	auto ratchets = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::Ratchets);
	// only ratchet when our clock is stable, otherwise it will sound weird
	if (ratchets > 0 && StableClock) {
		auto divisions = ratchets + 1;
		auto substep = EffectiveClockRate() / divisions;
		GateLen /= divisions;

		// schedule the ratchets to occur with accumulated timing
		Ratchets.Active = true;
		Ratchets.Count = ratchets;
		Ratchets.Substep = substep;
		Ratchets.GateLen = GateLen;
		Ratchets.RemainingDuration = substep;
		Ratchets.AccumulatedTime = 0;
	}
}


void Playhead::AttenuateGateLength() {
	// don't change the gate length if we are part of a tie
	if (Tie != TieMode::None)
		return;

	// if we are using a defined max gate length, we change that to attenuate.  We don't want to double attenuate here
	auto gateLengthSource = Config.GateSource;
	if (gateLengthSource == GateLengthSource::MaxGateLength) {
		return;
	}

	auto atten = Config.GateLengthAttenuate;

	// if we've attenuated down to zero, don't even calculate
	if (atten == 0) {
		GateLen = 0;
		if (Ratchets.Active) {
			Ratchets.GateLen = 0;
		}
		return;
	}

	// this cheat allows us to attenuate 100% gate lengths down and ensure they are not played legato due to floating point math issues
	if (atten > NearFullAttenuationPctMax && atten < 100) {
		atten = NearFullAttenuationPctMax;
	}

	// attenuate the gate length, but not below 2
	if (GateLen > 0) {
		GateLen *= (atten / 100);
		GateLen = max(GateLen, static_cast<uint32_t>(2));
	}

	// attenuate the ratcheting gate length, but not below 2
	if (Ratchets.Active) {
		if (Ratchets.GateLen > 0) {
			Ratchets.GateLen *= (atten / 100);
			Ratchets.GateLen = max(Ratchets.GateLen, static_cast<uint32_t>(2));
		}
	}
}


void Playhead::CalculateGate() {
	GateStart = Deps.Timer->TotalMs;
	GateEnd = GateStart + GateLen;
}


void Playhead::HumanizeGate() {
	// only humanize non-legato gates
	if (GatePct < 100) {
		auto human = Config.HumanizeValue;
		human *= 1000;
		float pct1 = Deps.Random->Next(0, human) / 1000.0f;
		float pct2 = Deps.Random->Next(0, human) / 1000.0f;
		auto off1 = GateLen * pct1 / 100.0f;
		auto off2 = GateLen * pct2 / 100.0f;
		// we always have to start late, because we can't read the future...
		GateStart += off1;
		// but we can end early or late
		off2 = off2 * (Deps.Random->Next(0, 1) == 1 ? -1 : 1);
		GateEnd -= off2;
	}
}


void Playhead::CalculateVelocity() {
	// don't change the velocity if we are continuing a tie
	if (Tie == TieMode::Continuation)
		return;
	
	auto atten = Config.VelocityAttenuate;
	auto offset = Config.VelocityOffset;
	auto velo = Deps.StepData->GetAdjustedCellValue(CurrentStep.x, CurrentStep.y, CellDataType::Velocity);
	velo = velo * atten / 100.0f;
	velo += offset;
	Velocity = clamp(static_cast<int>(velo), 1, 127);
}


void Playhead::HumanizeVelocity() {
	// don't change the velocity if we are continuing a tie
	if (Tie == TieMode::Continuation)
		return;
	
	auto human = Config.HumanizeValue;
	human *= 1000;
	float pct = Deps.Random->Next(0, human) / 1000.0f;
	auto off = Velocity * pct / 100.0f;
	// we can move up or down in velocity
	off = off * (Deps.Random->Next(0, 1) == 1 ? -1 : 1);
	auto velo = Velocity + off;
	// clamp the result to the velocity range
	Velocity = clamp(static_cast<int>(velo), 1, 127);
}


float Playhead::NormalizeVelocityForOutput() {
	// scale the velocity to a 0-10V range
	return static_cast<float>(Velocity) * 10.0 / 127.0f;
}


void PlayheadList::Init(int cnt, Playhead* arr) {
	Playheads = arr;
	Count = cnt;
}


Playhead& PlayheadList::operator[](size_t index) const {
	return Playheads[index];
}
