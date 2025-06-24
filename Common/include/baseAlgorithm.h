#pragma once
#include <distingnt/api.h>
#include "common.h"


struct BaseAlgorithm : public _NT_algorithm {
	private:
		bool HasPreviousPotValues = false;
		_NT_float3 PreviousPotValues;
		uint32_t InternalFrameCount = 0;
	
	protected:
		uint32_t CountMilliseconds(int numFrames);
		static void RecordPreviousPotValues(_NT_algorithm* self, const _NT_uiData& data);
		
	public:
		uint32_t static const SamplesPerMs;
		uint32_t TotalMs = 0;
	
		void UpdateValueWithPot(int potIndex, float currentPotVal, float& value, float min, float max);

		__attribute__((always_inline))
		inline float GetScaledParameterValue(uint16_t paramIndex) {
			return static_cast<float>(v[paramIndex]) / CalculateScaling(parameters[paramIndex].scaling);
		}
	
		// TODO: any other stuff I can pull up to this base class

	};
	
	