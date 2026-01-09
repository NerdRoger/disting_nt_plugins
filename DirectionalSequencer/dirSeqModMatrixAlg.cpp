#include <new>
#include <string.h>
#include <distingnt/api.h>
#include <distingnt/slot.h>
#include "common.h"
#include "dirSeqAlg.h"
#include "dirSeqModMatrixAlg.h"
#include "cellDefinition.h"


// anonymous namespace for this data keeps the compiler from generating GOT entries, keeps us using internal linkage
namespace {
	static uint8_t ModTargetPageDefs[DirSeqModMatrixAlg::NumMatrices][kParamModTargetStride];

	static size_t constexpr MaxTargetNameLen = 9;
	static char TargetNames[DirSeqModMatrixAlg::NumMatrices][MaxTargetNameLen];

	static const char* const CellDirectionNames[] = {
		"", "North", "NorthEast", "East", "SouthEast", "South", "SouthWest", "West", "NorthWest", 
	};

	// we use enum strings for these because the values are off-by-one
	static const char* const RatchetNames[] = {
		"--", "2", "3", "4", "5", "6", "7", "8", 
	};

	auto CellDefs = CellDefinition::All;
}


void DirSeqModMatrixAlg::BuildModTargetPageDefs() {
	for (int m = 0; m < NumMatrices; m++) {
		for (int i = 0; i < kParamModTargetStride; i++) {
			ModTargetPageDefs[m][i] = m * kParamModTargetStride + i;
		}
	}
}


const char** DirSeqModMatrixAlg::BuildCellTargetEnums() {
	CellTargetEnums[0] = "None";
	for (size_t i = 0; i < static_cast<uint16_t>(CellDataType::NumCellDataTypes); i++) {
		CellTargetEnums[i+1] = CellDefs[i].DisplayName;
	}
	return CellTargetEnums;
}


const char* const CellCoordStrings[] = {
	"", // dummy entry for index 0 since param goes from 1-32, not 0-31
	"Cell (1,1)", "Cell (2,1)", "Cell (3,1)", "Cell (4,1)", "Cell (5,1)", "Cell (6,1)", "Cell (7,1)", "Cell (8,1)", 
	"Cell (1,2)", "Cell (2,2)", "Cell (3,2)", "Cell (4,2)", "Cell (5,2)", "Cell (6,2)", "Cell (7,2)", "Cell (8,2)", 
	"Cell (1,3)", "Cell (2,3)", "Cell (3,3)", "Cell (4,3)", "Cell (5,3)", "Cell (6,3)", "Cell (7,3)", "Cell (8,3)", 
	"Cell (1,4)", "Cell (2,4)", "Cell (3,4)", "Cell (4,4)", "Cell (5,4)", "Cell (6,4)", "Cell (7,4)", "Cell (8,4)", 
};


DirSeqModMatrixAlg::DirSeqModMatrixAlg() {

}


DirSeqModMatrixAlg::~DirSeqModMatrixAlg() {

}


const char* const TriggerValues[] = { "Low", "High" };


