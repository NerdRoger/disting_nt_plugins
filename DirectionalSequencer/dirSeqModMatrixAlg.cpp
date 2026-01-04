#include <math.h>
#include <new>
#include <string.h>
#include <distingnt/api.h>
#include <distingnt/slot.h>
#include "common.h"
#include "dirSeqAlg.h"
#include "dirSeqModMatrixAlg.h"
#include "cellDefinition.h"


const char* const DirSeqModMatrixAlg::CellDirectionNames[] = {
	"", "North", "NorthEast", "East", "SouthEast", "South", "SouthWest", "West", "NorthWest", 
};


uint8_t ModTargetPageDefs[DirSeqModMatrixAlg::NumMatrices][kParamModTargetStride];
void DirSeqModMatrixAlg::BuildModTargetPageDefs() {
	for (int m = 0; m < NumMatrices; m++) {
		for (int i = 0; i < kParamModTargetStride; i++) {
			ModTargetPageDefs[m][i] = m * kParamModTargetStride + i;
		}
	}
}


const char* CellTargetEnums[static_cast<uint16_t>(CellDataType::NumCellDataTypes) + 1];
const char** DirSeqModMatrixAlg::BuildCellTargetEnums() {
	CellTargetEnums[0] = "None";
	for (size_t i = 0; i < static_cast<uint16_t>(CellDataType::NumCellDataTypes); i++) {
		CellTargetEnums[i+1] = CellDefs[i].DisplayName;
	}
	return CellTargetEnums;
}


DirSeqModMatrixAlg::DirSeqModMatrixAlg(const CellDefinition* cellDefs) {
	CellDefs = cellDefs;
	BuildParameters();
}


DirSeqModMatrixAlg::~DirSeqModMatrixAlg() {

}


const char* const TriggerValues[] = {	"Low", "High" };
char pageNames[DirSeqModMatrixAlg::NumMatrices][9];
char targetNames[DirSeqModMatrixAlg::NumMatrices][9];

