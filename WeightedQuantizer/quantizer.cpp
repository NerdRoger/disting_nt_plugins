#include <math.h>
#include <float.h>
#include "quantizer.h"


const Quantizer::NoteDef Quantizer::Notes[NumberOfNotes] = {
	{ .Name = "C",  .Fractional = 0.0f  / 12.0f },
	{ .Name = "C#", .Fractional = 1.0f  / 12.0f },
	{ .Name = "D",  .Fractional = 2.0f  / 12.0f },
	{ .Name = "D#", .Fractional = 3.0f  / 12.0f },
	{ .Name = "E",  .Fractional = 4.0f  / 12.0f },
	{ .Name = "F",  .Fractional = 5.0f  / 12.0f },
	{ .Name = "F#", .Fractional = 6.0f  / 12.0f },
	{ .Name = "G",  .Fractional = 7.0f  / 12.0f },
	{ .Name = "G#", .Fractional = 8.0f  / 12.0f },
	{ .Name = "A",  .Fractional = 9.0f  / 12.0f },
	{ .Name = "A#", .Fractional = 10.0f / 12.0f },
	{ .Name = "B",  .Fractional = 11.0f / 12.0f },
};


void Quantizer::Quantize(Quantizer::QuantRequest& req) {
	float transposeOffset = static_cast<float>(req.Transpose) / 12.0f;

	float val = req.UnquantizedValue * req.Attenuate / 100.0f + req.Offset;
	float bestScore = -FLT_MAX;
	float bestCandidate = val;
	int bestNoteIndex = -1;
	for (size_t i = 0; i < ARRAY_SIZE(Notes); i++) {
		auto& note = Notes[i];
		auto weight = NoteWeights[i];
		if (weight > 0) {
			int intPart = static_cast<int>(val);
			for (int offset = -1; offset <= 1; offset++) {
				float candidate = (intPart + offset) + note.Fractional;
				float distance = fabsf(val - candidate);
				float score = distance == 0.0f ? FLT_MAX : weight / distance;
				if (score > bestScore) {
					bestScore = score;
					bestCandidate = candidate;
					bestNoteIndex = i;
				}
			}
		}
	}

	req.OutputValue = bestCandidate + transposeOffset;
	if (bestNoteIndex == -1) {
		req.QuantizedNoteName = "??";
		req.FinalNoteName = "??";
	} else {
		req.QuantizedNoteName = Notes[bestNoteIndex].Name;
		auto transposedNoteIndex = (bestNoteIndex + req.Transpose) % NumberOfNotes;
		req.FinalNoteName = Notes[transposedNoteIndex].Name;

	}
}
