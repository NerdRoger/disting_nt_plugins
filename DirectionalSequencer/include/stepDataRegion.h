#pragma once

#include "gridInfo.h"
#include "cellDefinition.h"
#include "dirSeqModMatrixAlg.h"

struct DirSeqAlg;


// this class represents all of the steps and their values
struct StepDataRegion {
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

	_NT_algorithm* Algorithm = nullptr;
	RandomGenerator* Random = nullptr;
	SingleCellData Cells[GridSizeX][GridSizeY];	

	DirSeqModMatrixAlg* GetModMatrixAlgorithm(CellDataType ct, int& paramTargetIndex) const;
	void DoDataChanged();

public:

	void (*OnDataChangedCallback)(_NT_algorithm* alg) = nullptr;


	StepDataRegion();
	void InjectDependencies(_NT_algorithm* alg, RandomGenerator* random, void (*onDataChanged)(_NT_algorithm*));

	bool  CellTypeHasMapping(CellDataType ct) const;
	float GetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct) const;
	float GetAdjustedCellValue(uint8_t x, uint8_t y, CellDataType ct) const;
	void  SetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct, float val, bool updateMatrix);
	void  SetDefaultCellValues();
	void  ScrambleAllCellValues(CellDataType ct);
	void  InvertCellValue(uint8_t x, uint8_t y, CellDataType ct);
	void  InvertAllCellValues(CellDataType ct);
	void  SwapWithSurroundingCellValue(uint8_t x, uint8_t y, CellDataType ct);
	void  RandomizeCellValue(uint8_t x, uint8_t y, CellDataType ct, float min, float max);
	void  RandomizeAllCellValues(CellDataType ct, float min, float max);
	void  RotateCellValuesInRow(uint8_t row, CellDataType ct, int8_t rotateBy);
	void  RotateCellValuesInColumn(uint8_t col, CellDataType ct, int8_t rotateBy);
	void  RandomlyChangeCellValue(uint8_t x, uint8_t y, CellDataType ct, uint8_t deltaPercent);
	void  RandomlyChangeAllCellValues(CellDataType ct, uint8_t deltaPercent);
};
