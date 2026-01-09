#include <math.h>
#include <distingnt/api.h>
#include <distingnt/serialisation.h>
#include "common.h"
#include "weightedQuantizerAlg.h"
#include "quantizer.h"


// anonymous namespace for this data keeps the compiler from generating GOT entries, keeps us using internal linkage
namespace {
	const uint8_t GeneralPageDef[] = {
		kWQParamTransposeAll,
		kWQParamBankScanPosition,
		kWQParamTriggerSampleDelay,
	};

	const uint8_t NoteWeightsPageDef[] = {
		kWQParamQuantWeightC,	kWQParamQuantWeightCSharp,
		kWQParamQuantWeightD,	kWQParamQuantWeightDSharp,
		kWQParamQuantWeightE,
		kWQParamQuantWeightF,	kWQParamQuantWeightFSharp,
		kWQParamQuantWeightG,	kWQParamQuantWeightGSharp,
		kWQParamQuantWeightA,	kWQParamQuantWeightASharp,
		kWQParamQuantWeightB,
	};

	const char* const PageNamesDef[] = {
		"Channel 1", "Channel 2", "Channel 3", "Channel 4", "Channel 5", "Channel 6", "Channel 7", "Channel 8",
	};

	const _NT_specification SpecificationsDef[] = {
		{ .name = "Channels", .min = 1, .max = 8, .def = 1, .type = kNT_typeGeneric },
	};
}


WeightedQuantizerAlg::WeightedQuantizerAlg() {

}


WeightedQuantizerAlg::~WeightedQuantizerAlg() {

}


void WeightedQuantizerAlg::InjectDependencies(uint16_t numChannels, const _NT_globals* globals) {
	NumChannels = numChannels;
	Timer.InjectDependencies(globals);
	Banks.InjectDependencies(this);
	QuantView.InjectDependencies(this);
}


