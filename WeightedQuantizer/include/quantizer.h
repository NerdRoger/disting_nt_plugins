#pragma once
#include "ownedBase.h"


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
		// outputs
		const char* QuantizedNoteName;
		const char* FinalNoteName;
    float       OutputValue;
	};

	float NoteWeights[12];

	void Quantize(QuantRequest& req);

};
