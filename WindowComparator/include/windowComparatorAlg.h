#pragma once
#include <distingnt/api.h>
#include "common.h"
#include "comparatorView.h"
#include "potManager.h"
#include "helpTextHelper.h"


enum {
	kParamGlobalInputScale,
	kParamGlobalInputOffset,

	kNumCommonParameters,
};


enum {
	kParamInput,
	kParamWindowLeft,
	kParamWindowRight,
	kParamWindowCenter,
	kParamWindowWidth,
	kParamOverrideGlobalAtten,
	kParamInputScale,
	kParamInputOffset,
	kParamInsideWindowGate,
	kParamOutsideWindowGate,
	kParamEnterTrigger,
	kParamExitTrigger,

	kNumPerChannelParameters,
};


enum {
	kParamAllInsideWindowGate,
	kParamAllOutsideWindowGate,

	kNumCommonAggregateParameters,
};


enum {
	kParamExactlyNInsideWindowGate,
	kParamExactlyNOutsideWindowGate,
	kParamAtLeastNInsideWindowGate,
	kParamAtLeastNOutsideWindowGate,

	kNumPerChannelAggregateParameters,
};


struct WindowComparatorAlg : public _NT_algorithm {
private:

	friend struct ComparatorView;

	_NT_parameter* ParameterDefs;
	_NT_parameterPages PagesDefs;
	_NT_parameterPage* PageDefs;
	uint8_t* PageLayout;
	uint8_t* ChannelOffsets;
	bool* PreviouslyInside;
	float* CurrentValues;
	bool* UpdatingBounds;
	bool* UpdatingSizePos;
	uint16_t TriggerRemainingSamples[MAX_BUS_COUNT] = { };
	uint16_t TriggerSampleLength;
	bool FirstStep = true;
	static uint8_t CountParameters(uint8_t numChannels);
	void BuildParameters();
	void BuildParameterPages();
	void InjectDependencies(uint8_t numChannels, const _NT_globals* globals);

	// NT factory "methods"
	static void CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications);
	static _NT_algorithm* Construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications);
	static void ParameterChanged(_NT_algorithm* self, int p);
	static void Step(_NT_algorithm* self, float* busFrames, int numFramesBy4);
	static bool Draw(_NT_algorithm* self);
	static uint32_t HasCustomUI(_NT_algorithm* self);
	static void CustomUI(_NT_algorithm* self, const _NT_uiData& data);
	static void SetupUI(_NT_algorithm* self, _NT_float3& pots);
	static int  ParameterUiPrefix(_NT_algorithm* self, int p, char* buff);

public:
	static constexpr int8_t RangeMin = -10;
	static constexpr int8_t RangeMax = 10;
	static constexpr uint8_t Range = RangeMax - RangeMin;

	HIDDEN static const _NT_factory Factory;

	uint8_t NumChannels;

	TimeKeeper Timer;
	PotManager PotMgr;
	HelpTextHelper HelpText;
	ComparatorView View;

};