void WeightedQuantizerAlg::BuildParameters() {
	int numPages = 0;

	// general page
	PageDefs[numPages] = { .name = "General", .numParams = ARRAY_SIZE(GeneralPageDef), .params = GeneralPageDef };
	ParameterDefs[kWQParamTransposeAll]       = { .name = "Transpose All",    .min =  -60, .max =    60, .def =    0, .unit = kNT_unitSemitones, .scaling = kNT_scalingNone, .enumStrings = NULL };
	ParameterDefs[kWQParamBankScanPosition]   = { .name = "Bank Scan",        .min = 1000, .max = 10000, .def = 1000, .unit = kNT_unitNone,      .scaling = kNT_scaling1000, .enumStrings = NULL };
	ParameterDefs[kWQParamTriggerSampleDelay] = { .name = "Trig Samp. Delay", .min =    0, .max =    10, .def =    2, .unit = kNT_unitNone,      .scaling = kNT_scalingNone, .enumStrings = NULL };
	numPages++;

	// note weights page
	PageDefs[numPages] = { .name = "Note Weights", .numParams = ARRAY_SIZE(NoteWeightsPageDef), .params = NoteWeightsPageDef };
	ParameterDefs[kWQParamQuantWeightC]      = { .name = "C",  .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	ParameterDefs[kWQParamQuantWeightCSharp] = { .name = "C#", .min = 0, .max = 1000, .def =    0, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	ParameterDefs[kWQParamQuantWeightD]      = { .name = "D",  .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	ParameterDefs[kWQParamQuantWeightDSharp] = { .name = "D#", .min = 0, .max = 1000, .def =    0, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	ParameterDefs[kWQParamQuantWeightE]      = { .name = "E",  .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	ParameterDefs[kWQParamQuantWeightF]      = { .name = "F",  .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	ParameterDefs[kWQParamQuantWeightFSharp] = { .name = "F#", .min = 0, .max = 1000, .def =    0, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	ParameterDefs[kWQParamQuantWeightG]      = { .name = "G",  .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	ParameterDefs[kWQParamQuantWeightGSharp] = { .name = "G#", .min = 0, .max = 1000, .def =    0, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	ParameterDefs[kWQParamQuantWeightA]      = { .name = "A",  .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	ParameterDefs[kWQParamQuantWeightASharp] = { .name = "A#", .min = 0, .max = 1000, .def =    0, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	ParameterDefs[kWQParamQuantWeightB]      = { .name = "B",  .min = 0, .max = 1000, .def = 1000, .unit = kNT_unitNone, .scaling = kNT_scaling100, .enumStrings = NULL };
	numPages++;

	size_t idx = kWQNumCommonParameters;
	uint8_t* pagePtr = PageParams;
	for (int16_t i = 0; i < NumChannels; i++) {
		// inputs and outputs
		ParameterDefs[idx + kWQParamInput]   = { .name = "Input",   .min = 0, .max = 28, .def = static_cast<int16_t>(i + 1),  .unit = kNT_unitCvInput,  .scaling = 0, .enumStrings = NULL };
		ParameterDefs[idx + kWQParamTrigger] = { .name = "Trigger", .min = 0, .max = 28, .def = 0,                            .unit = kNT_unitCvInput,  .scaling = 0, .enumStrings = NULL };
		ParameterDefs[idx + kWQParamOutput]  = { .name = "Output",  .min = 0, .max = 28, .def = static_cast<int16_t>(i + 13), .unit = kNT_unitCvOutput, .scaling = 0, .enumStrings = NULL };
		// parameters
		ParameterDefs[idx + kWQParamAttenValue]  = { .name = "(Pre) Attenuate",  .min = 0,     .max = 1000, .def = 1000, .unit = kNT_unitPercent,   .scaling = kNT_scaling10,   .enumStrings = NULL };
		ParameterDefs[idx + kWQParamOffsetValue] = { .name = "(Pre) Offset",     .min = -5000, .max = 5000, .def = 0,    .unit = kNT_unitVolts,     .scaling = kNT_scaling1000, .enumStrings = NULL };
		ParameterDefs[idx + kWQParamTranspose]   = { .name = "(Post) Transpose", .min = -60,   .max = 60,   .def = 0,    .unit = kNT_unitSemitones, .scaling = kNT_scalingNone, .enumStrings = NULL };

		for (int j = 0; j < kWQNumPerChannelParameters; j++) {
			pagePtr[j] = idx + j;
		}

		PageDefs[numPages] = { .name = PageNamesDef[i], .numParams = kWQNumPerChannelParameters, .params = pagePtr };

		pagePtr += kWQNumPerChannelParameters;
		idx += kWQNumPerChannelParameters;
		numPages++;
	}

	PagesDefs.numPages = numPages;
	PagesDefs.pages = PageDefs;

	parameters = ParameterDefs;
	parameterPages = &PagesDefs;
}


void WeightedQuantizerAlg::CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications) {
	int32_t numChannels = specifications[0];
	req.numParameters = kWQNumCommonParameters + numChannels * kWQNumPerChannelParameters;
	req.sram = 0;
	req.dram = 0;
	req.dtc = 0;
	req.itc = 0;

	// calculate the memory requirements for all of the objects and dynamic arrays we need to create
	// THIS MUST STAY IN SYNC WITH THE CONSTRUCTION REQUIREMENTS IN Construct() BELOW
	MemoryHelper<Quantizer::QuantResult>::AlignAndIncrementMemoryRequirement(req.sram, numChannels); // WeightedQuantizer::QuantResults
	MemoryHelper<WeightedQuantizerAlg>::AlignAndIncrementMemoryRequirement(req.sram, 1); // WeightedQuantizer
	MemoryHelper<Trigger>::AlignAndIncrementMemoryRequirement(req.sram, numChannels); // WeightedQuantizer::Triggers
	MemoryHelper<uint32_t>::AlignAndIncrementMemoryRequirement(req.sram, numChannels); // WeightedQuantizer::DelayedTriggers
	MemoryHelper<_NT_parameter>::AlignAndIncrementMemoryRequirement(req.sram, req.numParameters); // WeightedQuantizer::ParameterDefs
	MemoryHelper<_NT_parameterPage>::AlignAndIncrementMemoryRequirement(req.sram, numChannels + 2); // WeightedQuantizer::PageDefs
	MemoryHelper<uint8_t>::AlignAndIncrementMemoryRequirement(req.sram, numChannels * kWQNumPerChannelParameters); // WeightedQuantizer::PageParams
}


_NT_algorithm* WeightedQuantizerAlg::Construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications) {
	auto numChannels = specifications[0];
	auto mem = ptrs.sram;

	// initialize arrays that depend on number of channels
	// THIS MUST STAY IN SYNC WITH THE REQUIREMENTS OF CALCULATION IN CalculateRequirements() ABOVE
	auto& alg = *MemoryHelper<WeightedQuantizerAlg>::InitializeDynamicDataAndIncrementPointer(mem, 1);
	alg.Triggers = MemoryHelper<Trigger>::InitializeDynamicDataAndIncrementPointer(mem, numChannels);
	alg.QuantResults = MemoryHelper<Quantizer::QuantResult>::InitializeDynamicDataAndIncrementPointer(mem, numChannels);
	alg.DelayedTriggers = MemoryHelper<uint32_t>::InitializeDynamicDataAndIncrementPointer(mem, numChannels);
	alg.ParameterDefs = MemoryHelper<_NT_parameter>::InitializeDynamicDataAndIncrementPointer(mem, req.numParameters);
	alg.PageDefs = MemoryHelper<_NT_parameterPage>::InitializeDynamicDataAndIncrementPointer(mem, numChannels + 2);
	alg.PageParams = MemoryHelper<uint8_t>::InitializeDynamicDataAndIncrementPointer(mem, numChannels * kWQNumPerChannelParameters);
	alg.InjectDependencies(numChannels, &NT_globals);

	alg.BuildParameters();
	alg.QuantView.Activate();

	return &alg;
}


void WeightedQuantizerAlg::ParameterChanged(_NT_algorithm* self, int p) {
	auto& alg = *static_cast<WeightedQuantizerAlg*>(self);
	alg.QuantView.ParameterChanged(p);

	// if one of the note weight parameters changes, set it on the quantizer, so we don't have to constantly interrogate it
	if (p >= kWQParamQuantWeightC && p < kWQNumCommonParameters) {
		auto val = GetScaledParameterValue(alg, p);
		alg.Quant.NoteWeights[p - kWQParamQuantWeightC] = val;
	}

	// if the bank scanning position changed, we need to adjust the weightings
	if (p == kWQParamBankScanPosition) {
		auto val = alg.v[p];
		alg.Banks.DoBankScan(val);
	}
}


void WeightedQuantizerAlg::Step(_NT_algorithm* self, float* busFrames, int numFramesBy4) {
	auto& alg = *static_cast<WeightedQuantizerAlg*>(self);
	auto numFrames = numFramesBy4 * 4;

	auto ms = alg.Timer.CountMilliseconds(numFrames);

	auto globalTrans = GetScaledParameterValue(alg, kWQParamTransposeAll);
	// loop over each channel
	for (int ch = 0; ch < alg.NumChannels; ch++) {

		// if we have an assigned trigger for this channel, we only want to sample the quantized value when that is triggered
		// otherwise, we only want to sample the quantized value once every millisecond.  No need to sample every frame.
		bool sample = false;
		auto triggerBus = alg.v[kWQNumCommonParameters + (ch * kWQNumPerChannelParameters) + kWQParamTrigger] - 1;
		if (triggerBus >= 0) {
			// this means we are only sampling the trigger itself once per block, not once per frame.  hopefully this should be plenty
			// I could loop the frames here to see if a very small trigger occurred within the block if I have to, but performance.
			auto trig = alg.Triggers[ch].Process(busFrames[triggerBus * numFrames]);

			// delay the sample trigger slightly to allow the incoming pitch to be correct
			if (trig == Trigger::Edge::Rising) {
				auto delay = alg.v[kWQParamTriggerSampleDelay];
				alg.DelayedTriggers[ch] = alg.Timer.TotalMs + delay;
			}

			// if our delayed sample trigger has come due, sample
			if (alg.DelayedTriggers[ch] > 0 && alg.Timer.TotalMs >= alg.DelayedTriggers[ch]) {
				sample = true;
				alg.DelayedTriggers[ch] = 0;
			}
		} else {
			sample = (ms > 0);
		}

		if (sample) {
			// get the bus number for the input
			auto inBus  = alg.v[kWQNumCommonParameters + (ch * kWQNumPerChannelParameters) + kWQParamInput] - 1;

			// if we have a valid input bus, take it's value, otherwise use zero
			auto inputValue = (inBus >= 0) ? busFrames[inBus * numFrames] : 0.0f;

			// get other parameters
			auto atten  = GetScaledParameterValue(alg, kWQNumCommonParameters + (ch * kWQNumPerChannelParameters) + kWQParamAttenValue);
			auto offset = GetScaledParameterValue(alg, kWQNumCommonParameters + (ch * kWQNumPerChannelParameters) + kWQParamOffsetValue);
			auto trans  = GetScaledParameterValue(alg, kWQNumCommonParameters + (ch * kWQNumPerChannelParameters) + kWQParamTranspose);
			trans += globalTrans;

			// since we are only quantizing once a millisecond, we will just use the first value of the block
			alg.QuantRequest.Attenuate = atten;
			alg.QuantRequest.Offset = offset;
			alg.QuantRequest.Transpose = trans;
			alg.QuantRequest.UnquantizedValue = inputValue;
			alg.Quant.Quantize(alg.QuantRequest, alg.QuantResults[ch]);
		}

		// get the bus number for the output
		auto outBus = alg.v[kWQNumCommonParameters + (ch * kWQNumPerChannelParameters) + kWQParamOutput] - 1;

		// don't write anything if the output bus is "none"
		if (outBus >= 0) {
			//fill the whole output block with our result
			for (int i = 0; i < numFrames; i++) {
				busFrames[outBus * numFrames + i] = alg.QuantResults[ch].QuantizedValue;
			}
		}

	}
}


bool WeightedQuantizerAlg::Draw(_NT_algorithm* self) {
	auto& alg = *static_cast<WeightedQuantizerAlg*>(self);
	// do this in draw, because we don't need it as frequently as step
	alg.QuantView.ProcessLongPresses();
	alg.QuantView.Draw();


	// NT_drawShapeI(kNT_rectangle, 120, 0, 255, 60, 0);
	// char buf[15];
	// NT_intToString(buf, alg.updateCount);
	// NT_drawText(125, 10, buf);
	// NT_floatToString(buf, alg.firstValuePre, 3);
	// NT_drawText(125, 20, buf);
	// NT_floatToString(buf, alg.firstValuePost, 3);
	// NT_drawText(125, 30, buf);
	// NT_floatToString(buf, alg.firstCurrPotVal, 3);
	// NT_drawText(125, 40, buf);
	// NT_floatToString(buf, alg.firstPrevPotVal, 3);
	// NT_drawText(125, 50, buf);

	// NT_drawText(180, 10, alg.reason);
	// NT_intToString(buf, fixupCount);
	// NT_drawText(180, 20, buf);

	return true;
}


uint32_t WeightedQuantizerAlg::HasCustomUI(_NT_algorithm* self) {
	return kNT_potL | kNT_potR | kNT_encoderL | kNT_encoderR | kNT_encoderButtonR | kNT_potButtonR | kNT_button3;
}


void WeightedQuantizerAlg::SetupUI(_NT_algorithm* self, _NT_float3& pots) {
	auto& alg = *static_cast<WeightedQuantizerAlg*>(self);
	alg.QuantView.FixupPotValues(pots);
}


void WeightedQuantizerAlg::CustomUI(_NT_algorithm* self, const _NT_uiData& data) {
	auto& alg = *static_cast<WeightedQuantizerAlg*>(self);
	alg.QuantView.ProcessControlInput(data);
	alg.PotMgr.RecordPreviousPotValues(data);
}


void WeightedQuantizerAlg::Serialise(_NT_algorithm* self, _NT_jsonStream& stream) {
	auto& alg = *static_cast<WeightedQuantizerAlg*>(self);

	stream.addMemberName("BankNoteValues");
	stream.openArray();
	for (size_t b = 0; b < ARRAY_SIZE(alg.Banks); b++) {
		for (size_t n = 0; n < ARRAY_SIZE(alg.Banks[b].NoteValues); n++) {
			stream.addNumber(alg.Banks[b].NoteValues[n]);
		}
	}
	stream.closeArray();

	stream.addMemberName("ScanningLocked");
	stream.addBoolean(alg.Banks.ScanningLocked);

}


bool WeightedQuantizerAlg::Deserialise(_NT_algorithm* self, _NT_jsonParse& parse) {
	auto& alg = *static_cast<WeightedQuantizerAlg*>(self);

	int num;
	if (!parse.numberOfObjectMembers(num)) {
		return false;
	}

	for (int i = 0; i < num; i++) {
		if (parse.matchName("BankNoteValues")) {
			int numValues;
			if (!parse.numberOfArrayElements(numValues)) {
				return false;
			}

			// validate we have the expected number of values (10 banks of 12 note values)
			if (numValues != 120) {
				return false;
			}

			for (size_t b = 0; b < ARRAY_SIZE(alg.Banks); b++) {
				for (size_t n = 0; n < ARRAY_SIZE(alg.Banks[b].NoteValues); n++) {
					int val;
					if (!parse.number(val)) {
						return false;
					}
					alg.Banks[b].NoteValues[n] = val;
				}
			}
		} else if (parse.matchName("ScanningLocked")) {
			bool val;
			if (!parse.boolean(val)) {
				return false;
			}
			alg.Banks.ScanningLocked = val;
		} else {
			if (!parse.skipMember()) {
				return false;
			}
		}
	}

	return true;
}


int WeightedQuantizerAlg::ParameterUiPrefix(_NT_algorithm* self, int p, char* buff) {
	int len = 0;
	if (p >= kWQNumCommonParameters) {
		auto channel = (p - kWQNumCommonParameters) / kWQNumPerChannelParameters;
		channel++;
		len = NT_intToString( buff, channel );
		buff[len++] = ':';
		buff[len] = 0;
	}
	return len;
}


const _NT_factory WeightedQuantizerAlg::Factory =
{
	.guid = NT_MULTICHAR( 'A', 'T', 'w', 'q' ),
	.name = "Weighted Quantizer",
	.description = "A weighted note quantizer",
	.numSpecifications = ARRAY_SIZE(SpecificationsDef),
	.specifications = SpecificationsDef,
	.calculateRequirements = CalculateRequirements,
	.construct = Construct,
	.parameterChanged = ParameterChanged,
	.step = Step,
	.draw = Draw,
	.tags = kNT_tagUtility,
	.hasCustomUi = HasCustomUI,
	.customUi = CustomUI,
	.setupUi = SetupUI,
	.serialise = Serialise,
	.deserialise = Deserialise,
	.parameterUiPrefix = ParameterUiPrefix,
};
