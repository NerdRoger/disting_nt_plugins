#pragma once
#include <stdint.h>


struct TimeKeeper {
	private:
		uint32_t InternalFrameCount;
		uint32_t SamplesPerMs;
	
	public:
		uint32_t TotalMs;

		TimeKeeper(uint32_t sampleRate);
		uint32_t CountMilliseconds(int numFrames);
	};
	
	