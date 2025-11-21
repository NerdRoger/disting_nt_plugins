#pragma once
#include <new>
#include <distingnt/api.h>
#include <distingnt/serialisation.h>
#include "baseAlgorithm.h"
#include "helpTextHelper.h"
#include "cellDefinition.h"
#include "sequencer.h"
#include "gridView.h"


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

	kParamModATarget,
	kParamModATargetCell1,
	kParamModATargetCell2,
	kParamModATargetCell3,
	kParamModATargetCell4,
	kParamModATargetCell5,
	kParamModATargetCell6,
	kParamModATargetCell7,
	kParamModATargetCell8,
	kParamModATargetCell9,
	kParamModATargetCell10,
	kParamModATargetCell11,
	kParamModATargetCell12,
	kParamModATargetCell13,
	kParamModATargetCell14,
	kParamModATargetCell15,
	kParamModATargetCell16,
	kParamModATargetCell17,
	kParamModATargetCell18,
	kParamModATargetCell19,
	kParamModATargetCell20,
	kParamModATargetCell21,
	kParamModATargetCell22,
	kParamModATargetCell23,
	kParamModATargetCell24,
	kParamModATargetCell25,
	kParamModATargetCell26,
	kParamModATargetCell27,
	kParamModATargetCell28,
	kParamModATargetCell29,
	kParamModATargetCell30,
	kParamModATargetCell31,
	kParamModATargetCell32,

	kNumCommonParameters,
};


struct DirectionalSequencer : public BaseAlgorithm {
private:

	// NT Parameter Data
	static const char* const EnumStringsMaxGateFrom[];
	static const char* const EnumStringsResetWhenInactive[];
	static const char* const CellNamesDef[];
	static const uint8_t RoutingPageDef[];
	static const uint8_t SequencerPageDef[];
	static const uint8_t ModATargetPageDef[];
	_NT_parameter ParameterDefs[kNumCommonParameters];
	_NT_parameterPages PagesDefs;
	_NT_parameterPage	PageDefs[3];

	void BuildParameters();

	 bool ModParametersMapped;
	 void MapModParameters(int modTargetParamIndex);

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
	static bool DeserialiseInitialStep(_NT_algorithm* self, _NT_jsonParse& parse);
	static bool DeserialiseGridCellData(_NT_algorithm* self, _NT_jsonParse& parse);
	static bool Deserialise(_NT_algorithm* self, _NT_jsonParse& parse);

public:
	static const _NT_factory Factory;

	RandomGenerator Random;

	// TODO:  Maybe find a better naming scheme for types/members
	GridView Grid;
	HelpTextHelper HelpText;
	Sequencer Seq;

	// TODO:  consider visibility, maybe some of this stuff is private to the algorithm
	Trigger ResetTrigger;
	Trigger ClockTrigger;

	DirectionalSequencer() {}
	~DirectionalSequencer() {}
};
