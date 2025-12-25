#pragma once
#include <stdint.h>
#include <math.h>
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
	// TODO:  eliminate precision, it's the same as Scaling
	int Precision;
	uint8_t Unit;
	uint8_t Scaling;
	int HelpTextX;
	const char* HelpText;

	__attribute__((always_inline))
	float NTValueToCellValue(int16_t ntValue) {
		float result = ntValue;
		float divisor = pow(10.0f, Scaling);
		result /= divisor;
		return result;
	}

	static const CellDefinition All[static_cast<uint16_t>(CellDataType::NumCellDataTypes)];

};
