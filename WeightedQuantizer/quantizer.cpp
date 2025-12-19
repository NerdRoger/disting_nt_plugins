#include <math.h>
#include <float.h>
#include "common.h"
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


void Quantizer::Quantize(Quantizer::QuantRequest& req, Quantizer::QuantResult& result) {
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

	result.QuantizedValue = bestCandidate + transposeOffset;
	if (bestNoteIndex == -1) {
		result.QuantizedNoteName = "??";
		result.FinalNoteName = "??";
	} else {
		result.QuantizedNoteName = Notes[bestNoteIndex].Name;
		// transposition might take us out of range of our notes array, so wrap it around the boundaries
		// simple modulo won't work for negative numbers, so use this helper
		auto transposedNoteIndex = wrap(bestNoteIndex + req.Transpose, 0, 11);
		result.FinalNoteName = Notes[transposedNoteIndex].Name;
	}
}
