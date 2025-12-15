#include <math.h>
#include <new>
#include <string.h>
#include <distingnt/api.h>
#include <distingnt/serialisation.h>
#include "common.h"
#include "directionalSequencer.h"
#include "stepDataRegion.h"
#include "playhead.h"



// TODO:  remove this once we get strcat/strncat
void concatenateStrings(char *destination, const char *source) {
	int i = 0;
	// Find the end of the destination string
	while (destination[i] != '\0') {
		i++;
	}

	int j = 0;
	// Copy characters from source to destination
	while (source[j] != '\0') {
		destination[i] = source[j];
		i++;
		j++;
	}
	// Null-terminate the concatenated string
	destination[i] = '\0';
}


const uint8_t DirectionalSequencer::SequencerPageDef[] = {
	kParamGateLengthSource, kParamMaxGateLength, kParamGateLengthAttenuate, kParamHumanizeValue,
	kParamAttenValue, kParamOffsetValue, kParamVelocityAttenuate, kParamVelocityOffset,
	kParamMoveNCells, kParamRestAfterNSteps,kParamSkipAfterNSteps, kParamResetAfterNSteps, kParamResetWhenInactive
};


const uint8_t DirectionalSequencer::RoutingPageDef[] = {
	kParamClock, kParamReset, kParamValue, kParamGate, kParamVelocity, kParamQuantSend, kParamQuantReturn
};


const uint8_t DirectionalSequencer::ModATargetPageDef[] = {
	kParamModATarget,
	kParamModATargetCell1,  kParamModATargetCell2,  kParamModATargetCell3,  kParamModATargetCell4,
	kParamModATargetCell5,  kParamModATargetCell6,  kParamModATargetCell7,  kParamModATargetCell8,
	kParamModATargetCell9,  kParamModATargetCell10, kParamModATargetCell11, kParamModATargetCell12,
	kParamModATargetCell13, kParamModATargetCell14, kParamModATargetCell15, kParamModATargetCell16,
	kParamModATargetCell17, kParamModATargetCell18, kParamModATargetCell19, kParamModATargetCell20,
	kParamModATargetCell21, kParamModATargetCell22, kParamModATargetCell23, kParamModATargetCell24,
	kParamModATargetCell25, kParamModATargetCell26, kParamModATargetCell27, kParamModATargetCell28,
	kParamModATargetCell29, kParamModATargetCell30, kParamModATargetCell31, kParamModATargetCell32,

};


const char* const DirectionalSequencer::EnumStringsMaxGateFrom[] = { "Max Gate Len", "Clock" };


const char* const DirectionalSequencer::EnumStringsResetWhenInactive[] = { "No", "Yes" };


const char* const DirectionalSequencer::CellNamesDef[] = {
	"Cell 1",  "Cell 2",  "Cell 3",  "Cell 4",  "Cell 5",  "Cell 6",  "Cell 7",  "Cell 8", 
	"Cell 9",  "Cell 10", "Cell 11", "Cell 12", "Cell 13", "Cell 14", "Cell 15", "Cell 16", 
	"Cell 17", "Cell 18", "Cell 19", "Cell 20", "Cell 21", "Cell 22", "Cell 23", "Cell 24", 
	"Cell 25", "Cell 26", "Cell 27", "Cell 28", "Cell 29", "Cell 30", "Cell 31", "Cell 32", 
};


