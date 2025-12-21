#include <math.h>
#include <new>
#include <string.h>
#include <distingnt/api.h>
#include <distingnt/serialisation.h>
#include "common.h"
#include "dirSeqAlg.h"
#include "stepDataRegion.h"
#include "playhead.h"



const uint8_t DirSeqAlg::SequencerPageDef[] = {
	kParamGateLengthSource, kParamMaxGateLength, kParamGateLengthAttenuate, kParamHumanizeValue,
	kParamAttenValue, kParamOffsetValue, kParamVelocityAttenuate, kParamVelocityOffset,
	kParamMoveNCells, kParamRestAfterNSteps,kParamSkipAfterNSteps, kParamResetAfterNSteps, kParamResetWhenInactive
};


const uint8_t DirSeqAlg::RoutingPageDef[] = {
	kParamClock, kParamReset, kParamValue, kParamGate, kParamVelocity, kParamQuantSend, kParamQuantReturn
};


const char* const DirSeqAlg::EnumStringsMaxGateFrom[] = { "Max Gate Len", "Clock" };


const char* const DirSeqAlg::EnumStringsResetWhenInactive[] = { "No", "Yes" };


DirSeqAlg::DirSeqAlg(const CellDefinition* cellDefs) : Timer(NT_globals.sampleRate), StepData(this, cellDefs), Head(this, &Timer, &Random, &StepData), Grid(cellDefs, &Timer, &Head, &StepData, &HelpText, &PotMgr) {
	CellDefs = cellDefs;
	BuildParameters();
	Grid.Activate();
	Random.Seed(NT_getCpuCycleCount());
}


DirSeqAlg::~DirSeqAlg() {

}


void DirSeqAlg::BuildParameters() {
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

	PagesDefs.numPages = numPages;
	PagesDefs.pages = PageDefs;

	parameters = ParameterDefs;
	parameterPages = &PagesDefs;
}


void DirSeqAlg::CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications) {
	req.numParameters = kNumCommonParameters;
	req.sram = 0;
	req.dram = 0;
	req.dtc = 0;
	req.itc = 0;

	// use the memory helper instead of just a normal placement new to ensure proper alignment
	// this becomes important when we start allocating space for other objects here dynamically, so that they are also properly aligned
	// THIS MUST STAY IN SYNC WITH THE CONSTRUCTION REQUIREMENTS IN Construct() BELOW
	MemoryHelper<DirSeqAlg>::AlignAndIncrementMemoryRequirement(req.sram, 1);
}


_NT_algorithm* DirSeqAlg::Construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications) {
	auto mem = ptrs.sram;

	// THIS MUST STAY IN SYNC WITH THE REQUIREMENTS OF CALCULATION IN CalculateRequirements() ABOVE
	auto& alg = *MemoryHelper<DirSeqAlg>::InitializeDynamicDataAndIncrementPointer(mem, 1, [](DirSeqAlg* addr, size_t){ new (addr) DirSeqAlg(CellDefinition::All); });

	return &alg;
}


void DirSeqAlg::ParameterChanged(_NT_algorithm* self, int p) {
	auto& alg = *static_cast<DirSeqAlg*>(self);
	auto algIndex = NT_algorithmIndex(self);

	// Max Gate Length is only relevant if we are using it to source the gate length
	if (p == kParamGateLengthSource) {
		NT_setParameterGrayedOut(algIndex, kParamMaxGateLength + NT_parameterOffset(), alg.v[kParamGateLengthSource] != 0);
	}

// 	if (p == kParamModATarget) {
// //		alg.MapModParameters(p);
// 	}

	// notify every view of the parameter change
	alg.Grid.ParameterChanged(p);
}


void DirSeqAlg::Step(_NT_algorithm* self, float* busFrames, int numFramesBy4) {
	auto& alg = *static_cast<DirSeqAlg*>(self);
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
	auto deltaMs = alg.Timer.CountMilliseconds(numFrames);

	// process the sequencer once per millisecond, we don't need sample-accurate changes
	if (deltaMs > 0) {
		alg.Head.Process();
	}

}


