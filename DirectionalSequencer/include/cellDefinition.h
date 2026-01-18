#pragma once
#include <stdint.h>
#include <distingnt/api.h>


// orders must match the CellDefinition::All array
enum class CellDataType {
	Direction,
	Value,
	Velocity,
	Glide,
	GateLength,
	Ratchets,
	TieSteps,
	Probability,
	RestAfter,
	Repeats,
	DriftProb,
	MaxDrift,
	AccumAdd,
	AccumTimes,

	NumCellDataTypes
};


struct CellDefinition {
	const char* FieldName;    // Used to identify this field in the preset JSON.  Should never change
	const char* DisplayName;
	int16_t Min;
	int16_t Max;
	int16_t Default;
	uint8_t Unit;
	uint8_t Scaling;
	int HelpTextX;
	const char* HelpText;

	INLINE float ScalingFactor() const {
		switch (Scaling) {
			case kNT_scalingNone: return 1.0f;
			case kNT_scaling10:   return 10.0f;
			case kNT_scaling100:  return 100.0f;
			case kNT_scaling1000: return 1000.0f;
			default: return 0.0f;  // might cause divide by zeroes, but that's ok we want to highlight that we have a bad value somehow
		}
	}

	// many of these inline functions do the same thing, but in different contexts
	// so they are duplicated with different names for clarity.  But since they
	// are inline, it shouldn't matter as all the code will just be inlined

	// converts a scaled int down to the unscaled float
	INLINE float UnscaleValue(int16_t value) const {
		return static_cast<float>(value) / ScalingFactor();
	}
	
	// converts unscaled float up to a scaled int
	INLINE int16_t ScaleValue(float value) const {
		return value * ScalingFactor();
	}

	// converts data stored in internal cell data to the actual cell value
	INLINE float CellStorageToCellValue(int16_t storedValue) const {
		return static_cast<float>(storedValue) / ScalingFactor();
	}
	
	// converts an actual cell value into internal cell data storage
	INLINE int16_t CellValueToCellStorage(float cellValue) const {
		return cellValue * ScalingFactor();
	}

	// convert an NT parameter value to the cell value
	INLINE float NTValueToCellValue(int16_t ntValue) const {
		return static_cast<float>(ntValue) / ScalingFactor();
	}

	// convert the cell value to an NT parameter value
	INLINE int16_t CellValueToNTValue(float cellValue) const {
		return cellValue * ScalingFactor();
	}

	INLINE float ScaledMin() const {
		return static_cast<float>(Min) / ScalingFactor();
	}

	INLINE float ScaledMax() const {
		return static_cast<float>(Max) / ScalingFactor();
	}

	INLINE float ScaledDefault() const {
		return static_cast<float>(Default) / ScalingFactor();
	}

	INLINE float Epsilon() const {
		return 0.5f / ScalingFactor();
	}

	HIDDEN static const CellDefinition All[static_cast<uint16_t>(CellDataType::NumCellDataTypes)];

};
