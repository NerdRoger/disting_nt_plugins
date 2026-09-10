#pragma once
#include <stdint.h>
#include <distingnt/api.h>


struct TimeKeeper {
	public:
		struct Dependencies {
			const _NT_globals* Globals = nullptr;
		};

	private:
		uint32_t InternalFrameCount;
		uint32_t SamplesPerMs;
	
	public:
		uint32_t TotalMs;

		_NT_DRAM_SECTION TimeKeeper();
		_NT_DRAM_SECTION void InjectDependencies(const Dependencies& dependencies);
		uint32_t CountMilliseconds(int numFrames);
	};

	
