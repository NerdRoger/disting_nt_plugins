#include <distingnt/api.h>
#include "timeKeeper.h"


_NT_DRAM_SECTION
TimeKeeper::TimeKeeper() {
	InternalFrameCount = 0;
	TotalMs = 0;
}


_NT_DRAM_SECTION
void TimeKeeper::InjectDependencies(const Dependencies& dependencies) {
	SamplesPerMs = dependencies.Globals->sampleRate / 1000;
}


uint32_t TimeKeeper::CountMilliseconds(int numFrames) {
	InternalFrameCount += numFrames;

	uint32_t deltaMs = InternalFrameCount / SamplesPerMs;
	TotalMs += deltaMs;

	// subtract off the number of samples we just added to our running ms counter, to keep InternalFrameCount low
	InternalFrameCount -= (deltaMs * SamplesPerMs);

	return deltaMs;
}