void DirSeqModMatrixAlg::BuildParameters() {
	int numPages = 0;

	auto cellTargetEnums = DirSeqModMatrixAlg::BuildCellTargetEnums();

	for (int m = 0; m < NumMatrices; m++) {
		// just fill with spaces for now....  this will actually be set when the target parameter changes down below
		memset(PageNames[m], ' ', MaxPageNameLen);
		PageNames[m][MaxPageNameLen - 1] = 0;
		PageDefs[m] = { .name = PageNames[m], .numParams = kParamModTargetStride, .params = ModTargetPageDefs[m] };

		strncpy(TargetNames[m], "Target ", 8);
		TargetNames[m][7] = 'A' + m;
		TargetNames[m][8] = 0;

		auto matrixIndex = m * kParamModTargetStride;

		ParameterDefs[matrixIndex] = { .name = TargetNames[m], .min = 0, .max = static_cast<uint16_t>(CellDataType::NumCellDataTypes), .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = cellTargetEnums };

		for (int i = 0; i < 32; i++) {
			ParameterDefs[matrixIndex + kParamModTargetCell1 + i] = { .name = CellCoordStrings[i+1], .min = 0, .max = 0, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scalingNone, .enumStrings = NULL };
		}

		ParameterDefs[matrixIndex + kParamModTargetRandomizeRangeA]    = { .name = "Randomize Range A", .min = 0, .max =   0, .def = 0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL };
		ParameterDefs[matrixIndex + kParamModTargetRandomizeRangeB]    = { .name = "Randomize Range B", .min = 0, .max =   0, .def = 0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL };
		ParameterDefs[matrixIndex + kParamModTargetChangeByPercentMax] = { .name = "Change By Max",     .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = kNT_scalingNone, .enumStrings = NULL };
		ParameterDefs[matrixIndex + kParamModTargetActionCellIndex]    = { .name = "Action Cell",       .min = 1, .max =  32, .def = 0, .unit = kNT_unitEnum,    .scaling = kNT_scalingNone, .enumStrings = CellCoordStrings };

		ParameterDefs[matrixIndex + kParamModTargetScrambleAllValuesTrigger]       = { .name = "Scramble All",        .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetInvertAllValuesTrigger]         = { .name = "Invert All",          .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRandomizeAllValuesTrigger]      = { .name = "Randomize All",       .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRandomlyChangeAllValuesTrigger] = { .name = "Randomly Change All", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };

		ParameterDefs[matrixIndex + kParamModTargetInvertCellValueTrigger]               = { .name = "Invert",             .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRandomizeCellValueTrigger]            = { .name = "Randomize",          .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRandomlyChangeCellValueTrigger]       = { .name = "Randomly Change",    .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetSwapWithSurroundingCellValueTrigger]  = { .name = "Swap With Neighbor", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRotateValuesInRowAboutCellTrigger]    = { .name = "Rotate Row",         .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRotateValuesInColumnAboutCellTrigger] = { .name = "Rotate Column",      .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
	}

	numPages += NumMatrices;

	PagesDefs.numPages = numPages;
	PagesDefs.pages = PageDefs;

	parameters = ParameterDefs;
	parameterPages = &PagesDefs;
}


void DirSeqModMatrixAlg::CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications) {
	req.numParameters = NumMatrices * kParamModTargetStride;
	req.sram = 0;
	req.dram = 0;
	req.dtc = 0;
	req.itc = 0;

	// use the memory helper instead of just a normal placement new to ensure proper alignment
	// this becomes important when we start allocating space for other objects here dynamically, so that they are also properly aligned
	// THIS MUST STAY IN SYNC WITH THE CONSTRUCTION REQUIREMENTS IN Construct() BELOW
	MemoryHelper<DirSeqModMatrixAlg>::AlignAndIncrementMemoryRequirement(req.sram, 1);
}


_NT_algorithm* DirSeqModMatrixAlg::Construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications) {
	auto mem = ptrs.sram;
	memset(mem, 0, req.sram);

	BuildModTargetPageDefs();

	// THIS MUST STAY IN SYNC WITH THE REQUIREMENTS OF CALCULATION IN CalculateRequirements() ABOVE
	auto& alg = *MemoryHelper<DirSeqModMatrixAlg>::InitializeDynamicDataAndIncrementPointer(mem, 1);
	alg.BuildParameters();

	return &alg;
}


void DirSeqModMatrixAlg::ParameterChanged(_NT_algorithm* self, int p) {
 	auto& alg = *static_cast<DirSeqModMatrixAlg*>(self);
	auto idx = p % kParamModTargetStride;

	if (idx == kParamModTarget) {
		alg.SetupParametersForTarget(p);
		return;
	}

	// "quantize" the param number to the stride, which gives us the targeting index
	auto modTargetParamIndex = (p / kParamModTargetStride) * kParamModTargetStride;
	DirSeqAlg* seq = alg.GetSequencerAlgorithm();
	
	// if no sequencer algorithm found, nothing left to do
	if (seq == nullptr)
		return;

	// subtract 1 from the target to strip out the "None" (0)
	auto target = alg.v[modTargetParamIndex] - 1;

	// if we are targetting "None", we can bail out
	if (target < 0)
		return;

	auto ct = static_cast<CellDataType>(target);
	auto cd = CellDefs[target];

	if (idx >= kParamModTargetCell1 && idx <= kParamModTargetCell32) {
		auto cellNum = idx - 1;
		auto algIndex = NT_algorithmIndex(&alg);
		_NT_slot slot;
		NT_getSlot(slot, algIndex);
		auto val = static_cast<float>(slot.parameterPresetValue(p + NT_parameterOffset())) / cd.ScalingFactor;
		seq->StepData.SetBaseCellValue(cellNum % GridSizeX, cellNum / GridSizeX, ct, val, false);
		return;
	}

	// always check to see if the sequencer is loaded and the parameter value is high before running triggers.
	// this prevents them from firing when a preset is loading if the value was left High	
	if (alg.v[p] != 1 || !seq->Loaded)
		return;

	auto a = static_cast<float>(alg.v[modTargetParamIndex + kParamModTargetRandomizeRangeA]) / cd.ScalingFactor;
	auto b = static_cast<float>(alg.v[modTargetParamIndex + kParamModTargetRandomizeRangeB]) / cd.ScalingFactor;
	auto changeBy = static_cast<float>(alg.v[modTargetParamIndex + kParamModTargetChangeByPercentMax]);
	auto cellIndex = alg.v[modTargetParamIndex + kParamModTargetActionCellIndex] - 1;

	switch (idx) {
		case kParamModTargetScrambleAllValuesTrigger:
			seq->StepData.ScrambleAllCellValues(ct);
			break;
		case kParamModTargetInvertAllValuesTrigger:
			seq->StepData.InvertAllCellValues(ct);
			break;
		case kParamModTargetRandomizeAllValuesTrigger:
			seq->StepData.RandomizeAllCellValues(ct, min(a, b), max(a, b));
			break;
		case kParamModTargetRandomlyChangeAllValuesTrigger:
			seq->StepData.RandomlyChangeAllCellValues(ct, changeBy);
			break;
		case kParamModTargetInvertCellValueTrigger:
			seq->StepData.InvertCellValue(cellIndex % GridSizeX, cellIndex / GridSizeX, ct);
			break;
		case kParamModTargetRandomizeCellValueTrigger:
			seq->StepData.RandomizeCellValue(cellIndex % GridSizeX, cellIndex / GridSizeX, ct, min(a, b), max(a, b));
			break;
		case kParamModTargetRandomlyChangeCellValueTrigger:
			seq->StepData.RandomlyChangeCellValue(cellIndex % GridSizeX, cellIndex / GridSizeX, ct, changeBy);
			break;
		case kParamModTargetSwapWithSurroundingCellValueTrigger:
			seq->StepData.SwapWithSurroundingCellValue(cellIndex % GridSizeX, cellIndex / GridSizeX, ct);
			break;
		case kParamModTargetRotateValuesInRowAboutCellTrigger:
			seq->StepData.RotateCellValuesInRow(cellIndex / GridSizeX, ct, 1);
			break;
		case kParamModTargetRotateValuesInColumnAboutCellTrigger:
			seq->StepData.RotateCellValuesInColumn(cellIndex % GridSizeX, ct, 1);
			break;
	}
}


