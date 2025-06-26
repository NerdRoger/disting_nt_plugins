#include <math.h>
#include <new>
#include <distingnt/api.h>
#include <distingnt/serialisation.h>
#include "common.h"
#include "directionalSequencer.h"
#include "sequencer.h"


const char* const DirectionalSequencer::EnumStringsMaxGateFrom[] = { "Max Gate Len", "Clock" };


const char* const DirectionalSequencer::EnumStringsResetWhenInactive[] = { "No", "Yes" };


const _NT_parameter DirectionalSequencer::ParametersDef[] {
	// TODO:  change I/O defaults to final....  these are for ease of development
	NT_PARAMETER_CV_INPUT("Clock", 1, 1)
	NT_PARAMETER_CV_INPUT("Reset", 0, 2)

	NT_PARAMETER_CV_INPUT("Quant Return", 0, 3)

	NT_PARAMETER_CV_OUTPUT("Value",    1, 13)
	NT_PARAMETER_CV_OUTPUT("Gate",     1, 14)
	NT_PARAMETER_CV_OUTPUT("Velocity", 1, 15)

	NT_PARAMETER_CV_OUTPUT("Quant Send", 0, 16)

	{ .name = "Atten. Value", .min = 0,     .max = 1000, .def = 1000, .unit = kNT_unitPercent,   .scaling = kNT_scaling10,   .enumStrings = NULL },
	{ .name = "Offset Value", .min = -5000, .max = 5000, .def = 0,    .unit = kNT_unitVolts,     .scaling = kNT_scaling1000, .enumStrings = NULL },

	{ .name = "Gate Len From",       .min =    0, .max =    1, .def =    1, .unit = kNT_unitEnum,    .scaling = kNT_scalingNone, .enumStrings = EnumStringsMaxGateFrom },
	{ .name = "Max Gate Len",        .min =    0, .max = 1000, .def =  100, .unit = kNT_unitMs,      .scaling = kNT_scalingNone, .enumStrings = NULL },
	{ .name = "Gate Atten. %",       .min =    0, .max = 1000, .def = 1000, .unit = kNT_unitPercent, .scaling = kNT_scaling10,   .enumStrings = NULL },
	{ .name = "Humanize %",          .min =    0, .max =  250, .def =    0, .unit = kNT_unitPercent, .scaling = kNT_scaling10,   .enumStrings = NULL },
	{ .name = "Atten. Velocity",     .min =    0, .max = 1000, .def = 1000, .unit = kNT_unitPercent, .scaling = kNT_scaling10,   .enumStrings = NULL },
	{ .name = "Offset Velocity",     .min = -127, .max =  127, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL },
	{ .name = "Move N Cells",        .min =    1, .max =    7, .def =    1, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL },
	{ .name = "Rest after N steps",  .min =    0, .max =   32, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL },
	{ .name = "Skip after N steps",  .min =    0, .max =   32, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL },
	{ .name = "Reset after N steps", .min =    0, .max =   64, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL },
	{ .name = "Reset when Inactive", .min =    0, .max =    1, .def =    1, .unit = kNT_unitEnum,    .scaling = kNT_scalingNone, .enumStrings = EnumStringsResetWhenInactive },
};


const uint8_t DirectionalSequencer::QuantizePageDef[] = {
	kParamAttenValue, kParamOffsetValue
};


const uint8_t DirectionalSequencer::RoutingPageDef[] = {
	kParamClock, kParamReset, kParamValue, kParamGate, kParamVelocity, kParamQuantSend, kParamQuantReturn
};


const uint8_t DirectionalSequencer::SequencerPageDef[] = {
	kParamGateLengthSource, kParamMaxGateLength, kParamGateLengthAttenuate, kParamHumanizeValue, kParamVelocityAttenuate, 
	kParamVelocityOffset, kParamMoveNCells, kParamRestAfterNSteps, kParamSkipAfterNSteps, kParamResetAfterNSteps, kParamResetWhenInactive
};


const _NT_parameterPage DirectionalSequencer::PagesDef[] = {
	{ .name = "Sequencer", .numParams = ARRAY_SIZE(SequencerPageDef), .params = SequencerPageDef },
	{ .name = "Quantize",  .numParams = ARRAY_SIZE(QuantizePageDef),  .params = QuantizePageDef },
	{ .name = "Routing",   .numParams = ARRAY_SIZE(RoutingPageDef),   .params = RoutingPageDef },
};


const _NT_parameterPages DirectionalSequencer::ParameterPagesDef = {
	.numPages = ARRAY_SIZE(PagesDef),
	.pages = PagesDef,
};


void DirectionalSequencer::CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications) {
	req.numParameters = ARRAY_SIZE(ParametersDef);
	req.sram = sizeof(DirectionalSequencer);
	req.dram = 0;
	req.dtc = 0;
	req.itc = 0;
}


_NT_algorithm* DirectionalSequencer::Construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications) {
	auto& alg = *new (ptrs.sram) DirectionalSequencer();
	alg.parameters = ParametersDef;
	alg.parameterPages = &ParameterPagesDef;

	// "seed" the random sequence
	alg.Random.Seed(NT_getCpuCycleCount());

	alg.Grid.Initialize(alg);
	alg.Seq.Initialize(alg);
	return &alg;
}


