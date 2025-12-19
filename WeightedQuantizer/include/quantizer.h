#pragma once


struct Quantizer {
private:

	static constexpr uint16_t NumberOfNotes = 12;

	struct NoteDef {
		const char* Name;
		float       Fractional;
	};

	static const NoteDef Notes[NumberOfNotes];
	
public:

	struct QuantRequest {
		// inputs
		float       UnquantizedValue;
		float       Attenuate;
		float       Offset;
		int16_t     Transpose;
	};


	struct QuantResult {
		float       QuantizedValue = 0.0f;
		const char* QuantizedNoteName = "C";
		const char* FinalNoteName = "C";
	};

	float NoteWeights[12];

	void Quantize(QuantRequest& req, QuantResult& result);

};
