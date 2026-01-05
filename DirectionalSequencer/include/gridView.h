#pragma once

#include "common.h"
#include "gridInfo.h"
#include "viewBase.h"
#include "cellDefinition.h"
#include "stepDataRegion.h"
#include "playhead.h"
#include "helpTextHelper.h"
#include "potManager.h"


struct DirSeqAlg;


struct GridView : ViewBase {
private:

	friend DirSeqAlg;

	PlayheadList* Playheads = nullptr;
	StepDataRegion* StepData = nullptr;
	HelpTextHelper* HelpText = nullptr;
	PotManager* PotMgr = nullptr;

	char PlayheadHelpText[20];

	Point GridPosition { 50, 2 };
	CellDataType SelectedParameterIndex = CellDataType::Direction;
	float ParamEditRaw = 0;
	float SelectedParameterIndexRaw = 0;
	bool Editable = true;
	float SelectedPlayheadIndexRaw = 0;
	uint8_t SelectedPlayheadIndex = 0;

	Bounds CellCoordsToBounds(const CellCoords& coords) const;

	void DrawCellPercentage(float val, bool selected, int x1, int y1, int x2, int y2) const;
	void DrawCellValue(float val, bool selected, int x1, int y1, int x2, int y2) const;
	void DrawCellVelocity(float val, bool selected, int x1, int y1, int x2, int y2) const;
	void DrawCellNumber(int16_t val, bool selected, int x1, int y1, int x2, int y2, int16_t specialCase, const char *specialCaseTxt) const;
	void DrawCellBipolarValue(float val, bool selected, int x1, int y1, int x2, int y2) const;

	void DrawCell(uint8_t cx, uint8_t cy, bool selected, int x1, int y1, int x2, int y2) const;
	void DrawCells() const;
	void DrawInitialCellBorder() const;
	void DrawSelectedCellBorder() const;
	void DrawBullet(int x, int y, int color) const;
	void DrawParamLine(int paramIndex, int top) const;
	void DrawParamLineValue(int x, int y, int color, CellDataType ct, const CellDefinition& cd, float fval) const;
	void DrawParams() const;
	void DrawPlayheadIcon(int x, int y, int width, int color) const;
	void DrawPlayheadLine(int playheadIndex, int top) const;
	void DrawPlayheadList() const;
	void DrawHelpSection() const;
	void DrawDirectionArrow(unsigned int dir, int x, int y, int color) const;
	float CalculateEpsilon(const CellDefinition& cd) const;

public:

	CellCoords SelectedCell;

	GridView();
	void InjectDependencies(TimeKeeper* timer, StepDataRegion* stepData, HelpTextHelper* helpText, PotManager* potMgr, PlayheadList* playheads);
	void Draw() const override;
	void Encoder1Turn(int8_t x) override;
	void Encoder2Turn(int8_t x) override;
	void Encoder2ShortPress() override;
	void Encoder2LongPress() override;
	void Pot1Turn(float val) override;
	void Pot2Turn(float val) override;
	void Pot3Turn(float val) override;
	void Pot3ShortPress() override;
	void Pot3LongPress() override;
	void FixupPotValues(_NT_float3& pots) override;

	void LoadParamForEditing();
	void Activate() override;
};
