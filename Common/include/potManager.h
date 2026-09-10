#pragma once
#include <distingnt/api.h>


struct PotManager {
	private:
		bool HasPreviousPotValues = false;
		_NT_float3 PreviousPotValues;
	
	public:
		_NT_DRAM_SECTION void RecordPreviousPotValues(const _NT_uiData& data);
		_NT_DRAM_SECTION void UpdateValueWithPot(int potIndex, float currentPotVal, float& value, float min, float max);
	};

	
