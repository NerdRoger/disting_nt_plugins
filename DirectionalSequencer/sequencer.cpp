#include <stdio.h>
#include "sequencer.h"
#include "directionalSequencer.h"
#include "cellDefinition.h"


Sequencer::Sequencer() {
	// set the default cell values
	for (size_t i = 0; i < ARRAY_SIZE(CellDefinitions); i++) {
		auto& cd = CellDefinitions[i];
		for (int x = 0; x < GridSizeX; x++) {
			for (int y = 0; y < GridSizeY; y++) {
				Cells[x][y].SetField(*AlgorithmInstance, static_cast<CellDataType>(i), cd.Default);
			}
		}
	}
	// default state for direction should give us an initial direction (east)
	Cells[InitialStep.x][InitialStep.y].SetField(*AlgorithmInstance, CellDataType::Direction, 3);
}


void Sequencer::Reset() {
	CurrentStep = { -1, -1 };
	AdvanceCount = 0;
	UniqueAdvanceCount = 0;
	// if there is no direction on the initial step, set us moving to the right
	auto dir = Cells[InitialStep.x][InitialStep.y].GetField(*AlgorithmInstance, CellDataType::Direction);
	dir = (dir == 0 ? 3 : dir);
	Direction = dir;
	RepeatCount = 0;
	IsRepeat = false;
	Ratchets.Active = false;
	InitVisitCounts();
	// TODO:  anything else need to happen when resetting the sequencer?
}


void Sequencer::InitVisitCounts() {
	for (int x = 0; x < GridSizeX; x++) {
		for (int y = 0; y < GridSizeY; y++) {
			CellVisitCounts[x][y] = 0;
		}
	}
}


void Sequencer::ProcessClockTrigger() {
	// we can't calculate the clock rate without knowing the time of the last clock
	if (LastClock > 0) {
		// if the clock rate has been stable, but we are now longer by more than 10x, assume this is because the clock was stopped,
		// and continue to use the old clock rate instead of calculating a new one.
		auto newRate = AlgorithmInstance->TotalMs - LastClock;
		bool wayOff = newRate > (ClockRate * 10);
		if (StableClock && wayOff) {
			// don't change the clock rate
		} else {
			ClockRate = AlgorithmInstance->TotalMs - LastClock;
		}
	}

	Advance();

	// calculate if the clock rate is stable...  only then can we ratchet
	// we define stable to mean within 5% of the previous rate
	float delta = static_cast<float>(PrevClockRate) - static_cast<float>(ClockRate);
	float percentOff = fabsf(delta) / static_cast<float>(PrevClockRate);
	StableClock = (percentOff <= 0.05f);

	LastClock = AlgorithmInstance->TotalMs;
	PrevClockRate = ClockRate;
}


void Sequencer::ProcessResetTrigger() {
	ResetQueued = true;
}