void DirSeqModMatrixAlg::BuildParameters() {
	int numPages = 0;

	auto cellTargetEnums = DirSeqModMatrixAlg::BuildCellTargetEnums();

	for (int m = 0; m < NumMatrices; m++) {
		strncpy(pageNames[m], "Matrix ", 8);
		pageNames[m][7] = 'A' + m;
		pageNames[m][8] = 0;
		PageDefs[m] = { .name = pageNames[m], .numParams = kParamModTargetStride, .params = ModTargetPageDefs[m] };

		strncpy(targetNames[m], "Target ", 8);
		targetNames[m][7] = 'A' + m;
		targetNames[m][8] = 0;

		auto matrixIndex = m * kParamModTargetStride;

		ParameterDefs[matrixIndex] = { .name = targetNames[m], .min = 0, .max = static_cast<uint16_t>(CellDataType::NumCellDataTypes), .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = cellTargetEnums };

		for (int i = 0; i < 32; i++) {
			ParameterDefs[matrixIndex + kParamModTargetCell1 + i] = { .name = CellParamNames[0][i], .min = 0, .max = 0, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scalingNone, .enumStrings = NULL };
		}

		ParameterDefs[matrixIndex + kParamModTargetRandomizeRangeA]    = { .name = "Randomize Range A", .min = 0, .max =   0, .def = 0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL };
		ParameterDefs[matrixIndex + kParamModTargetRandomizeRangeB]    = { .name = "Randomize Range B", .min = 0, .max =   0, .def = 0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL };
		ParameterDefs[matrixIndex + kParamModTargetChangeByPercentMax] = { .name = "Change By Max",     .min = 0, .max = 100, .def = 0, .unit = kNT_unitPercent, .scaling = kNT_scalingNone, .enumStrings = NULL };
		ParameterDefs[matrixIndex + kParamModTargetActionCellIndex]    = { .name = "Action Cell #",     .min = 1, .max =  32, .def = 0, .unit = kNT_unitNone,    .scaling = kNT_scalingNone, .enumStrings = NULL };

		ParameterDefs[matrixIndex + kParamModTargetScrambleAllValuesTrigger]       = { .name = "Scramble All",        .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetInvertAllValuesTrigger]         = { .name = "Invert All",          .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRandomizeAllValuesTrigger]      = { .name = "Randomize All",       .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRandomlyChangeAllValuesTrigger] = { .name = "Randomly Change All", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };

		ParameterDefs[matrixIndex + kParamModTargetInvertCellValueTrigger]               = { .name = "Invert",                .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRandomizeCellValueTrigger]            = { .name = "Randomize",             .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRandomlyChangeCellValueTrigger]       = { .name = "Randomly Change",       .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetSwapWithSurroundingCellValueTrigger]  = { .name = "Swap With Surrounding", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRotateValuesInRowAboutCellTrigger]    = { .name = "Rotate Row",            .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
		ParameterDefs[matrixIndex + kParamModTargetRotateValuesInColumnAboutCellTrigger] = { .name = "Rotate Column",         .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = TriggerValues };
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
	auto& alg = *MemoryHelper<DirSeqModMatrixAlg>::InitializeDynamicDataAndIncrementPointer(mem, 1, [](DirSeqModMatrixAlg* addr, size_t){ new (addr) DirSeqModMatrixAlg(CellDefinition::All); });

	return &alg;
}


void DirSeqModMatrixAlg::ParameterChanged(_NT_algorithm* self, int p) {
 	auto& alg = *static_cast<DirSeqModMatrixAlg*>(self);
	DirSeqAlg* seq = alg.GetSequencerAlgorithm();

	auto idx = p % kParamModTargetStride;
	if (idx == kParamModTarget) {
		alg.SetupParametersForTarget(p);
	} else {
		// "quantize" the param number to the stride, which gives us the targeting index
		auto modTargetParamIndex = (p / kParamModTargetStride) * kParamModTargetStride;
		if (seq != nullptr) {
			// subtract 1 from the target to strip out the "None" (0)
			auto target = alg.v[modTargetParamIndex] - 1;
			if (target >= 0) {
				auto ct = static_cast<CellDataType>(target);
				auto cd = alg.CellDefs[target];
				int multiplier = pow(10, cd.Scaling);
				if (idx >= kParamModTargetCell1 && idx <= kParamModTargetCell32) {
					auto cellNum = idx - 1;
					auto algIndex = NT_algorithmIndex(&alg);
					_NT_slot slot;
					NT_getSlot(slot, algIndex);
					auto val = static_cast<float>(slot.parameterPresetValue(p + NT_parameterOffset())) / multiplier;
					seq->StepData.SetBaseCellValue(cellNum % GridSizeX, cellNum / GridSizeX, ct, val, false);
				// always check to see if the sequencer is loaded before running triggers.
				// this prevents them from firing when a preset is loading if the value was left High	
				} else if (idx == kParamModTargetScrambleAllValuesTrigger && alg.v[p] == 1 && seq->Loaded) {
					seq->StepData.ScrambleAllCellValues(ct);
				} else if (idx == kParamModTargetInvertAllValuesTrigger && alg.v[p] == 1 && seq->Loaded) {
					seq->StepData.InvertAllCellValues(ct);
				} else if (idx == kParamModTargetRandomizeAllValuesTrigger && alg.v[p] == 1 && seq->Loaded) {
					auto a = static_cast<float>(alg.v[modTargetParamIndex + kParamModTargetRandomizeRangeA]) / multiplier;
					auto b = static_cast<float>(alg.v[modTargetParamIndex + kParamModTargetRandomizeRangeB]) / multiplier;
					seq->StepData.RandomizeAllCellValues(ct, min(a, b), max(a, b));
				} else if (idx == kParamModTargetRandomlyChangeAllValuesTrigger && alg.v[p] == 1 && seq->Loaded) {
					auto changeBy = static_cast<float>(alg.v[modTargetParamIndex + kParamModTargetChangeByPercentMax]);
					seq->StepData.RandomlyChangeAllCellValues(ct, changeBy);
				} else if (idx == kParamModTargetInvertCellValueTrigger && alg.v[p] == 1 && seq->Loaded) {
					auto cellIndex = alg.v[modTargetParamIndex + kParamModTargetActionCellIndex] - 1;
					seq->StepData.InvertCellValue(cellIndex % GridSizeX, cellIndex / GridSizeX, ct);
				} else if (idx == kParamModTargetRandomizeCellValueTrigger && alg.v[p] == 1 && seq->Loaded) {
					auto cellIndex = alg.v[modTargetParamIndex + kParamModTargetActionCellIndex] - 1;
					auto a = static_cast<float>(alg.v[modTargetParamIndex + kParamModTargetRandomizeRangeA]) / multiplier;
					auto b = static_cast<float>(alg.v[modTargetParamIndex + kParamModTargetRandomizeRangeB]) / multiplier;
					seq->StepData.RandomizeCellValue(cellIndex % GridSizeX, cellIndex / GridSizeX, ct, min(a, b), max(a, b));
				} else if (idx == kParamModTargetRandomlyChangeCellValueTrigger && alg.v[p] == 1 && seq->Loaded) {
					auto cellIndex = alg.v[modTargetParamIndex + kParamModTargetActionCellIndex] - 1;
					auto changeBy = static_cast<float>(alg.v[modTargetParamIndex + kParamModTargetChangeByPercentMax]);
					seq->StepData.RandomlyChangeCellValue(cellIndex % GridSizeX, cellIndex / GridSizeX, ct, changeBy);
				} else if (idx == kParamModTargetSwapWithSurroundingCellValueTrigger && alg.v[p] == 1 && seq->Loaded) {
					auto cellIndex = alg.v[modTargetParamIndex + kParamModTargetActionCellIndex] - 1;
					seq->StepData.SwapWithSurroundingCellValue(cellIndex % GridSizeX, cellIndex / GridSizeX, ct);
				} else if (idx == kParamModTargetRotateValuesInRowAboutCellTrigger && alg.v[p] == 1 && seq->Loaded) {
					auto cellIndex = alg.v[modTargetParamIndex + kParamModTargetActionCellIndex] - 1;
					seq->StepData.RotateCellValuesInRow(cellIndex / GridSizeX, ct, 1);
				} else if (idx == kParamModTargetRotateValuesInColumnAboutCellTrigger && alg.v[p] == 1 && seq->Loaded) {
					auto cellIndex = alg.v[modTargetParamIndex + kParamModTargetActionCellIndex] - 1;
					seq->StepData.RotateCellValuesInColumn(cellIndex % GridSizeX, ct, 1);
				}
			}
		}
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
		NT_drawText(140, 10 * m + 10, modTarget == 0 ? "None" : alg.CellDefs[modTarget - 1].DisplayName, 15);
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
		if (slot.guid() == DirSeqModMatrixAlg::Factory.guid)
			continue;
		if (slot.guid() == DirSeqAlg::Factory.guid) {
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

	// if "None" target is selected, or we don't have a linked sequencer, just make all the parameters "zeroes"
	// otherwise, configure them to match the cell definition of the target
	if (modTarget == 0 || seq == nullptr) {
		for (int i = 0; i < 32; i++) {
			// pad with spaces so the NT UI will leave enough room for when we change the parameter names later
			ParameterDefs[modTargetParamIndex + 1 + i].name = "Unassigned          ";
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
		int16_t min = cd.Min * pow(10, cd.Scaling);
		int16_t max = cd.Max * pow(10, cd.Scaling);
		int16_t def = cd.Default * pow(10, cd.Scaling);
		auto enums = modTarget == 0 ? CellDirectionNames : NULL;
		uint8_t unit = enums == NULL ? cd.Unit : kNT_unitEnum;
		int multiplier = pow(10, cd.Scaling);
		for (int i = 0; i < 32; i++) {
			char numbuf[3];
			NT_intToString(numbuf, i + 1);
			StringConcat(CellParamNames[modTargetParamIndex / kParamModTargetStride][i], 20, cd.DisplayName, " Cell ", numbuf, nullptr);
			ParameterDefs[modTargetParamIndex + 1 + i].name = CellParamNames[modTargetParamIndex / kParamModTargetStride][i];
			ParameterDefs[modTargetParamIndex + 1 + i].min = min;
			ParameterDefs[modTargetParamIndex + 1 + i].max = max;
			ParameterDefs[modTargetParamIndex + 1 + i].def = def;
			ParameterDefs[modTargetParamIndex + 1 + i].unit = unit;
			ParameterDefs[modTargetParamIndex + 1 + i].scaling = cd.Scaling;
			ParameterDefs[modTargetParamIndex + 1 + i].enumStrings = enums;
			NT_updateParameterDefinition(algIndex, modTargetParamIndex + 1 + i);
			
			// set the parameter value to match the cell's current value
			auto x = i % 8;
			auto y = i / 8;
			auto fval = seq->StepData.GetBaseCellValue(x, y, static_cast<CellDataType>(modTarget));
			int16_t val = fval * multiplier;
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
	.guid = NT_MULTICHAR( 'A', 'T', 'd', 'm' ),
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
