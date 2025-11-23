#pragma once
#include <stdlib.h>
#include <distingnt/api.h>


struct Point {
	uint8_t x;
	uint8_t y;
};


struct Bounds {
	uint8_t x1;
	uint8_t y1;
	uint8_t x2;
	uint8_t y2;
};


template <typename T>
__attribute__((always_inline)) 
inline constexpr T min(T a, T b) {
	return (a < b) ? a : b;
}


template <typename T>
__attribute__((always_inline)) 
inline constexpr T max(T a, T b) {
	return (a > b) ? a : b;
}


template <typename T>
__attribute__((always_inline)) 
inline constexpr T clamp(T val, T lo, T hi) {
	return (val < lo) ? lo : (val > hi) ? hi : val;
}


template <typename T>
__attribute__((always_inline)) 
inline constexpr T wrap(T val, T lo, T hi) {
	const T range = hi - lo + 1;
	val = (val - lo) % range;
	if (val < 0) val += range;
	return val + lo;
}


__attribute__((always_inline)) 
inline constexpr uint16_t CalculateScaling(int scale) {
	switch (scale)
	{
		case kNT_scaling10:   return 10;
		case kNT_scaling100:  return 100;
		case kNT_scaling1000: return 1000;
		default: return 1;
	}
}


// TODO:  possibly track when the last transition was, to measure gate duration
struct Trigger {
private:
	static constexpr float LowThreshold = 0.1f;
	static constexpr float HighThreshold = 1.0f;
	static constexpr float TriggerVoltage = 10.0f;

	bool State;
public:
	enum Edge {
		None,
		Rising,
		Falling
	};

	__attribute__((always_inline)) 
	inline Edge Process(float val) {
		if (State && (val < LowThreshold)) {
			State = false;
			return Falling;
		} else if (!State && (val >= HighThreshold)) {
			State = true;
			return Rising;
		}
		return None;
	}
};



struct RandomGenerator {
private:
	uint32_t PrevRandom;

public:

	__attribute__((always_inline)) 
	inline void Seed(uint32_t seed) {
		PrevRandom = seed;
	}

	__attribute__((always_inline)) 
	inline uint32_t Next(uint32_t lowInclusive, uint32_t highInclusive) {
		uint32_t x = PrevRandom;
		x ^= x << 13;
		x ^= x >> 17;
		x ^= x << 5;
		PrevRandom = x;

		uint32_t diff = highInclusive - lowInclusive;
		x %= (diff + 1);
		x += lowInclusive;
		return x;
	}
};


// helper used when dynamically allocating/aligning memory for various types from the same memory block
template <typename T>
struct MemoryHelper {
private:	
	static constexpr size_t size = sizeof(T);
	static constexpr size_t alignment = alignof(T);
	
public:
	static void AlignAndIncrementMemoryRequirement(uint32_t& memRequired, size_t numElements) {
		// align the memory requirement to the next alignment boundary for the type
		memRequired = (memRequired + alignment - 1) & ~(alignment - 1);
		// then increment it by the size of the type * number of elements
		memRequired += size * numElements;
	}

	static T* AlignMemoryPointer(uint8_t*& ptr) {
		// advance the pointer to the next alignment boundary for the type
		uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
		uintptr_t remainder = addr % alignment;
		if (remainder != 0) {
				addr += (alignment - remainder);
		}
		ptr = reinterpret_cast<uint8_t*>(addr);
		return reinterpret_cast<T*>(ptr);
	}

	static T* InitializeDynamicDataAndIncrementPointer(uint8_t*& ptr, size_t num) {
		// align the memory pointer to the boundary for the type
		T* result = AlignMemoryPointer(ptr);

		// construct each element in place using placement new
		// placement new is not strictly necessary for primitive types, but if the type has a constructor we want it to be called
		for (size_t i = 0; i < num; i++) {
			new (result + i) T();
		}

		// advance the raw pointer by the total size of the allocated elements
		ptr = reinterpret_cast<uint8_t*>(result + num);

		return result;
	}
};