void Sequencer::Process() {
	// check to see if we have been inactive long enough to reset the sequencer
	bool resetWhenInactive = (AlgorithmInstance->v[kParamResetWhenInactive] == 1);
	if (resetWhenInactive) {
		auto inactiveFor = AlgorithmInstance->TotalMs - LastAdvanceTime;
		if (inactiveFor > InactiveTime) {
			Reset();
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
				GateStart = AlgorithmInstance->TotalMs;
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

	float gate = ((AlgorithmInstance->TotalMs >= GateStart) && (AlgorithmInstance->TotalMs < GateEnd)) ? GateHigh : GateLow;

	if (Dip > 0) {
		Dip -= 1;
		gate = GateLow;
	}

	Outputs.Value = val;
	Outputs.Gate = gate;
	Outputs.Velocity = NormalizeVelocityForOutput();
	Outputs.PreQuantStepVal = PreQuantStepVal;
}


void Sequencer::Advance() {
	ResetIfNecessary();

	LastAdvanceTime = AlgorithmInstance->TotalMs;
	AdvanceCount++;
	if (RepeatCount <= 0) {
		UniqueAdvanceCount++;
	}

	MoveToNextCell();

	// start off emitting a gate...  further processing may change this
	EmitGate = true;
	ProcessRest();
	ProcessProbability();
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


void Sequencer::ResetIfNecessary() {
	if (ResetQueued) {
		Reset();
		ResetQueued = false;
		return;
	}

	uint32_t resetAfter = AlgorithmInstance->v[kParamResetAfterNSteps];
	if (resetAfter > 0) {
		if (AdvanceCount > resetAfter) {
			Reset();
		}
	}
}


void Sequencer::MoveToNextCell() {
	IsRepeat = false;
	// if we don't have a current step, start at the initial step, otherwise, move to the next step
	if (CurrentStep.x == -1 && CurrentStep.y == -1) {
		CurrentStep = InitialStep;
	} else if (RepeatCount > 0) {
		// don't change the step if we need to repeat
		IsRepeat = true;
		RepeatCount--;
	} else {
		// if we have a new direction, change to that, otherwise keep going in the direction we had
		uint8_t dir = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::Direction);
		if (dir > 0) {
			Direction = dir;
		}

		// we want to move forward in the given direction this many times
		auto nSteps = AlgorithmInstance->v[kParamMoveNCells];

		// also we want to skip forward in the sequence, in the direction of travel, by one step every so often, but only counting non-repeated steps
		auto skipEveryN = AlgorithmInstance->v[kParamSkipAfterNSteps];

		uint8_t skip = 0;
		if (skipEveryN > 0) {
			if (static_cast<uint16_t>(UniqueAdvanceCount % skipEveryN) == (skipEveryN - 1)) {
				skip = 1;
			}
		}

		// change the selected position, and wrap it to the other side if it goes off the grid
		uint8_t xOffset = (Direction == 6 || Direction == 7 || Direction == 8) ? -1 : 0;
		xOffset = (Direction == 2 || Direction == 3 || Direction == 4) ? 1 : xOffset;
		xOffset = (xOffset * nSteps) + (xOffset * skip);
		CurrentStep.x = wrap(CurrentStep.x + xOffset, 0, GridSizeX - 1);

		uint8_t yOffset = (Direction == 1 || Direction == 2 || Direction == 8) ? -1 : 0;
		yOffset = (Direction == 4 || Direction == 5 || Direction == 6) ? 1 : yOffset;
		yOffset = (yOffset * nSteps) + (yOffset * skip);
		CurrentStep.y = wrap(CurrentStep.y + yOffset, 0, GridSizeY - 1);
	}
}


void Sequencer::CalculateStepValue() {
	StepVal = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::Value);
}


void Sequencer::ProcessAccumulator() {
	auto accumAdd = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::AccumAdd);
	auto accumTimes = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::AccumTimes);

	// add the accumulation amount as many times as specified
	if (accumTimes > 0) {
		auto visit = CellVisitCounts[CurrentStep.x][CurrentStep.y];
		auto times = visit % static_cast<uint32_t>(accumTimes + 1);
		StepVal += (accumAdd * times);
	}
}


void Sequencer::ProcessDrift() {
	auto driftProb = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::DriftProb);
	if (AlgorithmInstance->Random.Next(1, 100) <= driftProb) {
		// we are going to drift the value, now let's figure out by how much
		// we want a much higher likelihood of the drift amount being small vs. large, so use an exponential scale
		// this gives us a scaling factor from 0 to 1
		// don't use UINT32_MAX here because I'm not sure of the uniformity of the randomness distribution.
		// Instead use a smaller number, which gives us less resolution, but hopefully better uniformity due to modulo distribution
		static constexpr uint32_t res = 987654;
		auto driftScale = static_cast<float>(AlgorithmInstance->Random.Next(0, res)) / res;
		driftScale = pow(driftScale, 0.5f);
		// calculate the drift range from the max
		auto driftRange = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::MaxDrift);
		// scale the drift range to get the actual drift, and make it negative half of the time
		auto actualDrift = driftRange * driftScale * (AlgorithmInstance->Random.Next(0, 1) == 1 ? -1 : 1);
		StepVal += actualDrift;
	}
}


