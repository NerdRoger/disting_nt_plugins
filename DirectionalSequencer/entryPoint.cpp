#include <distingnt/api.h>
#include "directionalSequencerAlgorithm.h"
#include "directionalSequencerModMatrixAlgorithm.h"


uintptr_t pluginEntry( _NT_selector selector, uint32_t data )
{
	switch ( selector )
	{
		case kNT_selector_version:
			return kNT_apiVersionCurrent;
		case kNT_selector_numFactories:
			return 2;
		case kNT_selector_factoryInfo:
			if (data == 0) {
				return (uintptr_t)&DirectionalSequencerAlgorithm::Factory;
			}
			if (data == 1) {
				return (uintptr_t)&DirectionalSequencerModMatrixAlgorithm::Factory;
			}
			return 0;
	}
	return 0;
}
