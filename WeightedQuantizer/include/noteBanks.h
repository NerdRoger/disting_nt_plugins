#pragma once
#include <stdint.h>
#include <stdio.h>
#include <distingnt/api.h>


struct WeightedQuantizerAlg;


struct NoteBanks {
public:
	struct Bank {
		int16_t NoteValues[12];
	};

private:
	WeightedQuantizerAlg* Algorithm = nullptr;

	Bank Banks[10];

public:

	bool ScanningLocked;

	NoteBanks();
	void InjectDependencies(WeightedQuantizerAlg* alg);

	Bank& operator[](size_t index);
	const Bank& operator[](size_t index) const;
	void DoBankScan(int16_t val);
	void LoadNotesFromBank(size_t bankNum);
	void SaveNotesToBank(size_t bankNum);


	const size_t Count = ARRAY_SIZE(Banks);
};