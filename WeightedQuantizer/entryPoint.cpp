#include <distingnt/api.h>
#include "weightedQuantizerAlg.h"


// anonymous namespace for this data keeps the compiler from generating GOT entries, keeps us using internal linkage
namespace {
	auto WeightedQuantizerFactory = &WeightedQuantizerAlg::Factory;
}


uintptr_t pluginEntry( _NT_selector selector, uint32_t data )
{
	switch ( selector )
	{
		case kNT_selector_version:
			return kNT_apiVersionCurrent;
		case kNT_selector_numFactories:
			return 1;
		case kNT_selector_factoryInfo:
			if (data == 0) {
				return (uintptr_t)WeightedQuantizerFactory;
			}
			return 0;
	}
	return 0;
}
