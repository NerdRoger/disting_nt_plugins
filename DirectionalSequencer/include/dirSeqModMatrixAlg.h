#pragma once
#include <new>
#include <distingnt/api.h>
#include "cellDefinition.h"


class DirSeqAlg;


enum {
	kParamModTarget,
	kParamModTargetCell1,  kParamModTargetCell2,  kParamModTargetCell3,  kParamModTargetCell4,
	kParamModTargetCell5,  kParamModTargetCell6,  kParamModTargetCell7,  kParamModTargetCell8,
	kParamModTargetCell9,  kParamModTargetCell10, kParamModTargetCell11, kParamModTargetCell12,
	kParamModTargetCell13, kParamModTargetCell14, kParamModTargetCell15, kParamModTargetCell16,
	kParamModTargetCell17, kParamModTargetCell18, kParamModTargetCell19, kParamModTargetCell20,
	kParamModTargetCell21, kParamModTargetCell22, kParamModTargetCell23, kParamModTargetCell24,
	kParamModTargetCell25, kParamModTargetCell26, kParamModTargetCell27, kParamModTargetCell28,
	kParamModTargetCell29, kParamModTargetCell30, kParamModTargetCell31, kParamModTargetCell32,

	kParamModTargetRandomizeRangeA,
	kParamModTargetRandomizeRangeB,
	kParamModTargetChangeByPercentMax,
	kParamModTargetActionCellIndex,

	kParamModTargetScrambleAllValuesTrigger,
	kParamModTargetInvertAllValuesTrigger,
	kParamModTargetRandomizeAllValuesTrigger,
	kParamModTargetRandomlyChangeAllValuesTrigger,
	
	kParamModTargetInvertCellValueTrigger,
	kParamModTargetRandomizeCellValueTrigger,
	kParamModTargetRandomlyChangeCellValueTrigger,
	kParamModTargetSwapWithSurroundingCellValueTrigger,
	kParamModTargetRotateValuesInRowAboutCellTrigger,
	kParamModTargetRotateValuesInColumnAboutCellTrigger,
	
	kParamModTargetStride,
};


struct DirSeqModMatrixAlg : public _NT_algorithm{
public:
	static constexpr uint32_t Guid = NT_MULTICHAR( 'A', 'T', 'd', 'm' );
	static constexpr uint16_t NumMatrices = 4;

private:
	// NT Parameter Data
	static size_t constexpr MaxPageNameLen = 13;
  char PageNames[DirSeqModMatrixAlg::NumMatrices][MaxPageNameLen];
	
	static void BuildModTargetPageDefs();

	_NT_parameter ParameterDefs[NumMatrices * kParamModTargetStride];
	_NT_parameterPages PagesDefs;
	_NT_parameterPage	PageDefs[NumMatrices];

	void BuildParameters();

	// NT factory "methods"
	static void CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications);
	static _NT_algorithm* Construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications);
	static void ParameterChanged(_NT_algorithm* self, int p);
	static bool Draw(_NT_algorithm* self);
	static int ParameterString(_NT_algorithm* self, int p, int v, char* buff);

	DirSeqAlg* GetSequencerAlgorithm();
	void SetupParametersForTarget(int modTargetParamIndex);

public:
	__attribute__((visibility("hidden")))
	static const _NT_factory Factory;

	DirSeqModMatrixAlg();
	~DirSeqModMatrixAlg();

};
