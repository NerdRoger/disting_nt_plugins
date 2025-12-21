#pragma once
#include <new>
#include <distingnt/api.h>
#include "cellDefinition.h"


class DirectionalSequencerAlgorithm;

enum {
	kParamModATarget,
	kParamModATargetCell1,  kParamModATargetCell2,  kParamModATargetCell3,  kParamModATargetCell4,
	kParamModATargetCell5,  kParamModATargetCell6,  kParamModATargetCell7,  kParamModATargetCell8,
	kParamModATargetCell9,  kParamModATargetCell10, kParamModATargetCell11, kParamModATargetCell12,
	kParamModATargetCell13, kParamModATargetCell14, kParamModATargetCell15, kParamModATargetCell16,
	kParamModATargetCell17, kParamModATargetCell18, kParamModATargetCell19, kParamModATargetCell20,
	kParamModATargetCell21, kParamModATargetCell22, kParamModATargetCell23, kParamModATargetCell24,
	kParamModATargetCell25, kParamModATargetCell26, kParamModATargetCell27, kParamModATargetCell28,
	kParamModATargetCell29, kParamModATargetCell30, kParamModATargetCell31, kParamModATargetCell32,

	kParamModBTarget,
	kParamModBTargetCell1,  kParamModBTargetCell2,  kParamModBTargetCell3,  kParamModBTargetCell4,
	kParamModBTargetCell5,  kParamModBTargetCell6,  kParamModBTargetCell7,  kParamModBTargetCell8,
	kParamModBTargetCell9,  kParamModBTargetCell10, kParamModBTargetCell11, kParamModBTargetCell12,
	kParamModBTargetCell13, kParamModBTargetCell14, kParamModBTargetCell15, kParamModBTargetCell16,
	kParamModBTargetCell17, kParamModBTargetCell18, kParamModBTargetCell19, kParamModBTargetCell20,
	kParamModBTargetCell21, kParamModBTargetCell22, kParamModBTargetCell23, kParamModBTargetCell24,
	kParamModBTargetCell25, kParamModBTargetCell26, kParamModBTargetCell27, kParamModBTargetCell28,
	kParamModBTargetCell29, kParamModBTargetCell30, kParamModBTargetCell31, kParamModBTargetCell32,

	kParamModCTarget,
	kParamModCTargetCell1,  kParamModCTargetCell2,  kParamModCTargetCell3,  kParamModCTargetCell4,
	kParamModCTargetCell5,  kParamModCTargetCell6,  kParamModCTargetCell7,  kParamModCTargetCell8,
	kParamModCTargetCell9,  kParamModCTargetCell10, kParamModCTargetCell11, kParamModCTargetCell12,
	kParamModCTargetCell13, kParamModCTargetCell14, kParamModCTargetCell15, kParamModCTargetCell16,
	kParamModCTargetCell17, kParamModCTargetCell18, kParamModCTargetCell19, kParamModCTargetCell20,
	kParamModCTargetCell21, kParamModCTargetCell22, kParamModCTargetCell23, kParamModCTargetCell24,
	kParamModCTargetCell25, kParamModCTargetCell26, kParamModCTargetCell27, kParamModCTargetCell28,
	kParamModCTargetCell29, kParamModCTargetCell30, kParamModCTargetCell31, kParamModCTargetCell32,

	kParamModDTarget,
	kParamModDTargetCell1,  kParamModDTargetCell2,  kParamModDTargetCell3,  kParamModDTargetCell4,
	kParamModDTargetCell5,  kParamModDTargetCell6,  kParamModDTargetCell7,  kParamModDTargetCell8,
	kParamModDTargetCell9,  kParamModDTargetCell10, kParamModDTargetCell11, kParamModDTargetCell12,
	kParamModDTargetCell13, kParamModDTargetCell14, kParamModDTargetCell15, kParamModDTargetCell16,
	kParamModDTargetCell17, kParamModDTargetCell18, kParamModDTargetCell19, kParamModDTargetCell20,
	kParamModDTargetCell21, kParamModDTargetCell22, kParamModDTargetCell23, kParamModDTargetCell24,
	kParamModDTargetCell25, kParamModDTargetCell26, kParamModDTargetCell27, kParamModDTargetCell28,
	kParamModDTargetCell29, kParamModDTargetCell30, kParamModDTargetCell31, kParamModDTargetCell32,

	kParamModETarget,
	kParamModETargetCell1,  kParamModETargetCell2,  kParamModETargetCell3,  kParamModETargetCell4,
	kParamModETargetCell5,  kParamModETargetCell6,  kParamModETargetCell7,  kParamModETargetCell8,
	kParamModETargetCell9,  kParamModETargetCell10, kParamModETargetCell11, kParamModETargetCell12,
	kParamModETargetCell13, kParamModETargetCell14, kParamModETargetCell15, kParamModETargetCell16,
	kParamModETargetCell17, kParamModETargetCell18, kParamModETargetCell19, kParamModETargetCell20,
	kParamModETargetCell21, kParamModETargetCell22, kParamModETargetCell23, kParamModETargetCell24,
	kParamModETargetCell25, kParamModETargetCell26, kParamModETargetCell27, kParamModETargetCell28,
	kParamModETargetCell29, kParamModETargetCell30, kParamModETargetCell31, kParamModETargetCell32,

	kNumModTargetParameters,
};


struct DirectionalSequencerModMatrixAlgorithm : public _NT_algorithm{
private:

	// stride is the number of cells + 1 (the target parameter)
	static const uint16_t MatrixStride = GridSizeX * GridSizeY + 1;

	// NT Parameter Data
	const char** BuildCellTargetEnums();
	static const char* const CellDirectionNames[];

	char CellParamNames[5][32][20];

	static const uint8_t ModATargetPageDef[];
	static const uint8_t ModBTargetPageDef[];
	static const uint8_t ModCTargetPageDef[];
	static const uint8_t ModDTargetPageDef[];
	static const uint8_t ModETargetPageDef[];
	_NT_parameter ParameterDefs[kNumModTargetParameters];
	_NT_parameterPages PagesDefs;
	_NT_parameterPage	PageDefs[5];

	void BuildParameters();

	// NT factory "methods"
	static void CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications);
	static _NT_algorithm* Construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications);
	static void ParameterChanged(_NT_algorithm* self, int p);
	static bool Draw(_NT_algorithm* self);

	DirectionalSequencerAlgorithm* GetSequencerAlgorithm();
	void SetupParametersForTarget(int modTargetParamIndex);

public:

	const CellDefinition* CellDefs = nullptr;

	static const _NT_factory Factory;

	DirectionalSequencerModMatrixAlgorithm(const CellDefinition* cellDefs);
	~DirectionalSequencerModMatrixAlgorithm();

};