void DirectionalSequencer::ParameterChanged(_NT_algorithm* self, int p) {
	auto& alg = *static_cast<DirectionalSequencer*>(self);
	// notify every mode of the parameter change
	alg.Grid.ParameterChanged(p);
}


void DirectionalSequencer::Step(_NT_algorithm* self, float* busFrames, int numFramesBy4) {
	auto& alg = *static_cast<DirectionalSequencer*>(self);
	auto numFrames = numFramesBy4 * 4;

	// the parameter contains the bus number.  convert from 1-based bus numbers to 0-based bus indices
	// this will make the ones makred "None" in the NT a -1 index
	auto clockBusIndex    = alg.v[kParamClock]    - 1; 
	auto resetBusIndex    = alg.v[kParamReset]    - 1; 
	auto valueBusIndex    = alg.v[kParamValue]    - 1; 
	auto gateBusIndex     = alg.v[kParamGate]     - 1; 
	auto velocityBusIndex = alg.v[kParamVelocity] - 1; 

	auto quantSendBusIndex   = alg.v[kParamQuantSend]   - 1; 
	auto quantReturnBusIndex = alg.v[kParamQuantReturn] - 1; 

	// check to see if we are using a return for quant values
	alg.Seq.QuantReturnSupplied = (quantReturnBusIndex >= 0);

	for (int i = 0; i < numFrames; i++) {

		// only send/receive values from legit bus numbers.  If it's marked "None" in NT, we don't want to send/receive

		// process triggers
		if (resetBusIndex >= 0) {
			auto reset = alg.ResetTrigger.Process(busFrames[resetBusIndex * numFrames + i]);
			if (reset == Trigger::Edge::Rising) {
				alg.Seq.ProcessResetTrigger();
			}
		}

		if (clockBusIndex >= 0) {
			auto clock = alg.ClockTrigger.Process(busFrames[clockBusIndex * numFrames + i]);
			if (clock == Trigger::Edge::Rising) {
				alg.Seq.ProcessClockTrigger();
			}
		}

		// process other inputs
		if (alg.Seq.QuantReturnSupplied) {
			alg.Seq.QuantReturn = busFrames[quantReturnBusIndex * numFrames + i];
		}

		// process outputs
		if (valueBusIndex >= 0) {
			busFrames[valueBusIndex * numFrames + i] = alg.Seq.Outputs.Value;
		}

		if (gateBusIndex >= 0) {
			busFrames[gateBusIndex * numFrames + i] = alg.Seq.Outputs.Gate;
		}

		if (velocityBusIndex >= 0) {
			busFrames[velocityBusIndex * numFrames + i] = alg.Seq.Outputs.Velocity;
		}
		
		if (quantSendBusIndex >= 0) {
			busFrames[quantSendBusIndex * numFrames + i] = alg.Seq.Outputs.PreQuantStepVal;
		}
	}

	// we can do this tracking outside of the processing loop, because we don't need sample-level accuracy
	// we are only tracking milliseconds, so block-level accuracy should be fine
	auto deltaMs = alg.CountMilliseconds(numFrames);

	// process the sequencer once per millisecond, we don't need sample-accurate changes
	if (deltaMs > 0) {
		alg.Seq.Process();
	}

}


bool DirectionalSequencer::Draw(_NT_algorithm* self) {
	auto& alg = *static_cast<DirectionalSequencer*>(self);
	// do this in draw, because we don't need it as frequently as step
	alg.Grid.ProcessLongPresses(alg);


// TODO:  remove this at the end of development
	// char buf[15];
	// NT_drawShapeI(kNT_rectangle, 0, 0, 50, 50, 0);
  // NT_drawText(0, 10, alg.Seq.StableClock ? "stable" : "unstable", 15);
	// NT_intToString(buf, alg.Seq.ClockRateFreeze);
	// NT_drawText(0, 20, buf, 15);
	// NT_intToString(buf, alg.Seq.PrevClockRateFreeze);
	// NT_drawText(0, 30, buf, 15);
  // NT_drawText(0, 40, alg.Seq.Ratchets.Active ? "ratchet" : "normal", 15);
	// NT_floatToString(buf, alg.Seq.PercentOff, 3);
	// NT_drawText(0, 50, buf, 15);
	// NT_floatToString(buf, alg.Seq.Delta, 3);
	// NT_drawText(100, 50, buf, 15);


	alg.Grid.Draw();
	return true;
}


uint32_t DirectionalSequencer::HasCustomUI(_NT_algorithm* self) {
	return kNT_potL | kNT_potR | kNT_encoderL | kNT_encoderR | kNT_encoderButtonR | kNT_potButtonR;
}


void DirectionalSequencer::SetupUI(_NT_algorithm* self, _NT_float3& pots) {
	auto& alg = *static_cast<DirectionalSequencer*>(self);
	alg.Grid.FixupPotValues(pots);
}


