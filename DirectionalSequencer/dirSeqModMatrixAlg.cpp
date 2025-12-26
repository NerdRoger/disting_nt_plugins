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


const uint8_t DirSeqModMatrixAlg::ModATargetPageDef[] = {
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


const uint8_t DirSeqModMatrixAlg::ModBTargetPageDef[] = {
	kParamModBTarget,
	kParamModBTargetCell1,  kParamModBTargetCell2,  kParamModBTargetCell3,  kParamModBTargetCell4,
	kParamModBTargetCell5,  kParamModBTargetCell6,  kParamModBTargetCell7,  kParamModBTargetCell8,
	kParamModBTargetCell9,  kParamModBTargetCell10, kParamModBTargetCell11, kParamModBTargetCell12,
	kParamModBTargetCell13, kParamModBTargetCell14, kParamModBTargetCell15, kParamModBTargetCell16,
	kParamModBTargetCell17, kParamModBTargetCell18, kParamModBTargetCell19, kParamModBTargetCell20,
	kParamModBTargetCell21, kParamModBTargetCell22, kParamModBTargetCell23, kParamModBTargetCell24,
	kParamModBTargetCell25, kParamModBTargetCell26, kParamModBTargetCell27, kParamModBTargetCell28,
	kParamModBTargetCell29, kParamModBTargetCell30, kParamModBTargetCell31, kParamModBTargetCell32,
};


const uint8_t DirSeqModMatrixAlg::ModCTargetPageDef[] = {
	kParamModCTarget,
	kParamModCTargetCell1,  kParamModCTargetCell2,  kParamModCTargetCell3,  kParamModCTargetCell4,
	kParamModCTargetCell5,  kParamModCTargetCell6,  kParamModCTargetCell7,  kParamModCTargetCell8,
	kParamModCTargetCell9,  kParamModCTargetCell10, kParamModCTargetCell11, kParamModCTargetCell12,
	kParamModCTargetCell13, kParamModCTargetCell14, kParamModCTargetCell15, kParamModCTargetCell16,
	kParamModCTargetCell17, kParamModCTargetCell18, kParamModCTargetCell19, kParamModCTargetCell20,
	kParamModCTargetCell21, kParamModCTargetCell22, kParamModCTargetCell23, kParamModCTargetCell24,
	kParamModCTargetCell25, kParamModCTargetCell26, kParamModCTargetCell27, kParamModCTargetCell28,
	kParamModCTargetCell29, kParamModCTargetCell30, kParamModCTargetCell31, kParamModCTargetCell32,
};


const uint8_t DirSeqModMatrixAlg::ModDTargetPageDef[] = {
	kParamModDTarget,
	kParamModDTargetCell1,  kParamModDTargetCell2,  kParamModDTargetCell3,  kParamModDTargetCell4,
	kParamModDTargetCell5,  kParamModDTargetCell6,  kParamModDTargetCell7,  kParamModDTargetCell8,
	kParamModDTargetCell9,  kParamModDTargetCell10, kParamModDTargetCell11, kParamModDTargetCell12,
	kParamModDTargetCell13, kParamModDTargetCell14, kParamModDTargetCell15, kParamModDTargetCell16,
	kParamModDTargetCell17, kParamModDTargetCell18, kParamModDTargetCell19, kParamModDTargetCell20,
	kParamModDTargetCell21, kParamModDTargetCell22, kParamModDTargetCell23, kParamModDTargetCell24,
	kParamModDTargetCell25, kParamModDTargetCell26, kParamModDTargetCell27, kParamModDTargetCell28,
	kParamModDTargetCell29, kParamModDTargetCell30, kParamModDTargetCell31, kParamModDTargetCell32,
};


const uint8_t DirSeqModMatrixAlg::ModETargetPageDef[] = {
	kParamModETarget,
	kParamModETargetCell1,  kParamModETargetCell2,  kParamModETargetCell3,  kParamModETargetCell4,
	kParamModETargetCell5,  kParamModETargetCell6,  kParamModETargetCell7,  kParamModETargetCell8,
	kParamModETargetCell9,  kParamModETargetCell10, kParamModETargetCell11, kParamModETargetCell12,
	kParamModETargetCell13, kParamModETargetCell14, kParamModETargetCell15, kParamModETargetCell16,
	kParamModETargetCell17, kParamModETargetCell18, kParamModETargetCell19, kParamModETargetCell20,
	kParamModETargetCell21, kParamModETargetCell22, kParamModETargetCell23, kParamModETargetCell24,
	kParamModETargetCell25, kParamModETargetCell26, kParamModETargetCell27, kParamModETargetCell28,
	kParamModETargetCell29, kParamModETargetCell30, kParamModETargetCell31, kParamModETargetCell32,
};


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


void DirSeqModMatrixAlg::BuildParameters() {
	int numPages = 0;

	auto cellTargetEnums = DirSeqModMatrixAlg::BuildCellTargetEnums();

	PageDefs[numPages + 0] = { .name = "Matrix A", .numParams = ARRAY_SIZE(ModATargetPageDef), .params = ModATargetPageDef };
	PageDefs[numPages + 1] = { .name = "Matrix B", .numParams = ARRAY_SIZE(ModBTargetPageDef), .params = ModBTargetPageDef };
	PageDefs[numPages + 2] = { .name = "Matrix C", .numParams = ARRAY_SIZE(ModCTargetPageDef), .params = ModCTargetPageDef };
	PageDefs[numPages + 3] = { .name = "Matrix D", .numParams = ARRAY_SIZE(ModDTargetPageDef), .params = ModDTargetPageDef };
	PageDefs[numPages + 4] = { .name = "Matrix E", .numParams = ARRAY_SIZE(ModETargetPageDef), .params = ModETargetPageDef };
	ParameterDefs[kParamModATarget] = { .name = "Target A", .min = 0, .max = static_cast<uint16_t>(CellDataType::NumCellDataTypes), .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = cellTargetEnums };
	ParameterDefs[kParamModBTarget] = { .name = "Target B", .min = 0, .max = static_cast<uint16_t>(CellDataType::NumCellDataTypes), .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = cellTargetEnums };
	ParameterDefs[kParamModCTarget] = { .name = "Target C", .min = 0, .max = static_cast<uint16_t>(CellDataType::NumCellDataTypes), .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = cellTargetEnums };
	ParameterDefs[kParamModDTarget] = { .name = "Target D", .min = 0, .max = static_cast<uint16_t>(CellDataType::NumCellDataTypes), .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = cellTargetEnums };
	ParameterDefs[kParamModETarget] = { .name = "Target E", .min = 0, .max = static_cast<uint16_t>(CellDataType::NumCellDataTypes), .def = 0, .unit = kNT_unitEnum, .scaling = kNT_scalingNone, .enumStrings = cellTargetEnums };
	for (int i = 0; i < 32; i++) {
		ParameterDefs[kParamModATargetCell1 + i] = { .name = CellParamNames[0][i], .min = 0, .max = 0, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scalingNone, .enumStrings = NULL };
		ParameterDefs[kParamModBTargetCell1 + i] = { .name = CellParamNames[1][i], .min = 0, .max = 0, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scalingNone, .enumStrings = NULL };
		ParameterDefs[kParamModCTargetCell1 + i] = { .name = CellParamNames[2][i], .min = 0, .max = 0, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scalingNone, .enumStrings = NULL };
		ParameterDefs[kParamModDTargetCell1 + i] = { .name = CellParamNames[3][i], .min = 0, .max = 0, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scalingNone, .enumStrings = NULL };
		ParameterDefs[kParamModETargetCell1 + i] = { .name = CellParamNames[4][i], .min = 0, .max = 0, .def = 0, .unit = kNT_unitNone, .scaling = kNT_scalingNone, .enumStrings = NULL };
	}
	numPages+=5;

	PagesDefs.numPages = numPages;
	PagesDefs.pages = PageDefs;

	parameters = ParameterDefs;
	parameterPages = &PagesDefs;
}


void DirSeqModMatrixAlg::CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications) {
	req.numParameters = kNumModTargetParameters;
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

	// THIS MUST STAY IN SYNC WITH THE REQUIREMENTS OF CALCULATION IN CalculateRequirements() ABOVE
	auto& alg = *MemoryHelper<DirSeqModMatrixAlg>::InitializeDynamicDataAndIncrementPointer(mem, 1, [](DirSeqModMatrixAlg* addr, size_t){ new (addr) DirSeqModMatrixAlg(CellDefinition::All); });

	return &alg;
}


