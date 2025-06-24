#pragma once
#include <cstddef>
#include "cellDefinition.h"

struct DirectionalSequencer;

struct CellData {
private:
	// IMPORTANT:  maintain field order for optimal bitfield packing into 32-bit boundaries
	unsigned int Velocity         : 7;   // 0-127
	unsigned int Probability      : 7;   // 0-100
	unsigned int GateLength       : 7;   // 0-100
	unsigned int DriftProbability : 7;   // 0-100
	unsigned int Direction        : 4;   // 0-8
	// 32-BIT BOUNDARY
	unsigned int StepValue        : 14;  // 0.000 - 10.000 (fixed point)
	unsigned int GlidePercent     : 7;   // 0-100
	signed   int AccumulatorAdd   : 11;  // -1.000 - 1.000 (fixed point)
	// 32-BIT BOUNDARY
	unsigned int RatchetCount     : 3;   // 0-7
	unsigned int RestAfter        : 3;   // 0-7
	unsigned int RepeatCount      : 3;   // 0-7
	unsigned int AccumulatorTimes : 3;   // 0-7
	unsigned int MaxDriftAmount   : 14;  // 0.000 - 10.000 (fixed point)
	// 6 bits left, could we think of another data point to pack into this space?
	// or maybe just make some of the others bigger???

public:
	// methods here take a sequencer reference, because we don't want to carry that per cell
	float GetField(const DirectionalSequencer& alg, CellDataType ct) const;
	void SetField(const DirectionalSequencer& alg, CellDataType ct, float val);

};