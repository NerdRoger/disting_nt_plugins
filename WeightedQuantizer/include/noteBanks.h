#pragma once
#include "common.h"


struct WeightedQuantizerAlg;


struct NoteBanks {
public:
	struct Dependencies {
		WeightedQuantizerAlg* Algorithm = nullptr;
	};

	struct Bank {
		int16_t NoteValues[12];
	};

	static constexpr size_t Count = 10;

private:
	WeightedQuantizerAlg* Algorithm = nullptr;

	Bank Banks[Count];

public:

	bool ScanningLocked;

	NoteBanks();
	void InjectDependencies(const Dependencies& dependencies);

	Bank& operator[](size_t index);
	const Bank& operator[](size_t index) const;
	void DoBankScan(int16_t val, CallingContext ctx);
	void LoadNotesFromBank(size_t bankNum);
	void SaveNotesToBank(size_t bankNum);
};