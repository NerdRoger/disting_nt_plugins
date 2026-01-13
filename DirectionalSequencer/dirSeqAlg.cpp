#include <new>
#include <string.h>
#include <distingnt/api.h>
#include <distingnt/serialisation.h>
#include "common.h"
#include "dirSeqAlg.h"
#include "stepDataRegion.h"
#include "playhead.h"


static const char* const HeadOptionsPageNamesDef[] = {
	"Head A Options", "Head B Options", "Head C Options", "Head D Options", "Head E Options", "Head F Options", "Head G Options", "Head H Options",
};

static const char* HeadRoutingPageNamesDef[] = {
	"Head A Routing", "Head B Routing", "Head C Routing", "Head D Routing", "Head E Routing", "Head F Routing", "Head G Routing", "Head H Routing",
};

static const char* const EnumStringsMaxGateFrom[] = { "Max Gate Len", "Clock" };
static const char* const EnumStringsResetWhenInactive[] = { "No", "Yes" };

static const _NT_specification SpecificationsDef[] = {
	{ .name = "Playheads", .min = 1, .max = 8, .def = 1, .type = kNT_typeGeneric },
};


DirSeqAlg::DirSeqAlg() {

}


DirSeqAlg::~DirSeqAlg() {

}


void DirSeqAlg::StepDataChangedHandler() {
	Grid.LoadParamForEditing();
}


void DirSeqAlg::InjectDependencies(const _NT_globals* globals) {
	Timer.InjectDependencies(globals);
	StepData.InjectDependencies(this);
	Grid.InjectDependencies(this);
}