void DirSeqModMatrixAlg::ParameterChanged(_NT_algorithm* self, int p) {
 	auto& alg = *static_cast<DirSeqModMatrixAlg*>(self);
	DirSeqAlg* seq = alg.GetSequencerAlgorithm();

	// a lambda function to update the cell value in the sequencer, called a number of times further down
	auto updateCellValue = [&](size_t modTargetParamIndex) {
		if (seq != nullptr) {
			auto target = alg.v[modTargetParamIndex] - 1;
			if (target >= 0) {
				auto cellNum = (p - 1) % MatrixStride;
				auto cd = alg.CellDefs[target];
				int multiplier = pow(10, cd.Scaling);
				auto algIndex = NT_algorithmIndex(&alg);
				_NT_slot slot;
				NT_getSlot(slot, algIndex);
				auto val = static_cast<float>(slot.parameterPresetValue(p + NT_parameterOffset())) / multiplier;
				seq->StepData.SetBaseCellValue(cellNum % 8, cellNum / 8, static_cast<CellDataType>(target), val, false);
			}
		}
	};

 	if (p == kParamModATarget || p == kParamModBTarget || p == kParamModCTarget || p == kParamModDTarget || p == kParamModETarget) {
		alg.SetupParametersForTarget(p);
 	}
	else if (p >= kParamModATargetCell1 && p <= kParamModATargetCell32) {
		updateCellValue(kParamModATarget);
	}
	else if (p >= kParamModBTargetCell1 && p <= kParamModBTargetCell32) {
		updateCellValue(kParamModBTarget);
	}
	else if (p >= kParamModCTargetCell1 && p <= kParamModCTargetCell32) {
		updateCellValue(kParamModCTarget);
	}
	else if (p >= kParamModDTargetCell1 && p <= kParamModDTargetCell32) {
		updateCellValue(kParamModDTarget);
	}
	else if (p >= kParamModETargetCell1 && p <= kParamModETargetCell32) {
		updateCellValue(kParamModETarget);
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

	auto modTargetA = alg.v[kParamModATarget];
	auto modTargetB = alg.v[kParamModBTarget];
	auto modTargetC = alg.v[kParamModCTarget];
	auto modTargetD = alg.v[kParamModDTarget];
	auto modTargetE = alg.v[kParamModETarget];

	NT_drawText( 40, 10, "Matrix A Target:  ", 15);
	NT_drawText(140, 10, modTargetA == 0 ? "None" : alg.CellDefs[modTargetA - 1].DisplayName, 15);
	
	NT_drawText( 40, 20, "Matrix B Target:  ", 15);
	NT_drawText(140, 20, modTargetB == 0 ? "None" : alg.CellDefs[modTargetB - 1].DisplayName, 15);

	NT_drawText( 40, 30, "Matrix C Target:  ", 15);
	NT_drawText(140, 30, modTargetC == 0 ? "None" : alg.CellDefs[modTargetC - 1].DisplayName, 15);

	NT_drawText( 40, 40, "Matrix D Target:  ", 15);
	NT_drawText(140, 40, modTargetD == 0 ? "None" : alg.CellDefs[modTargetD - 1].DisplayName, 15);

	NT_drawText( 40, 50, "Matrix E Target:  ", 15);
	NT_drawText(140, 50, modTargetE == 0 ? "None" : alg.CellDefs[modTargetE - 1].DisplayName, 15);

	return true;
}


DirSeqAlg* DirSeqModMatrixAlg::GetSequencerAlgorithm() {
	auto algIndex = NT_algorithmIndex(this);

	_NT_slot slot;
	for (int32_t idx = algIndex - 1; idx >= 0; idx--) {
		if (!NT_getSlot(slot, idx))
			return nullptr;
		// if we encounter another modulator, keep going, as we can have multiple
		if (slot.guid() == NT_MULTICHAR( 'A', 'T', 'd', 'm' ))
			continue;
		if (slot.guid() == NT_MULTICHAR( 'A', 'T', 'd', 's' )) {
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
			// pad with spaces do the NT UI will leave enough room for when we change the parameter names later
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
	} else {
		// reduce modTarget by 1, since "None" == 0, but everything else is offset by 1
		modTarget--;
		auto cd = CellDefs[modTarget];
		auto enums = modTarget == 0 ? CellDirectionNames : NULL;
		uint8_t unit = enums == NULL ? cd.Unit : kNT_unitEnum;
		int multiplier = pow(10, cd.Scaling);
		for (int i = 0; i < 32; i++) {
			int16_t min = cd.Min * pow(10, cd.Scaling);
			int16_t max = cd.Max * pow(10, cd.Scaling);
			int16_t def = cd.Default * pow(10, cd.Scaling);
			char numbuf[3];
			NT_intToString(numbuf, i + 1);
			StringConcat(CellParamNames[modTargetParamIndex / MatrixStride][i], 20, cd.DisplayName, " Cell ", numbuf, nullptr);
			ParameterDefs[modTargetParamIndex + 1 + i].name = CellParamNames[modTargetParamIndex / MatrixStride][i];
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