void DirectionalSequencer::CustomUI(_NT_algorithm* self, const _NT_uiData& data) {
	auto& alg = *static_cast<DirectionalSequencer*>(self);
	alg.Grid.ProcessControlInput(alg, data);
	RecordPreviousPotValues(self, data);
}


void DirectionalSequencer::Serialise(_NT_algorithm* self, _NT_jsonStream& stream) {
	auto& alg = *static_cast<DirectionalSequencer*>(self);

	stream.addMemberName("GridCellData");
	stream.openArray();
	// flatten our 2D array into a 1D array when persisting, so we don't have arrays of arrays in the JSON
	for (size_t x = 0; x < GridSizeX; x++) {
		for (size_t y = 0; y < GridSizeY; y++) {
			stream.openObject();
			for (size_t i = 0; i < ARRAY_SIZE(CellDefinitions); i++) {
				auto cdt = static_cast<CellDataType>(i);
				auto fval = alg.Seq.Cells[x][y].GetField(alg, cdt);
				stream.addMemberName(CellDefinitions[i].FieldName);
				if (CellDefinitions[i].Precision > 0) {
					stream.addNumber(fval);
				} else {
					stream.addNumber(static_cast<int>(fval));
				}
			}
			stream.closeObject();
		}
	}
	stream.closeArray();

	stream.addMemberName("InitialStep");
	stream.openObject();
	{
		stream.addMemberName("x");
		stream.addNumber(alg.Seq.InitialStep.x);
		stream.addMemberName("y");
		stream.addNumber(alg.Seq.InitialStep.y);
	}
	stream.closeObject();

}


bool DirectionalSequencer::DeserialiseInitialStep(_NT_algorithm* self, _NT_jsonParse& parse) {
	auto& alg = *static_cast<DirectionalSequencer*>(self);

	int numMembers;
	if (!parse.numberOfObjectMembers(numMembers)) {
		return false;
	}	

	int val;
	for (int member = 0; member < numMembers; member++) {
		if (parse.matchName("x")) {
			if (!parse.number(val)) {
				return false;
			}
			alg.Seq.InitialStep.x = val;
		} else if (parse.matchName("y")) {
			if (!parse.number(val)) {
				return false;
			}
			alg.Seq.InitialStep.y = val;
		} else {
			if (!parse.skipMember()) {
				return false;
			}
		}
	}

	return true;
}


bool DirectionalSequencer::DeserialiseGridCellData(_NT_algorithm* self, _NT_jsonParse& parse) {
	auto& alg = *static_cast<DirectionalSequencer*>(self);

	int numElements;
	if (!parse.numberOfArrayElements(numElements)) {
		return false;
	}

	// validate we have the right number of cells
	if (numElements != GridSizeX * GridSizeY) {
		return false;
	}

	for (int cellNum = 0; cellNum < numElements; cellNum++) {
		int x = cellNum / GridSizeY;
		int y = cellNum % GridSizeY;
		
		int numMembers;
		if (!parse.numberOfObjectMembers(numMembers)) {
			return false;
		}

		// validate we have the right number of data points
		if (numMembers != ARRAY_SIZE(CellDefinitions)) {
			return false;
		}

		float fval;
		int ival;
		for (int member = 0; member < numMembers; member++) {

			bool found = false;
			for (size_t i = 0; i < ARRAY_SIZE(CellDefinitions); i++) {
				auto cdt = static_cast<CellDataType>(i);
				if (parse.matchName(CellDefinitions[i].FieldName)) {
					if (CellDefinitions[i].Precision > 0) {
						if (!parse.number(fval)) {
							return false;
						}
						alg.Seq.Cells[x][y].SetField(alg, cdt, fval);
					} else {
						if (!parse.number(ival)) {
							return false;
						}
						alg.Seq.Cells[x][y].SetField(alg, cdt, ival);
					}
					found = true;
					break;
				}
			}

			if (!found) {
				if (!parse.skipMember()) {
					return false;
				}
			}

		}
	}

	return true;
}


bool DirectionalSequencer::Deserialise(_NT_algorithm* self, _NT_jsonParse& parse) {
	int num;
	if (!parse.numberOfObjectMembers(num)) {
		return false;
	}

	for (int i = 0; i < num; i++) {
		if (parse.matchName("GridCellData")) {
			if (!DeserialiseGridCellData(self, parse)) {
				return false;
			}
		} else if (parse.matchName("InitialStep")) {
			if (!DeserialiseInitialStep(self, parse)) {
				return false;
			}
		} else {
			if (!parse.skipMember()) {
				return false;
			}
		}
	}

	return true;
}


const _NT_factory DirectionalSequencer::Factory =
{
	.guid = NT_MULTICHAR( 'A', 'T', 'd', 's' ),
	.name = "Directional Sequencer",
	// TODO:  flesh this out
	.description = "Does Stuff",
	.numSpecifications = 0,
	.calculateRequirements = CalculateRequirements,
	.construct = Construct,
	.parameterChanged = ParameterChanged,
	.step = Step,
	.draw = Draw,
	.tags = kNT_tagUtility,
	.hasCustomUi = HasCustomUI,
	.customUi = CustomUI,
	.setupUi = SetupUI,
	.serialise = Serialise,
	.deserialise = Deserialise
};