void DirSeqAlg::BuildParameters() {
	int numPages = 0;

	// any plugin-level parameters would go here


	// start the per-playhead parameters after any common parameters
	uint8_t* pagePtr = PageParams;
	for (int h = 0; h < Playheads.Count; h++) {

		size_t idx = kNumCommonParameters + (kNumPerPlayheadParameters * h);

		auto defineParamAndAddToPage = [this, idx, &pagePtr](size_t offset, const _NT_parameter &param) {
			ParameterDefs[idx + offset] = param;
			pagePtr[0] = idx + offset;
			pagePtr++;
		};

		// define the general parameters for this playhead
		PageDefs[numPages] = { .name = HeadOptionsPageNamesDef[h], .numParams = kNumGeneralPlayheadParameters, .params = pagePtr };
		defineParamAndAddToPage(kParamGateLengthSource,    { .name = "Gate Len From",  .min =     0, .max =    1, .def =    1, .unit = kNT_unitEnum,    .scaling = kNT_scalingNone, .enumStrings = EnumStringsMaxGateFrom });
		defineParamAndAddToPage(kParamMaxGateLength,       { .name = "Max Gate Len",   .min =     0, .max = 1000, .def =  100, .unit = kNT_unitMs,      .scaling = kNT_scalingNone, .enumStrings = NULL });
		defineParamAndAddToPage(kParamGateLengthAttenuate, { .name = "Gate Atten. %",  .min =     0, .max = 1000, .def = 1000, .unit = kNT_unitPercent, .scaling = kNT_scaling10,   .enumStrings = NULL });
		defineParamAndAddToPage(kParamHumanizeValue,       { .name = "Humanize %",     .min =     0, .max =  250, .def =    0, .unit = kNT_unitPercent, .scaling = kNT_scaling10,   .enumStrings = NULL });
		defineParamAndAddToPage(kParamAttenValue,          { .name = "Atten. Value",   .min =     0, .max = 1000, .def = 1000, .unit = kNT_unitPercent, .scaling = kNT_scaling10,   .enumStrings = NULL });
		defineParamAndAddToPage(kParamOffsetValue,         { .name = "Offset Value",   .min = -5000, .max = 5000, .def =    0, .unit = kNT_unitVolts,   .scaling = kNT_scaling1000, .enumStrings = NULL });
		defineParamAndAddToPage(kParamVelocityAttenuate,   { .name = "Atten. Velo.",   .min =     0, .max = 1000, .def = 1000, .unit = kNT_unitPercent, .scaling = kNT_scaling10,   .enumStrings = NULL });
		defineParamAndAddToPage(kParamVelocityOffset,      { .name = "Offset Velo.",   .min =  -127, .max =  127, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL });
		defineParamAndAddToPage(kParamMoveNCells,          { .name = "Move N ",        .min =     1, .max =    7, .def =    1, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL });
		defineParamAndAddToPage(kParamRestAfterNSteps,     { .name = "Rest after N",   .min =     0, .max =   32, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL });
		defineParamAndAddToPage(kParamSkipAfterNSteps,     { .name = "Skip after N",   .min =     0, .max =   32, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL });
		defineParamAndAddToPage(kParamResetAfterNSteps,    { .name = "Reset after N",  .min =     0, .max =   64, .def =    0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL });
		defineParamAndAddToPage(kParamResetWhenInactive,   { .name = "Inactive Reset", .min =     0, .max =    1, .def =    1, .unit = kNT_unitEnum,    .scaling = kNT_scalingNone, .enumStrings = EnumStringsResetWhenInactive });
		
		numPages++;

		// define the routing (I/O) parameters for this playhead
		PageDefs[numPages] = { .name = HeadRoutingPageNamesDef[h], .numParams = kNumIOPlayheadParameters, .params = pagePtr };
		// TODO:  user better defaults once develpmnet is done
		defineParamAndAddToPage(kParamClock,           { .name = "Clock",          .min = 1, .max =  28,   .def =  1,   .unit = kNT_unitCvInput,  .scaling = 0, .enumStrings = NULL });
		defineParamAndAddToPage(kParamClockDivisor,    { .name = "Clock Divisor",  .min = 1, .max = 128,   .def =  1,   .unit = kNT_unitNone,     .scaling = kNT_scalingNone, .enumStrings = NULL });
		defineParamAndAddToPage(kParamClockOffset,     { .name = "Clock Offset",   .min = 0, .max =   0,   .def =  0,   .unit = kNT_unitNone,     .scaling = kNT_scalingNone, .enumStrings = NULL });
		defineParamAndAddToPage(kParamReset,           { .name = "Reset",          .min = 0, .max =  28,   .def =  2,   .unit = kNT_unitCvInput,  .scaling = 0, .enumStrings = NULL });
		defineParamAndAddToPage(kParamValue,           { .name = "Value",          .min = 0, .max =  28,   .def = 13,   .unit = kNT_unitCvOutput, .scaling = 0, .enumStrings = NULL });
		defineParamAndAddToPage(kParamGate,            { .name = "Gate",           .min = 0, .max =  28,   .def = 14,   .unit = kNT_unitCvOutput, .scaling = 0, .enumStrings = NULL });
		defineParamAndAddToPage(kParamVelocityGateMin, { .name = "Velo. Gate Min", .min = 0, .max =  5000, .def = 5000, .unit = kNT_unitVolts,    .scaling = kNT_scaling1000, .enumStrings = NULL });
		defineParamAndAddToPage(kParamVelocity,        { .name = "Velocity",       .min = 0, .max =  28,   .def = 15,   .unit = kNT_unitCvOutput, .scaling = 0, .enumStrings = NULL });
		defineParamAndAddToPage(kParamQuantSend,       { .name = "Quant Send",     .min = 0, .max =  28,   .def =  0,   .unit = kNT_unitCvOutput, .scaling = 0, .enumStrings = NULL });
		defineParamAndAddToPage(kParamQuantReturn,     { .name = "Quant Return",   .min = 0, .max =  28,   .def =  0,   .unit = kNT_unitCvInput,  .scaling = 0, .enumStrings = NULL });
		numPages++;
	}

	PagesDefs.numPages = numPages;
	PagesDefs.pages = PageDefs;

	parameters = ParameterDefs;
	parameterPages = &PagesDefs;
}


void DirSeqAlg::CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications) {
	int32_t numPlayheads = specifications[0];
	req.numParameters = kNumCommonParameters + numPlayheads * kNumPerPlayheadParameters;
	req.sram = 0;
	req.dram = 0;
	req.dtc = 0;
	req.itc = 0;

	// use the memory helper instead of just a normal placement new to ensure proper alignment
	// this becomes important when we start allocating space for other objects here dynamically, so that they are also properly aligned
	// THIS MUST STAY IN SYNC WITH THE CONSTRUCTION REQUIREMENTS IN Construct() BELOW
	MemoryHelper<DirSeqAlg>::AlignAndIncrementMemoryRequirement(req.sram, 1);
	MemoryHelper<Playhead>::AlignAndIncrementMemoryRequirement(req.sram, numPlayheads);
	MemoryHelper<_NT_parameter>::AlignAndIncrementMemoryRequirement(req.sram, req.numParameters);
	MemoryHelper<_NT_parameterPage>::AlignAndIncrementMemoryRequirement(req.sram, numPlayheads * 2);
	MemoryHelper<uint8_t>::AlignAndIncrementMemoryRequirement(req.sram, numPlayheads * kNumPerPlayheadParameters);
}