bool DirSeqAlg::Draw(_NT_algorithm* self) {
	auto& alg = *static_cast<DirSeqAlg*>(self);
	// do this in draw, because we don't need it as frequently as step
	alg.Grid.ProcessLongPresses();


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


uint32_t DirSeqAlg::HasCustomUI(_NT_algorithm* self) {
	return kNT_potL | kNT_potR | kNT_encoderL | kNT_encoderR | kNT_encoderButtonR | kNT_potButtonR;
}


void DirSeqAlg::SetupUI(_NT_algorithm* self, _NT_float3& pots) {
	auto& alg = *static_cast<DirSeqAlg*>(self);
	alg.Grid.FixupPotValues(pots);
	alg.Grid.LoadParamForEditing();
}


void DirSeqAlg::CustomUI(_NT_algorithm* self, const _NT_uiData& data) {
	auto& alg = *static_cast<DirSeqAlg*>(self);
	alg.Grid.ProcessControlInput(data);
	alg.PotMgr.RecordPreviousPotValues(data);
}


void DirSeqAlg::Serialise(_NT_algorithm* self, _NT_jsonStream& stream) {
	auto& alg = *static_cast<DirSeqAlg*>(self);

	stream.addMemberName("GridCellData");
	stream.openArray();
	// flatten our 2D array into a 1D array when persisting, so we don't have arrays of arrays in the JSON
	for (size_t x = 0; x < GridSizeX; x++) {
		for (size_t y = 0; y < GridSizeY; y++) {
			stream.openObject();
			for (size_t i = 0; i < static_cast<size_t>(CellDataType::NumCellDataTypes); i++) {
				auto cdt = static_cast<CellDataType>(i);
				auto fval = alg.StepData.GetBaseCellValue(x, y, cdt);
				stream.addMemberName(alg.CellDefs[i].FieldName);
				if (alg.CellDefs[i].Precision > 0) {
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

	stream.addMemberName("SelectedCell");
	stream.openObject();
	{
		stream.addMemberName("x");
		stream.addNumber(alg.Grid.SelectedCell.x);
		stream.addMemberName("y");
		stream.addNumber(alg.Grid.SelectedCell.y);
	}
	stream.closeObject();

	stream.addMemberName("SelectedParameterIndex");
	stream.addNumber(static_cast<int>(alg.Grid.SelectedParameterIndex));

	stream.addMemberName("Editable");
	stream.addBoolean(alg.Grid.Editable);

}


bool DirSeqAlg::DeserialiseCellCoords(_NT_algorithm* self, _NT_jsonParse& parse, CellCoords& coords) {
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
			coords.x = val;
		} else if (parse.matchName("y")) {
			if (!parse.number(val)) {
				return false;
			}
			coords.y = val;
		} else {
			if (!parse.skipMember()) {
				return false;
			}
		}
	}

	return true;
}


bool DirSeqAlg::DeserialiseGridCellData(_NT_algorithm* self, _NT_jsonParse& parse) {
	auto& alg = *static_cast<DirSeqAlg*>(self);

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
		if (numMembers != static_cast<int>(CellDataType::NumCellDataTypes)) {
			return false;
		}

		float fval;
		int ival;
		for (int member = 0; member < numMembers; member++) {

			bool found = false;
			for (size_t i = 0; i < static_cast<size_t>(CellDataType::NumCellDataTypes); i++) {
				auto cdt = static_cast<CellDataType>(i);
				if (parse.matchName(alg.CellDefs[i].FieldName)) {
					if (alg.CellDefs[i].Precision > 0) {
						if (!parse.number(fval)) {
							return false;
						}
						alg.StepData.SetBaseCellValue(x, y, cdt, fval, true);
					} else {
						if (!parse.number(ival)) {
							return false;
						}
						alg.StepData.SetBaseCellValue(x, y, cdt, ival, true);
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


bool DirSeqAlg::Deserialise(_NT_algorithm* self, _NT_jsonParse& parse) {
	auto& alg = *static_cast<DirSeqAlg*>(self);
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
			if (!DeserialiseCellCoords(self, parse, alg.Head.InitialStep)) {
				return false;
			}
		} else if (parse.matchName("SelectedCell")) {
			if (!DeserialiseCellCoords(self, parse, alg.Grid.SelectedCell)) {
				return false;
			}
		} else if (parse.matchName("SelectedParameterIndex")) {
			int val;
			if (!parse.number(val)) {
				return false;
			}
			alg.Grid.SelectedParameterIndex = static_cast<CellDataType>(val);
			alg.Grid.SelectedParameterIndexRaw = val + 0.5f;
		} else if (parse.matchName("Editable")) {
			bool val;
			if (!parse.boolean(val)) {
				return false;
			}
			alg.Grid.Editable = val;
		} else {
			if (!parse.skipMember()) {
				return false;
			}
		}
	}

	return true;
}


const _NT_factory DirSeqAlg::Factory =
{
	.guid = NT_MULTICHAR( 'A', 'T', 'd', 's' ),
	.name = "Dir. Sequencer",
	// TODO:  flesh this out
	.description = "A 2-D Directional Sequencer",
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
