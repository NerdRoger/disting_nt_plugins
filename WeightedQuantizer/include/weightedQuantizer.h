#pragma once
#include <distingnt/api.h>
#include <distingnt/serialisation.h>
#include "baseAlgorithm.h"
#include "helpTextHelper.h"
#include "quantizerView.h"
#include "quantizer.h"


enum {
	// TODO:  fix names once I seperate this completely from the sequencer

	kWQParamTransposeAll,
	kWQParamBankScanPosition,
	kWQParamTriggerSampleDelay,

	kWQParamQuantWeightC,	kWQParamQuantWeightCSharp,
	kWQParamQuantWeightD,	kWQParamQuantWeightDSharp,
	kWQParamQuantWeightE,
	kWQParamQuantWeightF,	kWQParamQuantWeightFSharp,
	kWQParamQuantWeightG,	kWQParamQuantWeightGSharp,
	kWQParamQuantWeightA,	kWQParamQuantWeightASharp,
	kWQParamQuantWeightB,

	kWQNumCommonParameters,
};


enum {
	// TODO:  fix names once I seperate this completely from the sequencer
	kWQParamInput,
	kWQParamTrigger,
	kWQParamOutput,
	kWQParamAttenValue,
	kWQParamOffsetValue,
	kWQParamTranspose,

	kWQNumPerChannelParameters,
};


struct WeightedQuantizer : public BaseAlgorithm {
private:

	struct Bank {
		int16_t NoteValues[12];
	};

	// NT Parameter Data
	static const uint8_t GeneralPageDef[];
	static const uint8_t NoteWeightsPageDef[];
	static const char* const PageNamesDef[];
	static const _NT_specification SpecificationsDef[];
	_NT_parameter* ParameterDefs;
	_NT_parameterPages PagesDefs;
	_NT_parameterPage* PageDefs;
	uint8_t* PageParams;
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
	static bool Deserialise(_NT_algorithm* self, _NT_jsonParse& parse);
	static int  ParameterUiPrefix(_NT_algorithm* self, int p, char* buff);

	int16_t PreviousBankScanParameterValue = 0;
	uint32_t* DelayedTriggers;

public:
	static const _NT_factory Factory;

	uint16_t NumChannels;

	HelpTextHelper HelpText;
	QuantizerView QuantView;
	Quantizer Quant;
	Quantizer::QuantRequest QuantRequest;
	Trigger* Triggers;

	const char** QuantizedNoteNames;
	const char** FinalNoteNames;
	float* OutputValues;
	bool ScanningLocked = true;
	Bank Banks[10];

	WeightedQuantizer() {}
	~WeightedQuantizer() {}

	void LoadNotesFromBank(size_t bankNum);
	void SaveNotesToBank(size_t bankNum);
	void DoBankScan(int16_t val);

};
