#pragma once
#include <stdint.h>
#include <distingnt/api.h>


enum class CellDataType {
	Direction,
	Value,
	Velocity,
	Probability,
	Ratchets,
	RestAfter,
	GateLength,
	DriftProb,
	MaxDrift,
	Repeats,
	Glide,
	AccumAdd,
	AccumTimes,

	NumCellDataTypes
};


struct CellDefinition {
	const char* FieldName;    // Used to identify this field in the preset JSON.  Should never change
	const char* DisplayName;
	float Min;
	float Max;
	float Default;
	uint8_t Unit;
	uint8_t Scaling;
	uint16_t ScalingFactor;
	float Epsilon;
	int HelpTextX;
	const char* HelpText;

	__attribute__((always_inline))
	float NTValueToCellValue(int16_t ntValue) {
		float result = ntValue;
		result /= ScalingFactor;
		return result;
	}

	static const CellDefinition All[static_cast<uint16_t>(CellDataType::NumCellDataTypes)];

};
