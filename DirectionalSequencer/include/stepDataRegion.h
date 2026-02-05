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
		unsigned int MaxDriftAmount   : 14;  // 0.000 - 10.000 (fixed point)
		unsigned int TieStepCount     : 3;   // 0-7
		bool         Mute             : 1;   // true or false
		// 32-BIT BOUNDARY
		unsigned int RatchetCount     : 3;   // 0-7
		unsigned int RestAfter        : 3;   // 0-7
		unsigned int RepeatCount      : 3;   // 0-7
		unsigned int GlidePercent     : 7;   // 0-100
		signed   int AccumulatorAdd   : 13;  // -4.000 - 4.000 (fixed point)
		unsigned int AccumulatorTimes : 3;   // 0-7
		// 32-BIT BOUNDARY

	public:
		friend class StepDataRegion;
	};

	DirSeqAlg* Algorithm = nullptr;
	SingleCellData Cells[GridSizeX][GridSizeY];	

	DirSeqModMatrixAlg* GetModMatrixAlgorithm(CellDataType ct, int& paramTargetIndex) const;
	void DoDataChanged();

public:

	StepDataRegion();
	void InjectDependencies(DirSeqAlg* alg);

	bool  CellTypeHasMapping(CellDataType ct) const;
	float GetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct) const;
	float GetAdjustedCellValue(uint8_t x, uint8_t y, CellDataType ct) const;
	void  SetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct, float val, bool updateMatrix, CallingContext ctx);
	void  SetDefaultCellValues(CallingContext ctx);
	void  ScrambleAllCellValues(CellDataType ct, CallingContext ctx);
	void  InvertCellValue(uint8_t x, uint8_t y, CellDataType ct, CallingContext ctx);
	void  InvertAllCellValues(CellDataType ct, CallingContext ctx);
	void  SwapWithSurroundingCellValue(uint8_t x, uint8_t y, CellDataType ct, CallingContext ctx);
	void  RandomizeCellValue(uint8_t x, uint8_t y, CellDataType ct, float min, float max, CallingContext ctx);
	void  RandomizeAllCellValues(CellDataType ct, float min, float max, CallingContext ctx);
	void  RotateCellValuesInRow(uint8_t row, CellDataType ct, int8_t rotateBy, CallingContext ctx);
	void  RotateCellValuesInColumn(uint8_t col, CellDataType ct, int8_t rotateBy, CallingContext ctx);
	void  RandomlyChangeCellValue(uint8_t x, uint8_t y, CellDataType ct, uint8_t deltaPercent, CallingContext ctx);
	void  RandomlyChangeAllCellValues(CellDataType ct, uint8_t deltaPercent, CallingContext ctx);
};
