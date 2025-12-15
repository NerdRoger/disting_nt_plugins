#pragma once

#include "ownedBase.h"
#include "gridInfo.h"
#include "cellDefinition.h"


struct DirectionalSequencer;


// this class represents all of the steps and their values
struct StepDataRegion : OwnedBase<DirectionalSequencer> {
private:

	struct SingleCellData {
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
		friend class StepDataRegion;
	};


	SingleCellData Cells[GridSizeX][GridSizeY];	

public:
	float GetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct, bool readFromParam = true) const;
	float GetAdjustedCellValue(uint8_t x, uint8_t y, CellDataType ct) const;
	void  SetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct, float val, bool updateParam = true);
	void  SetDefaultCellValues();
};
