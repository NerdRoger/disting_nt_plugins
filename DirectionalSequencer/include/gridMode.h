#pragma once

#include "common.h"
#include "gridInfo.h"
#include "modeBase.h"
#include "cellDefinition.h"
#include "cellData.h"

struct GridMode : ModeBase {
private:
	static constexpr int CellSize = 12;
	static constexpr int CellBorderColor = 5;
	static constexpr int CellBrightColor = 15;
	static constexpr int CellDimColor = 2;
	static constexpr int EditableCellBorderColor = 15;
	static constexpr int NonEditableCellBorderColor = 7;
	static constexpr int SelectedParameterColor = 15;
	static constexpr int UnselectedParameterColor = 5;

	Point GridPosition { ModeAreaX, 2 };
	CellDataType SelectedParameterIndex = CellDataType::Direction;
	float ParamEditRaw = 0;
	float SelectedParameterIndexRaw = 0;

	Bounds CellCoordsToBounds(const CellCoords& coords) const;

	void DrawCellPercentage(float val, bool selected, int x1, int y1, int x2, int y2) const;
	void DrawCellValue(float val, bool selected, int x1, int y1, int x2, int y2) const;
	void DrawCellVelocity(float val, bool selected, int x1, int y1, int x2, int y2) const;
	void DrawCellNumber(int16_t val, bool selected, int x1, int y1, int x2, int y2) const;
	void DrawCellBipolarValue(float val, bool selected, int x1, int y1, int x2, int y2) const;

	void DrawCell(const CellData& cell, bool selected, int x1, int y1, int x2, int y2) const;
	void DrawCells() const;
	void DrawInitialCellBorder() const;
	void DrawSelectedCellBorder() const;
	void DrawBullet(int x, int y, int color) const;
	void DrawParamLine(int paramIndex, int top) const;
	void DrawParamLineValue(int x, int y, int color, CellDataType ct, const CellDefinition& cd) const;
	void DrawParams() const;
	void DrawHelpSection() const;
	void DrawDirectionArrow(unsigned int dir, int x, int y, int color) const;
	float CalculateEpsilon(const CellDefinition& cd) const;

public:

	CellCoords SelectedCell;

	GridMode();
	void DrawIcon(int x, int y, int color) const override;
	void Draw() const override;
	void Encoder1Turn(int8_t x) override;
	void Encoder2Turn(int8_t x) override;
	void Encoder2ShortPress() override;
	void Encoder2LongPress() override;
	void Pot2Turn(float val) override;
	void Pot3Turn(float val) override;
	void Pot3ShortPress() override;
	void Pot3LongPress() override;
	void FixupPotValues(_NT_float3& pots) override;

	void LoadParamForEditing();
	void Activate() override;
};
