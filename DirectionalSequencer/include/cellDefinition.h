#pragma once
#include <stdint.h>
#include <distingnt/api.h>


struct CellDefinition {
	const char* FieldName;    // Used to identify this field in the preset JSON.  Should never change
	const char* DisplayName;
	float Min;
	float Max;
	float Default;
	int Precision;
	const char* HelpText;
};


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
	AccumTimes
};


extern const CellDefinition CellDefinitions[13];