void DirectionalSequencer::BuildParameters() {
	int numPages = 0;

	// sequencer page
	PageDefs[numPages] = { .name = "Sequencer", .numParams = ARRAY_SIZE(SequencerPageDef), .params = SequencerPageDef };
	ParameterDefs[kParamGateLengthSource]    = { .name = "Gate Len From",       .min =     0, .max =    1, .def =    1, .unit = kNT_unitEnum,    .scaling = kNT_scalingNone, .enumStrings = EnumStringsMaxGateFrom };
	ParameterDefs[kParamMaxGateLength]       = { .name = "Max Gate Len",        .min =     0, .max = 1000, .def =  100, .unit = kNT_unitMs,      .scaling = kNT_scalingNone, .enumStrings = NULL };
	ParameterDefs[kParamGateLengthAttenuate] = { .name = "Gate Atten. %",       .min =     0, .max = 1000, .def = 1000, .unit = kNT_unitPercent, .scaling = kNT_scaling10,   .enumStrings = NULL };
	ParameterDefs[kParamHumanizeValue]       = { .name = "Humanize %",          .min =     0, .max =  250, .def =    0, .unit = kNT_unitPercent, .scaling = kNT_scaling10,   .enumStrings = NULL };
	ParameterDefs[kParamAttenValue]          = { .name = "Atten. Value",        .min =     0, .max = 1000, .def = 1000, .unit = kNT_unitPercent, .scaling = kNT_scaling10,   .enumStrings = NULL };
	ParameterDefs[kParamOffsetValue]         = { .name = "Offset Value",        .min = -5000, .max = 5000, .def =    0, .unit = kNT_unitVolts,   .scaling = kNT_scaling1000, .enumStrings = NULL };
	ParameterDefs[kParamVelocityAttenuate]   = { .name = "Atten. Velocity",     .min =     0, .max = 1000, .def = 1000, .unit = kNT_unitPercent, .scaling = kNT_scaling10,   .enumStrings = NULL };
	ParameterDefs[kParamVelocityOffset]      = { .name = "Offset Velocity",     .min =  -127, .max =  127, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL };
	ParameterDefs[kParamMoveNCells]          = { .name = "Move N Cells",        .min =     1, .max =    7, .def =    1, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL };
	ParameterDefs[kParamRestAfterNSteps]     = { .name = "Rest after N steps",  .min =     0, .max =   32, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL };
	ParameterDefs[kParamSkipAfterNSteps]     = { .name = "Skip after N steps",  .min =     0, .max =   32, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL };
	ParameterDefs[kParamResetAfterNSteps]    = { .name = "Reset after N steps", .min =     0, .max =   64, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL };
	ParameterDefs[kParamResetWhenInactive]   = { .name = "Reset when Inactive", .min =     0, .max =    1, .def =    1, .unit = kNT_unitEnum,    .scaling = kNT_scalingNone, .enumStrings = EnumStringsResetWhenInactive };
	numPages++;

	// routing page
	// TODO:  change I/O defaults to final....  these are for ease of development
	PageDefs[numPages] = { .name = "Routing",   .numParams = ARRAY_SIZE(RoutingPageDef), .params = RoutingPageDef };
	ParameterDefs[kParamClock]       = { .name = "Clock",        .min = 1, .max = 28, .def = 1,  .unit = kNT_unitCvInput,  .scaling = 0, .enumStrings = NULL };
	ParameterDefs[kParamReset]       = { .name = "Reset",        .min = 0, .max = 28, .def = 2,  .unit = kNT_unitCvInput,  .scaling = 0, .enumStrings = NULL };
	ParameterDefs[kParamValue]       = { .name = "Value",        .min = 0, .max = 28, .def = 13, .unit = kNT_unitCvOutput, .scaling = 0, .enumStrings = NULL };
	ParameterDefs[kParamGate]        = { .name = "Gate",         .min = 0, .max = 28, .def = 14, .unit = kNT_unitCvOutput, .scaling = 0, .enumStrings = NULL };
	ParameterDefs[kParamVelocity]    = { .name = "Velocity",     .min = 0, .max = 28, .def = 15, .unit = kNT_unitCvOutput, .scaling = 0, .enumStrings = NULL };
	ParameterDefs[kParamQuantSend]   = { .name = "Quant Send",   .min = 0, .max = 28, .def = 0,  .unit = kNT_unitCvOutput, .scaling = 0, .enumStrings = NULL };
	ParameterDefs[kParamQuantReturn] = { .name = "Quant Return", .min = 0, .max = 28, .def = 0,  .unit = kNT_unitCvInput,  .scaling = 0, .enumStrings = NULL };
	numPages++;

	// param target A page
	uint8_t numCells = GridSizeX * GridSizeY;

	PageDefs[numPages] = { .name = "Mod A", .numParams = ARRAY_SIZE(ModATargetPageDef), .params = ModATargetPageDef };
	ParameterDefs[kParamModATarget] = { .name = "Mod A Target", .min = 0, .max = ARRAY_SIZE(CellNames) - 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = CellNames };
	auto cd = CellDefinitions[ParameterDefs[kParamModATarget].def];
	for (int i = 0; i < numCells; i++) {
		int16_t min = cd.Min * pow(10, cd.Precision);
		int16_t max = cd.Max * pow(10, cd.Precision);
		int16_t def = cd.Default * pow(10, cd.Precision);
		ParameterDefs[kParamModATargetCell1 + i] = { .name = CellNamesDef[i], .min = min, .max = max, .def = def, .unit = cd.Unit, .scaling = cd.Scaling, .enumStrings = NULL };
	}
	numPages++;


	PagesDefs.numPages = numPages;
	PagesDefs.pages = PageDefs;

	parameters = ParameterDefs;
	parameterPages = &PagesDefs;
}


void DirectionalSequencer::MapModParameters(int modTargetParamIndex) {
	auto algIndex = NT_algorithmIndex(this);
	int16_t modTarget = v[modTargetParamIndex];

	uint8_t numCells = GridSizeX * GridSizeY;

	// TODO:  combine this with loop below once it's working as I expect
	auto cd = CellDefinitions[modTarget];
	for (int i = 0; i < numCells; i++) {
		int16_t min = cd.Min * pow(10, cd.Precision);
		int16_t max = cd.Max * pow(10, cd.Precision);
		int16_t def = cd.Default * pow(10, cd.Precision);
		ParameterDefs[modTargetParamIndex + 1 + i].min = min;
		ParameterDefs[modTargetParamIndex + 1 + i].max = max;
		ParameterDefs[modTargetParamIndex + 1 + i].def = def;
		ParameterDefs[modTargetParamIndex + 1 + i].unit = cd.Unit;
		ParameterDefs[modTargetParamIndex + 1 + i].scaling = cd.Scaling;
		ParameterDefs[modTargetParamIndex + 1 + i].enumStrings = NULL; // TODO:  use for direction
		NT_updateParameterDefinition(algIndex, modTargetParamIndex + 1 + i);
	}


	int multiplier = pow(10, CellDefinitions[modTarget].Precision);
	for (size_t x = 0; x < GridSizeX; x++) {
		for (size_t y = 0; y < GridSizeY; y++) {
			auto cellIndex = y * GridSizeX + x;
			auto fval = StepData.GetBaseCellValue(x, y, static_cast<CellDataType>(modTarget), false);
			int16_t val = fval * multiplier;
			NT_setParameterFromAudio(algIndex, modTargetParamIndex + 1 + cellIndex + NT_parameterOffset(), val);
		}
	}
}


