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
public:
	static constexpr uint8_t MaxChannels = 8;

private:

	struct Bank {
		int16_t NoteValues[12];
	};

	// NT Parameter Data
	static const uint8_t GeneralPageDef[];
	static const uint8_t NoteWeightsPageDef[];
	static const char* const PageNamesDef[];
	static const char* const InputNamesDef[];
	static const char* const TriggerNamesDef[];
	static const char* const OutputNamesDef[];
	static const char* const AttenuateNamesDef[];
	static const char* const OffsetNamesDef[];
	static const char* const TransposeNamesDef[];
	static const _NT_specification SpecificationsDef[];
	_NT_parameter ParameterDefs[kWQNumCommonParameters + MaxChannels * kWQNumPerChannelParameters];
	_NT_parameterPages PagesDefs;
	_NT_parameterPage	PageDefs[MaxChannels + 2];
	uint8_t PageParams[MaxChannels * kWQNumPerChannelParameters];
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

	static constexpr uint16_t ShortPressThreshold = 250; // How long (in ms) until a short press turns into a long press

	uint32_t Encoder2DownTime = 0;
	bool Encoder2LongPressFired = false;
	int16_t PreviousBankScanParameterValue = 0;
	uint32_t DelayedTriggers[MaxChannels];

	void Encoder1Turn(int8_t x);
	void Encoder2Turn(int8_t x);

	void Pot1Turn(float val);
	void Pot3Turn(float val);

	void Pot3Push();

	void Encoder2Push();
	void Encoder2Release();

	void Encoder2ShortPress();
	void Encoder2LongPress();

	void Button3Push();
	void Button3Release();

	void ProcessLongPresses();

public:
	static const _NT_factory Factory;

	uint16_t NumChannels;

	HelpTextHelper HelpText;
	QuantizerView QuantView;
	Quantizer Quant;
	Quantizer::QuantRequest QuantRequest;
	Trigger Triggers[MaxChannels];

	const char* QuantizedNoteNames[MaxChannels];
	const char* FinalNoteNames[MaxChannels];
	float OutputValues[MaxChannels];
	bool ScanningLocked = true;
	Bank Banks[10];

	WeightedQuantizer() {}
	~WeightedQuantizer() {}

	void LoadNotesFromBank(size_t bankNum);
	void SaveNotesToBank(size_t bankNum);
	void DoBankScan(int16_t val);

};