bool DirSeqModMatrixAlg::Draw(_NT_algorithm* self) {
	auto& alg = *static_cast<DirSeqModMatrixAlg*>(self);
	DirSeqAlg* seq = alg.GetSequencerAlgorithm();

	if (seq == nullptr) {
		NT_drawText( 20, 10, "This algorithm is a mod matrix expander", 15);
		NT_drawText( 20, 20, "for Directional Sequencer.  Place it", 15);
		NT_drawText( 20, 30, "directly below one to use it!!!", 15);
		return true;
	}

	for (int m = 0; m < NumMatrices; m++) {
		auto modTarget = alg.v[m * kParamModTargetStride + kParamModTarget];
		char buf[17];
		strncpy(buf, "Matrix X Target:", 17);
		buf[7] = 'A' + m;
		buf[16] = 0;
		NT_drawText( 40, 10 * m + 10, buf, 15);
		NT_drawText(140, 10 * m + 10, modTarget == 0 ? "None" : CellDefs[modTarget - 1].DisplayName, 15);
	}

	return true;
}


DirSeqAlg* DirSeqModMatrixAlg::GetSequencerAlgorithm() {
	auto algIndex = NT_algorithmIndex(this);

	_NT_slot slot;
	for (int32_t idx = algIndex - 1; idx >= 0; idx--) {
		if (!NT_getSlot(slot, idx))
			return nullptr;
		// if we encounter another modulator, keep going, as we can have multiple
		if (slot.guid() == DirSeqModMatrixAlg::Guid)
			continue;
		if (slot.guid() == DirSeqAlg::Guid) {
			return static_cast<DirSeqAlg*>(slot.plugin());
		}
		return nullptr;
	}
	return nullptr;
}