_NT_algorithm* DirSeqAlg::Construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications) {
	auto numPlayheads = specifications[0];
	auto mem = ptrs.sram;

	// THIS MUST STAY IN SYNC WITH THE REQUIREMENTS OF CALCULATION IN CalculateRequirements() ABOVE
	auto& alg = *MemoryHelper<DirSeqAlg>::InitializeDynamicDataAndIncrementPointer(mem, 1);
	alg.InjectDependencies(&NT_globals);
	auto heads = MemoryHelper<Playhead>::InitializeDynamicDataAndIncrementPointer(mem, numPlayheads);
	alg.Playheads.Init(numPlayheads, heads);
	for (int h = 0; h < alg.Playheads.Count; h++) {
		alg.Playheads[h].InjectDependencies(&alg, h);
	}

	alg.ParameterDefs = MemoryHelper<_NT_parameter>::InitializeDynamicDataAndIncrementPointer(mem, req.numParameters);
	alg.PageDefs = MemoryHelper<_NT_parameterPage>::InitializeDynamicDataAndIncrementPointer(mem, numPlayheads * 2);
	alg.PageParams = MemoryHelper<uint8_t>::InitializeDynamicDataAndIncrementPointer(mem, numPlayheads * kNumPerPlayheadParameters);

	alg.BuildParameters();
	alg.StepData.SetDefaultCellValues();
	alg.Grid.Activate();
	alg.Random.Seed(NT_getCpuCycleCount());

	return &alg;
}


void DirSeqAlg::ParameterChanged(_NT_algorithm* self, int p) {
	auto& alg = *static_cast<DirSeqAlg*>(self);
	auto algIndex = NT_algorithmIndex(self);

	for (int h = 0; h < alg.Playheads.Count; h++) {
		int idx = kNumCommonParameters + (kNumPerPlayheadParameters * h);
		
		// Max Gate Length is only relevant if we are using it to source the gate length
		if (p == idx + kParamGateLengthSource) {
			NT_setParameterGrayedOut(algIndex, idx + kParamMaxGateLength + NT_parameterOffset(), alg.v[p] != 0);
			NT_setParameterGrayedOut(algIndex, idx + kParamGateLengthAttenuate + NT_parameterOffset(), alg.v[p] == 0);
		}

		// Clock Offset Max should never equal or exceed Clock Divisor
		if (p == idx + kParamClockDivisor) {
			NT_setParameterGrayedOut(algIndex, idx + kParamClockOffset + NT_parameterOffset(), alg.v[p] == 1);
			auto newMax = alg.v[p] - 1;
			alg.ParameterDefs[idx + kParamClockOffset].max = newMax;
			NT_updateParameterDefinition(algIndex, idx + kParamClockOffset);
			auto newVal = clamp(alg.v[idx + kParamClockOffset], static_cast<int16_t>(0), static_cast<int16_t>(newMax));
			NT_setParameterFromAudio(algIndex, idx + kParamClockOffset + NT_parameterOffset(), newVal);
		}
	}

	// notify every view of the parameter change
	alg.Grid.ParameterChanged(p);
}


