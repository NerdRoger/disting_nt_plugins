#pragma once
#include <stdlib.h>
#include <distingnt/api.h>


#ifndef HIDDEN
	#define HIDDEN __attribute__((visibility("hidden")))
#endif

#ifndef INLINE
	#define INLINE __attribute__((always_inline)) inline
#endif


constexpr uint8_t MAX_BUS_COUNT = 28;


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
INLINE constexpr T min(T a, T b) {
	return (a < b) ? a : b;
}


template <typename T>
INLINE constexpr T max(T a, T b) {
	return (a > b) ? a : b;
}


template <typename T>
INLINE constexpr T clamp(T val, T lo, T hi) {
	return (val < lo) ? lo : (val > hi) ? hi : val;
}


template <typename T>
INLINE constexpr T wrap(T val, T lo, T hi) {
	const T range = hi - lo + 1;
	val = (val - lo) % range;
	if (val < 0) val += range;
	return val + lo;
}


template <typename T>
INLINE constexpr void lohi(T& a, T& b) {
	if (b < a) {
		T tmp = a;
		a = b;
		b = tmp;
	}
}


INLINE constexpr uint16_t CalculateScaling(int scale) {
	switch (scale)
	{
		case kNT_scaling10:   return 10;
		case kNT_scaling100:  return 100;
		case kNT_scaling1000: return 1000;
		default: return 1;
	}
}


INLINE float GetScaledParameterValue(_NT_algorithm& alg, uint16_t paramIndex) {
	return static_cast<float>(alg.v[paramIndex]) / CalculateScaling(alg.parameters[paramIndex].scaling);
}


INLINE int16_t UnscaleValueForParameter(_NT_algorithm& alg, uint16_t paramIndex, float val) {
	return val * CalculateScaling(alg.parameters[paramIndex].scaling);
}



// #ifdef DEBUG
// 	__attribute__((always_inline)) 
// 	inline constexpr void LogMessage(const char* msg) {
// 		uint8_t midibuf[] = {
// 			0xF0, // sysex start
// 			0x7C, // non-commercial identifier:  https://midi.org/new-midi-association-sysex-id-policies-as-of-oct-15-2025
// 		};
				
// 		NT_sendMidiSysEx(kNT_destinationUSB, midibuf, sizeof(midibuf), false);
// 		NT_sendMidiSysEx(kNT_destinationUSB, reinterpret_cast<const uint8_t*>(msg), strlen(msg), true);
// 	}
// #else
// 	#define LogMessage(msg) ((void)0)
// #endif





// LLMs helped me write this implementation...  it takes multiple parameters and can print strings, ints, and floats
// LogMessage("Test message ", 123, " with float ", 45.67f);

#ifdef DEBUG
  #include <cstring>
	inline char* _append_to_buf(char* ptr, char* end, const char* s) {
		while (s && *s && ptr < end) *ptr++ = *s++;
		return ptr;
	}

	inline char* _append_to_buf(char* ptr, char* end, int value) {
		if (value == 0) { 
			if (ptr < end) *ptr++ = '0'; 
			return ptr; 
		}
		if (value < 0) { 
			if (ptr < end) *ptr++ = '-'; 
			value = -value; 
		}
		char tmp[12];
		int i = 0;
		while (value > 0 && i < 12) { 
			tmp[i++] = (value % 10) + '0'; 
			value /= 10; 
		}
		while (i > 0 && ptr < end) { 
			*ptr++ = tmp[--i]; 
		}
		return ptr;
	}

	inline char* _append_to_buf(char* ptr, char* end, float value) {
		if (ptr >= end) return ptr;

		if (value < 0) {
			*ptr++ = '-';
			value = -value;
		}

		// Integer part
		int ipart = static_cast<int>(value);
		ptr = _append_to_buf(ptr, end, ipart);

		if (ptr < end) *ptr++ = '.';

		// Fractional part (5 digits)
		float fpart = value - static_cast<float>(ipart);
		int f_int = static_cast<int>(fpart * 100000.0f + 0.5f);

		// Manual leading zeros for the fraction (e.g. .00123)
		int divisor = 10000;
		while (divisor > 1 && f_int < divisor && ptr < end) {
			*ptr++ = '0';
			divisor /= 10;
		}

		return _append_to_buf(ptr, end, f_int);
	}

	inline void _RawLogSend(const char* buf, size_t len) {
		uint8_t header[] = { 0xF0, 0x7C };
		// Send header (0xF0 0x7C)
		NT_sendMidiSysEx(kNT_destinationUSB, header, sizeof(header), false);
		// Send message body and terminate with 0xF7 (last param = true)
		NT_sendMidiSysEx(kNT_destinationUSB, reinterpret_cast<const uint8_t*>(buf), len, true);
	}

	template<typename... Args>
	void LogMessage(Args... args) {
		char buf[128];
		char* ptr = buf;
		char* end = buf + 127;

		// Fold expression: calls the correct _append_to_buf for each argument
		((ptr = _append_to_buf(ptr, end, args)), ...);
		
		*ptr = '\0';
		_RawLogSend(buf, ptr - buf);
	}
#else
    // Release mode: entirely erased by preprocessor
    #define LogMessage(...) ((void)0)
#endif





// TODO:  possibly track when the last transition was, to measure gate duration
struct Trigger {
private:
	static constexpr float LowThreshold = 0.1f;
	static constexpr float HighThreshold = 1.0f;

	bool State;
public:
	enum Edge {
		None,
		Rising,
		Falling
	};

	INLINE Edge Process(float val) {
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

	INLINE void Seed(uint32_t seed) {
		PrevRandom = seed;
	}

	INLINE uint32_t Next(uint32_t lowInclusive, uint32_t highInclusive) {
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
		uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
		// align the memory address to the next alignment boundary for the type
		addr = (addr + alignment - 1) & ~(alignment - 1);
		ptr = reinterpret_cast<uint8_t*>(addr);
		return reinterpret_cast<T*>(ptr);
	}

	// use this version with a factory function to construct each element
	template <typename F>
	static T* InitializeDynamicDataAndIncrementPointer(uint8_t*& ptr, size_t num, F factory) {
		// align the memory pointer to the boundary for the type
		T* result = AlignMemoryPointer(ptr);

		// let the supplied factory construct each element in place
		for (size_t i = 0; i < num; i++) {
			factory(result + i, i);
		}

		// advance the raw pointer by the total size of the allocated elements
		ptr = reinterpret_cast<uint8_t*>(result + num);

		return result;
	}

	// use this version if default construction is sufficient
	static T* InitializeDynamicDataAndIncrementPointer(uint8_t*& ptr, size_t num) {
		return InitializeDynamicDataAndIncrementPointer(ptr, num, [](T* addr, size_t) {
			new (addr) T();
		});
	}

};


void StringConcat(char* dest, int maxLen, ...);