void DirSeqModMatrixAlg::SetupParametersForTarget(int modTargetParamIndex) {
	auto algIndex = NT_algorithmIndex(this);
	int16_t modTarget = v[modTargetParamIndex];

	// find the "linked" sequencer algorithm
	DirSeqAlg* seq = GetSequencerAlgorithm();

	size_t pageIndex = modTargetParamIndex / kParamModTargetStride;

	// if "None" target is selected, or we don't have a linked sequencer, just make all the parameters "zeroes"
	// otherwise, configure them to match the cell definition of the target
	if (modTarget == 0 || seq == nullptr) {

		memset(PageNames[pageIndex], ' ', MaxPageNameLen);
		PageNames[pageIndex][MaxPageNameLen - 1] = 0;
		strncpy(PageNames[pageIndex], "* Matrix X *", MaxPageNameLen);
		PageNames[pageIndex][9] = 'A' + pageIndex;

		for (int i = 0; i < 32; i++) {
			ParameterDefs[modTargetParamIndex + 1 + i].name = CellCoordStrings[i + 1];
			ParameterDefs[modTargetParamIndex + 1 + i].min = 0;
			ParameterDefs[modTargetParamIndex + 1 + i].max = 0;
			ParameterDefs[modTargetParamIndex + 1 + i].def = 0;
			ParameterDefs[modTargetParamIndex + 1 + i].unit = kNT_unitNone;
			ParameterDefs[modTargetParamIndex + 1 + i].scaling = kNT_scalingNone;
			ParameterDefs[modTargetParamIndex + 1 + i].enumStrings = NULL;
			NT_updateParameterDefinition(algIndex, modTargetParamIndex + 1 + i);
			NT_setParameterFromAudio(algIndex, modTargetParamIndex + 1 + i + NT_parameterOffset(), 0);
			NT_setParameterGrayedOut(algIndex, modTargetParamIndex + 1 + i + NT_parameterOffset(), true);
		}
		for (int i = kParamModTargetRandomizeRangeA; i < kParamModTargetStride; i++) {
			NT_setParameterFromAudio(algIndex, modTargetParamIndex + i + NT_parameterOffset(), 0);
			NT_setParameterGrayedOut(algIndex, modTargetParamIndex + i + NT_parameterOffset(), true);
		}
	} else {
		// reduce modTarget by 1, since "None" == 0, but everything else is offset by 1
		modTarget--;
		auto cd = CellDefs[modTarget];
		int16_t min = cd.Min * cd.ScalingFactor;
		int16_t max = cd.Max * cd.ScalingFactor;
		int16_t def = cd.Default * cd.ScalingFactor;

		memset(PageNames[pageIndex], ' ', MaxPageNameLen);
		memcpy(PageNames[pageIndex], cd.DisplayName, strlen(cd.DisplayName));
		PageNames[pageIndex][MaxPageNameLen - 2] = 0;

		const char* const *enums;
		switch(static_cast<CellDataType>(modTarget)) {
			using enum CellDataType;
			case Direction:
				enums = CellDirectionNames;
				break;
			case Ratchets:
				enums = RatchetNames;
				break;
			default:
				enums = NULL;
				break;
		}

		uint8_t unit = enums == NULL ? cd.Unit : kNT_unitEnum;
		for (int i = 0; i < 32; i++) {
			auto x = i % 8;
			auto y = i / 8;

			ParameterDefs[modTargetParamIndex + 1 + i].min = min;
			ParameterDefs[modTargetParamIndex + 1 + i].max = max;
			ParameterDefs[modTargetParamIndex + 1 + i].def = def;
			ParameterDefs[modTargetParamIndex + 1 + i].unit = unit;
			ParameterDefs[modTargetParamIndex + 1 + i].scaling = cd.Scaling;
			ParameterDefs[modTargetParamIndex + 1 + i].enumStrings = enums;
			NT_updateParameterDefinition(algIndex, modTargetParamIndex + 1 + i);
			
			// set the parameter value to match the cell's current value
			auto fval = seq->StepData.GetBaseCellValue(x, y, static_cast<CellDataType>(modTarget));
			int16_t val = fval * cd.ScalingFactor;
			NT_setParameterFromAudio(algIndex, modTargetParamIndex + 1 + i + NT_parameterOffset(), val);
			NT_setParameterGrayedOut(algIndex, modTargetParamIndex + 1 + i + NT_parameterOffset(), false);
		}

		for (int i = kParamModTargetRandomizeRangeA; i < kParamModTargetStride; i++) {
			NT_setParameterGrayedOut(algIndex, modTargetParamIndex + i + NT_parameterOffset(), false);
		}

		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeA].min = min;
		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeA].max = max;
		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeA].def = min;
		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeA].unit = unit;
		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeA].scaling = cd.Scaling;
		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeA].enumStrings = enums;
		NT_updateParameterDefinition(algIndex, modTargetParamIndex + kParamModTargetRandomizeRangeA);
		NT_setParameterFromAudio(algIndex, modTargetParamIndex + kParamModTargetRandomizeRangeA + NT_parameterOffset(), min);

		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeB].min = min;
		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeB].max = max;
		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeB].def = max;
		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeB].unit = unit;
		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeB].scaling = cd.Scaling;
		ParameterDefs[modTargetParamIndex + kParamModTargetRandomizeRangeB].enumStrings = enums;
		NT_updateParameterDefinition(algIndex, modTargetParamIndex + kParamModTargetRandomizeRangeB);
		NT_setParameterFromAudio(algIndex, modTargetParamIndex + kParamModTargetRandomizeRangeB + NT_parameterOffset(), max);

	}
}


const _NT_factory DirSeqModMatrixAlg::Factory =
{
	.guid = DirSeqModMatrixAlg::Guid,
	.name = "Dir. Seq. Mod Matrix",
	// TODO:  flesh this out
	.description = "Mod Matrix for Directional Sequencer",
	.numSpecifications = 0,
	.calculateRequirements = CalculateRequirements,
	.construct = Construct,
	.parameterChanged = ParameterChanged,
	.draw = Draw,
	.tags = kNT_tagUtility,
};
