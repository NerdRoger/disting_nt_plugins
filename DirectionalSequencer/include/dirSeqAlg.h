#pragma once
#include <new>
#include <distingnt/api.h>
#include <distingnt/serialisation.h>
#include "potManager.h"
#include "helpTextHelper.h"
#include "cellDefinition.h"
#include "stepDataRegion.h"
#include "playhead.h"
#include "gridView.h"
#include "timeKeeper.h"


enum {	
	kParamClock,
	kParamReset,
	kParamQuantReturn,
	kParamValue,
	kParamGate,
	kParamVelocity,
	kParamQuantSend,
	kParamAttenValue,
	kParamOffsetValue,

	kParamGateLengthSource,
	kParamMaxGateLength,
	kParamGateLengthAttenuate,
	kParamHumanizeValue,
	kParamVelocityAttenuate,
	kParamVelocityOffset,
	kParamMoveNCells,
	kParamRestAfterNSteps,
	kParamSkipAfterNSteps,
	kParamResetAfterNSteps,
	kParamResetWhenInactive,

	kNumCommonParameters,
};


struct DirSeqAlg : public _NT_algorithm{
private:

	friend class DirectionalSequencerModulatorAlgorithm;

	// NT Parameter Data
	static const char* const EnumStringsMaxGateFrom[];
	static const char* const EnumStringsResetWhenInactive[];
	static const uint8_t RoutingPageDef[];
	static const uint8_t SequencerPageDef[];
	_NT_parameter ParameterDefs[kNumCommonParameters];
	_NT_parameterPages PagesDefs;
	_NT_parameterPage	PageDefs[3];

	void BuildParameters();

	// NT factory "methods"
	static void CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications);
	static _NT_algorithm* Construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications);
	static void ParameterChanged(_NT_algorithm* self, int p);
	static void Step(_NT_algorithm* self, float* busFrames, int numFramesBy4);
	static bool Draw(_NT_algorithm* self);
	static uint32_t HasCustomUI(_NT_algorithm* self);
	static void SetupUI(_NT_algorithm* self, _NT_float3& pots);
	static void CustomUI(_NT_algorithm* self, const _NT_uiData& data);
	static void Serialise(_NT_algorithm* self, _NT_jsonStream& stream);
	static bool DeserialiseCellCoords(_NT_algorithm* self, _NT_jsonParse& parse, CellCoords& coords);
	static bool DeserialiseGridCellData(_NT_algorithm* self, _NT_jsonParse& parse);
	static bool Deserialise(_NT_algorithm* self, _NT_jsonParse& parse);

public:

	const CellDefinition* CellDefs = nullptr;

	static const _NT_factory Factory;


	// TODO:  Maybe find a better naming scheme for types/members
	// order is important here, as some classes depend on others being constructed first
	TimeKeeper Timer;
	RandomGenerator Random;
	PotManager PotMgr;
	HelpTextHelper HelpText;
	StepDataRegion StepData;
	Playhead Head;
	GridView Grid;

	DirSeqAlg(const CellDefinition* cellDefs);
	~DirSeqAlg();
};
