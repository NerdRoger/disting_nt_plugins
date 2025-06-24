#pragma once
#include <type_traits>
#include <distingnt/api.h>


template <typename T_Algorithm>
struct OwnedBase {
protected:
	T_Algorithm* AlgorithmInstance;
public:
	virtual void Initialize(T_Algorithm& alg) {
		AlgorithmInstance = &alg;
	}
};