void DirectionalSequencer::CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications) {
	req.numParameters = kNumCommonParameters;
	req.sram = sizeof(DirectionalSequencer);
	req.dram = 0;
	req.dtc = 0;
	req.itc = 0;
}


_NT_algorithm* DirectionalSequencer::Construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications) {
	auto& alg = *new (ptrs.sram) DirectionalSequencer();
	alg.Grid.Initialize(alg);
	alg.StepData.Initialize(alg);
	alg.Head.Initialize(alg);
	alg.BuildParameters();
	alg.Grid.Activate();
	alg.StepData.SetDefaultCellValues();

	// "seed" the random sequence
	alg.Random.Seed(NT_getCpuCycleCount());

	// whenever we construct, we need to sync NT parameters to cell parameters
//	alg.ParametersOutOfSync = true;

	return &alg;
}


void DirectionalSequencer::ParameterChanged(_NT_algorithm* self, int p) {
	auto& alg = *static_cast<DirectionalSequencer*>(self);

	if (p == kParamModATarget) {
		alg.MapModParameters(p);
	}

	// notify every view of the parameter change
	alg.Grid.ParameterChanged(p);
}


void DirectionalSequencer::Step(_NT_algorithm* self, float* busFrames, int numFramesBy4) {
	auto& alg = *static_cast<DirectionalSequencer*>(self);

	// ideally, this would happen from some sort of "doneConstructing" lifecycle event
	// TODO:  can I do this at the end of Construct?
	if (!alg.ModParametersMapped) {
		alg.MapModParameters(kParamModATarget);
		alg.ModParametersMapped = true;
	}

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
	alg.Head.QuantReturnSupplied = (quantReturnBusIndex >= 0);

	for (int i = 0; i < numFrames; i++) {

		// only send/receive values from legit bus numbers.  If it's marked "None" in NT, we don't want to send/receive

		// process triggers
		if (resetBusIndex >= 0) {
			auto reset = alg.Head.ResetTrigger.Process(busFrames[resetBusIndex * numFrames + i]);
			if (reset == Trigger::Edge::Rising) {
				alg.Head.ProcessResetTrigger();
			}
		}

		if (clockBusIndex >= 0) {
			auto clock = alg.Head.ClockTrigger.Process(busFrames[clockBusIndex * numFrames + i]);
			if (clock == Trigger::Edge::Rising) {
				alg.Head.ProcessClockTrigger();
			}
		}

		// process other inputs
		if (alg.Head.QuantReturnSupplied) {
			alg.Head.QuantReturn = busFrames[quantReturnBusIndex * numFrames + i];
		}

		// process outputs
		if (valueBusIndex >= 0) {
			busFrames[valueBusIndex * numFrames + i] = alg.Head.Outputs.Value;
		}

		if (gateBusIndex >= 0) {
			busFrames[gateBusIndex * numFrames + i] = alg.Head.Outputs.Gate;
		}

		if (velocityBusIndex >= 0) {
			busFrames[velocityBusIndex * numFrames + i] = alg.Head.Outputs.Velocity;
		}
		
		if (quantSendBusIndex >= 0) {
			busFrames[quantSendBusIndex * numFrames + i] = alg.Head.Outputs.PreQuantStepVal;
		}
	}

	// we can do this tracking outside of the processing loop, because we don't need sample-level accuracy
	// we are only tracking milliseconds, so block-level accuracy should be fine
	auto deltaMs = alg.CountMilliseconds(numFrames);

	// process the sequencer once per millisecond, we don't need sample-accurate changes
	if (deltaMs > 0) {
		alg.Head.Process();
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
				auto fval = alg.StepData.GetBaseCellValue(x, y, cdt);
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
		stream.addNumber(alg.Head.InitialStep.x);
		stream.addMemberName("y");
		stream.addNumber(alg.Head.InitialStep.y);
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
			alg.Head.InitialStep.x = val;
		} else if (parse.matchName("y")) {
			if (!parse.number(val)) {
				return false;
			}
			alg.Head.InitialStep.y = val;
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
						alg.StepData.SetBaseCellValue(x, y, cdt, fval);
					} else {
						if (!parse.number(ival)) {
							return false;
						}
						alg.StepData.SetBaseCellValue(x, y, cdt, ival);
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
