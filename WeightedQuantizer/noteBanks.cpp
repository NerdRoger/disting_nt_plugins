#include <distingnt/api.h>
#include "common.h"
#include "noteBanks.h"
#include "weightedQuantizerAlg.h"


NoteBanks::NoteBanks() {
	ScanningLocked = true;

	// lets give the banks some defaults

	// bank 0 - Ionian
	Banks[0].NoteValues[0] = 1000;
	Banks[0].NoteValues[2] = 1000;
	Banks[0].NoteValues[4] = 1000;
	Banks[0].NoteValues[5] = 1000;
	Banks[0].NoteValues[7] = 1000;
	Banks[0].NoteValues[9] = 1000;
	Banks[0].NoteValues[11] = 1000;

	// bank 1 - Dorian
	Banks[1].NoteValues[0] = 1000;
	Banks[1].NoteValues[2] = 1000;
	Banks[1].NoteValues[3] = 1000;
	Banks[1].NoteValues[5] = 1000;
	Banks[1].NoteValues[7] = 1000;
	Banks[1].NoteValues[9] = 1000;
	Banks[1].NoteValues[10] = 1000;

	// bank 2 - Phrygian
	Banks[2].NoteValues[0] = 1000;
	Banks[2].NoteValues[1] = 1000;
	Banks[2].NoteValues[3] = 1000;
	Banks[2].NoteValues[5] = 1000;
	Banks[2].NoteValues[7] = 1000;
	Banks[2].NoteValues[8] = 1000;
	Banks[2].NoteValues[10] = 1000;

	// bank 3 - Lydian
	Banks[3].NoteValues[0] = 1000;
	Banks[3].NoteValues[2] = 1000;
	Banks[3].NoteValues[4] = 1000;
	Banks[3].NoteValues[6] = 1000;
	Banks[3].NoteValues[7] = 1000;
	Banks[3].NoteValues[9] = 1000;
	Banks[3].NoteValues[11] = 1000;

	// bank 4 - Mixolydian
	Banks[4].NoteValues[0] = 1000;
	Banks[4].NoteValues[2] = 1000;
	Banks[4].NoteValues[4] = 1000;
	Banks[4].NoteValues[5] = 1000;
	Banks[4].NoteValues[7] = 1000;
	Banks[4].NoteValues[9] = 1000;
	Banks[4].NoteValues[10] = 1000;

	// bank 5 - Aeolian
	Banks[5].NoteValues[0] = 1000;
	Banks[5].NoteValues[2] = 1000;
	Banks[5].NoteValues[3] = 1000;
	Banks[5].NoteValues[5] = 1000;
	Banks[5].NoteValues[7] = 1000;
	Banks[5].NoteValues[8] = 1000;
	Banks[5].NoteValues[10] = 1000;

	// bank 6 - Locrian
	Banks[6].NoteValues[0] = 1000;
	Banks[6].NoteValues[1] = 1000;
	Banks[6].NoteValues[3] = 1000;
	Banks[6].NoteValues[5] = 1000;
	Banks[6].NoteValues[6] = 1000;
	Banks[6].NoteValues[8] = 1000;
	Banks[6].NoteValues[10] = 1000;

	// bank 7 - Major Pentatonic
	Banks[7].NoteValues[0] = 1000;
	Banks[7].NoteValues[2] = 1000;
	Banks[7].NoteValues[4] = 1000;
	Banks[7].NoteValues[7] = 1000;
	Banks[7].NoteValues[9] = 1000;

	// bank 8 - Minor Pentatonic
	Banks[8].NoteValues[0] = 1000;
	Banks[8].NoteValues[3] = 1000;
	Banks[8].NoteValues[5] = 1000;
	Banks[8].NoteValues[7] = 1000;
	Banks[8].NoteValues[10] = 1000;

	// bank 9 - Chromatic
	Banks[9].NoteValues[0] = 1000;
	Banks[9].NoteValues[1] = 1000;
	Banks[9].NoteValues[2] = 1000;
	Banks[9].NoteValues[3] = 1000;
	Banks[9].NoteValues[4] = 1000;
	Banks[9].NoteValues[5] = 1000;
	Banks[9].NoteValues[6] = 1000;
	Banks[9].NoteValues[7] = 1000;
	Banks[9].NoteValues[8] = 1000;
	Banks[9].NoteValues[9] = 1000;
	Banks[9].NoteValues[10] = 1000;
	Banks[9].NoteValues[11] = 1000;
}


void NoteBanks::InjectDependencies(_NT_algorithm* alg) {
	Algorithm = alg;
}


NoteBanks::Bank& NoteBanks::operator[](size_t index) {
	return Banks[index];
}


const NoteBanks::Bank& NoteBanks::operator[](size_t index) const {
	return Banks[index];
}


void NoteBanks::DoBankScan(int16_t val) {
	if (ScanningLocked) {
		return;
	}

	auto algIndex = NT_algorithmIndex(Algorithm);
	auto& param = Algorithm->parameters[kWQParamBankScanPosition];
	auto scaling = CalculateScaling(param.scaling);
	int b1 = (val / scaling) - 1;
	int b2 = (val == b1 * scaling) ? b1 : (val / scaling);
	for (int n = 0; n < 12; n++) {
		auto a = Banks[b1].NoteValues[n];
		auto b = Banks[b2].NoteValues[n];
		// weighted average
		auto bWeight = val % scaling;
		auto aWeight = scaling - bWeight;
		auto c = (a * aWeight + b * bWeight) / scaling;
		NT_setParameterFromAudio(algIndex, kWQParamQuantWeightC + n + NT_parameterOffset(), c);
	}
}


void NoteBanks::LoadNotesFromBank(size_t bankNum) {
	auto& bank = Banks[bankNum];
	auto algIndex = NT_algorithmIndex(Algorithm);
	for (size_t i = 0; i < ARRAY_SIZE(bank.NoteValues); i++) {
		NT_setParameterFromAudio(algIndex, kWQParamQuantWeightC + i + NT_parameterOffset(), bank.NoteValues[i]);
	}
}


void NoteBanks::SaveNotesToBank(size_t bankNum) {
	auto& bank = Banks[bankNum];
	for (size_t i = 0; i < ARRAY_SIZE(bank.NoteValues); i++) {
		bank.NoteValues[i] = Algorithm->v[kWQParamQuantWeightC + i];
	}
}