void DirSeqAlg::Step(_NT_algorithm* self, float* busFrames, int numFramesBy4) {
	auto& alg = *static_cast<DirSeqAlg*>(self);
	alg.Loaded = true;
	auto numFrames = numFramesBy4 * 4;

	// a lambda to get the value of a parameter for a particular playhead
	auto getPlayheadParamVal = [&alg](size_t headIdx, size_t offset) {
		return alg.v[kNumCommonParameters + (kNumPerPlayheadParameters * headIdx) + offset];
	};

	for (int h = 0; h < alg.Playheads.Count; h++) {
		// the parameter contains the bus number.  convert from 1-based bus numbers to 0-based bus indices
		// this will make the ones marked "None" in the NT a -1 index
		auto clockBusIndex    = getPlayheadParamVal(h, kParamClock)    - 1; 
		auto resetBusIndex    = getPlayheadParamVal(h, kParamReset)    - 1; 
		auto valueBusIndex    = getPlayheadParamVal(h, kParamValue)    - 1; 
		auto gateBusIndex     = getPlayheadParamVal(h, kParamGate)     - 1; 
		auto velocityBusIndex = getPlayheadParamVal(h, kParamVelocity) - 1; 

		auto quantSendBusIndex   = getPlayheadParamVal(h, kParamQuantSend)   - 1; 
		auto quantReturnBusIndex = getPlayheadParamVal(h, kParamQuantReturn) - 1; 

		// check to see if we are using a return for quant values
		alg.Playheads[h].QuantReturnSupplied = (quantReturnBusIndex >= 0);

		for (int i = 0; i < numFrames; i++) {
			
			// only send/receive values from legit bus numbers.  If it's marked "None" in NT, we don't want to send/receive

			// process triggers
			if (resetBusIndex >= 0) {
				auto reset = alg.Playheads[h].ResetTrigger.Process(busFrames[resetBusIndex * numFrames + i]);
				if (reset == Trigger::Edge::Rising) {
					alg.Playheads[h].ProcessResetTrigger();
				}
			}
			if (clockBusIndex >= 0) {
				auto clock = alg.Playheads[h].ClockTrigger.Process(busFrames[clockBusIndex * numFrames + i]);
				if (clock == Trigger::Edge::Rising) {
					alg.Playheads[h].ProcessClockTrigger();
				}
			}

			// process other inputs
			if (alg.Playheads[h].QuantReturnSupplied) {
				alg.Playheads[h].QuantReturn = busFrames[quantReturnBusIndex * numFrames + i];
			}

			// process outputs
			if (valueBusIndex >= 0) {
				busFrames[valueBusIndex * numFrames + i] = alg.Playheads[h].Outputs.Value;
			}
			if (gateBusIndex >= 0) {
				busFrames[gateBusIndex * numFrames + i] = alg.Playheads[h].Outputs.Gate;
			}
			if (velocityBusIndex >= 0) {
				busFrames[velocityBusIndex * numFrames + i] = alg.Playheads[h].Outputs.Velocity;
			}
			if (quantSendBusIndex >= 0) {
				busFrames[quantSendBusIndex * numFrames + i] = alg.Playheads[h].Outputs.PreQuantStepVal;
			}
		}
	}

	// we can do this tracking outside of the processing loop, because we don't need sample-level accuracy
	// we are only tracking milliseconds, so block-level accuracy should be fine
	auto deltaMs = alg.Timer.CountMilliseconds(numFrames);

	// process the sequencer once per millisecond, we don't need sample-accurate changes
	if (deltaMs > 0) {
		for (int h = 0; h < alg.Playheads.Count; h++) {
			alg.Playheads[h].Process();
		}
	}

}


bool DirSeqAlg::Draw(_NT_algorithm* self) {
	auto& alg = *static_cast<DirSeqAlg*>(self);
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
				stream.addMemberName(CellDefinition::All[i].FieldName);
				if (CellDefinition::All[i].Scaling > 0) {
					stream.addNumber(fval);
				} else {
					stream.addNumber(static_cast<int>(fval));
				}
			}
			stream.closeObject();
		}
	}
	stream.closeArray();

	stream.addMemberName("InitialSteps");
	stream.openArray();
	for (int h = 0; h < alg.Playheads.Count; h++) {
		stream.openObject();
		stream.addMemberName("x");
		stream.addNumber(alg.Playheads[h].InitialStep.x);
		stream.addMemberName("y");
		stream.addNumber(alg.Playheads[h].InitialStep.y);
		stream.closeObject();
	}
	stream.closeArray();

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

	stream.addMemberName("SelectedPlayheadIndex");
	stream.addNumber(static_cast<int>(alg.Grid.SelectedPlayheadIndex));

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
				if (parse.matchName(CellDefinition::All[i].FieldName)) {
					if (CellDefinition::All[i].Scaling > 0) {
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
		} else if (parse.matchName("InitialSteps")) {


			int numElements;
			if (!parse.numberOfArrayElements(numElements)) {
				return false;
			}
			for (int i = 0; i < numElements; i++)	{
				if (i < alg.Playheads.Count) {
					if (!DeserialiseCellCoords(self, parse, alg.Playheads[i].InitialStep)) {
						return false;
					}
				} else {
					// this can happen when we are respecifying downward...
					// the previous version has more entries than we now need, but we still need to parse them to progress the stream
					CellCoords throwaway;
					DeserialiseCellCoords(self, parse, throwaway);
				}
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
		} else if (parse.matchName("SelectedPlayheadIndex")) {
			int val;
			if (!parse.number(val)) {
				return false;
			}
			alg.Grid.SelectedPlayheadIndex = val;
			alg.Grid.SelectedPlayheadIndexRaw = val + 0.5f;
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


int DirSeqAlg::ParameterUiPrefix(_NT_algorithm* self, int p, char* buff) {
	if (p >= kNumCommonParameters) {
		auto h = (p - kNumCommonParameters) / kNumPerPlayheadParameters;
		buff[0] = 'A' + h;
		buff[1] = ':';
		buff[2] = 0;
	}
	return 3;
}


const _NT_factory DirSeqAlg::Factory =
{
	.guid = DirSeqAlg::Guid,
	.name = "Dir. Sequencer",
	.description = "A 2-D Directional Sequencer",
	.numSpecifications = ARRAY_SIZE(SpecificationsDef),
	.specifications = SpecificationsDef,
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
	.deserialise = Deserialise,
	.parameterUiPrefix = ParameterUiPrefix,
};