void Sequencer::AttenuateValue() {
	auto& param = AlgorithmInstance->parameters[kParamAttenValue];
	float scaling = CalculateScaling(param.scaling);
	auto atten = AlgorithmInstance->v[kParamAttenValue] / scaling;
	StepVal *= (atten / 100.0f);
}


void Sequencer::OffsetValue() {
	auto& param = AlgorithmInstance->parameters[kParamOffsetValue];
	float scaling = CalculateScaling(param.scaling);
	auto offset = AlgorithmInstance->v[kParamOffsetValue] / scaling;
	StepVal += offset;
}


void Sequencer::QuantizeValue()	{
	if (QuantReturnSupplied) {
		PreQuantStepVal = StepVal;
		StepVal = QuantReturn;
	}
}


void Sequencer::ProcessRest() {
	// calculate if we should rest this cell this time around
	uint8_t restAfter = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::RestAfter);
	if (restAfter > 0) {
		if (CellVisitCounts[CurrentStep.x][CurrentStep.y] % (restAfter + 1) == restAfter) {
			EmitGate = false;
		}
	}

	// calculate if we should apply the global rest
	auto restEvery = AlgorithmInstance->v[kParamRestAfterNSteps];
	if (restEvery > 0) {
		if (AdvanceCount % (restEvery + 1) == static_cast<uint32_t>(restEvery)) {
			EmitGate = false;
		}
	}
}


void Sequencer::ProcessProbability() {
	// apply probability to see if we should emit a gate for this step
	auto prob = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::Probability);
	if (AlgorithmInstance->Random.Next(1, 100) > prob) {
		EmitGate = false;
	}
}


void Sequencer::ProcessRepeats() {
	// if we're not already playing a repeated cell, set the repeat counter.  If we are repeating, this will tick down above
	if (!IsRepeat) {
		RepeatCount = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::Repeats);
	}
}


void Sequencer::RecordCellVisit() {
	CellVisitCounts[CurrentStep.x][CurrentStep.y]++;
}


void Sequencer::CalculateGateLength() {
	GatePct = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::GateLength);

	if (GatePct == 0) {
		EmitGate = false;
		return;
	}

	// if we are not playing legato, cheat a little bit to make sure there is always a gap, to make sure gates don't overlap
	if (GatePct > 95 && GatePct < 100) {
		GatePct = 95;
	}

	// calculate the gate length for the step
	auto gateLengthSource = AlgorithmInstance->v[kParamGateLengthSource];
	if (gateLengthSource == 0) {
		auto maxLen = AlgorithmInstance->v[kParamMaxGateLength];
		GateLen = maxLen * GatePct / 100;
	} else if (gateLengthSource == 1) {
		GateLen = ClockRate * GatePct / 100;
		// this cheat helps ensure that the gate will play legato if the gatelen is 100%
		// floating point math and/or slight clock fluctuations could make it not work out that way otherwise
		if (GateLen > 0) {
			GateLen++;
		}
	}
}


void Sequencer::SetupStepValue() {
	// a duration of zero means no glide, just a normal step.  We will adjust duration below, if required
	Glide.OldStepVal = Glide.NewStepVal;
	Glide.NewStepVal = StepVal;
	Glide.Duration = 0;
	Glide.Delta = 0;
}


void Sequencer::DipIfNeccessary() {
	// unless we are playing legato (last step gate len = 100), we have to briefly dip to end the previous gate,
	// no matter the calculated length, unless of course we are already low
	if (LastGatePct < 100 && Outputs.Gate > 0.0) {
		Dip = 2;
	}
}


