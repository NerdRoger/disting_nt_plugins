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
};


struct DirectionalSequencer : public BaseAlgorithm {
private:

	// NT Parameter Data
	static const char* const EnumStringsMaxGateFrom[];
	static const char* const EnumStringsResetWhenInactive[];
	static const _NT_parameter ParametersDef[];
	static const uint8_t QuantizePageDef[];
	static const uint8_t RoutingPageDef[];
	static const uint8_t SequencerPageDef[];
	static const _NT_parameterPage PagesDef[];
	static const _NT_parameterPages ParameterPagesDef;

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