void Sequencer::ProcessGlide() {
	auto glidePct = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::Glide);
	if (glidePct > 0) {
		// calculate the glide as a percentage of the gate length
		Glide.Duration = GateLen * glidePct / 100;
		Glide.Delta = (Glide.NewStepVal - Glide.OldStepVal) / Glide.Duration;
	} else {
		Glide.Duration = 0;
		Glide.Delta = 0;
	}
}


void Sequencer::ProcessRatchets() {
	auto ratchets = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::Ratchets);
	// only ratchet when our clock is stable, otherwise it will sound weird
	if (ratchets > 0 && StableClock) {
		auto divisions = ratchets + 1;
		auto substep = ClockRate / divisions;
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


void Sequencer::AttenuateGateLength() {
	// if we are using a defined max gate length, we change that to attenuate.  We don't want to double attenuate here
	auto gateLengthSource = AlgorithmInstance->v[kParamGateLengthSource];
	if (gateLengthSource == 0) {
		return;
	}

	auto& param = AlgorithmInstance->parameters[kParamGateLengthAttenuate];
	float scaling = CalculateScaling(param.scaling);
	auto atten = AlgorithmInstance->v[kParamGateLengthAttenuate] / scaling;

	// if we've attenuated down to zero, don't even calculate
	if (atten == 0) {
		GateLen = 0;
		if (Ratchets.Active) {
			Ratchets.GateLen = 0;
		}
		return;
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


void Sequencer::CalculateGate() {
	GateStart = AlgorithmInstance->TotalMs;
	GateEnd = GateStart + GateLen;
}


void Sequencer::HumanizeGate() {
	// only humanize non-legato gates
	if (GatePct < 100) {
		auto& param = AlgorithmInstance->parameters[kParamHumanizeValue];
		float scaling = CalculateScaling(param.scaling);
		auto human = AlgorithmInstance->v[kParamHumanizeValue] / scaling;
		human *= 1000;
		float pct1 = AlgorithmInstance->Random.Next(0, human) / 1000.0f;
		float pct2 = AlgorithmInstance->Random.Next(0, human) / 1000.0f;
		auto off1 = GateLen * pct1 / 100.0f;
		auto off2 = GateLen * pct2 / 100.0f;
		// we always have to start late, because we can't read the future...
		GateStart += off1;
		// but we can end early or late
		off2 = off2 * (AlgorithmInstance->Random.Next(0, 1) == 1 ? -1 : 1);
		GateEnd -= off2;
	}
}


void Sequencer::CalculateVelocity() {
	auto& param = AlgorithmInstance->parameters[kParamVelocityAttenuate];
	float scaling = CalculateScaling(param.scaling);
	auto atten = AlgorithmInstance->v[kParamVelocityAttenuate] / scaling;
	auto offset = AlgorithmInstance->v[kParamVelocityOffset];
	auto velo = Cells[CurrentStep.x][CurrentStep.y].GetField(*AlgorithmInstance, CellDataType::Velocity);
	velo = velo * atten / 100.0f;
	velo += offset;
	Velocity = clamp(static_cast<int>(velo), 1, 127);
}


void Sequencer::HumanizeVelocity() {
	auto& param = AlgorithmInstance->parameters[kParamHumanizeValue];
	float scaling = CalculateScaling(param.scaling);
	auto human = AlgorithmInstance->v[kParamHumanizeValue] / scaling;
	human *= 1000;
	float pct = AlgorithmInstance->Random.Next(0, human) / 1000.0f;
	auto off = Velocity * pct / 100.0f;
	// we can move up or down in velocity
	off = off * (AlgorithmInstance->Random.Next(0, 1) == 1 ? -1 : 1);
	auto velo = Velocity + off;
	// clamp the result to the velocity range
	Velocity = clamp(static_cast<int>(velo), 1, 127);
}


float Sequencer::NormalizeVelocityForOutput() {
	// scale the velocity to a 0-10V range
	return static_cast<float>(Velocity) * 10.0 / 127.0f;
}
