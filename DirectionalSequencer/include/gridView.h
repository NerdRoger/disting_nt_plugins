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

	friend struct DirSeqAlg;

	DirSeqAlg* Algorithm;

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
	void DrawCellAccumAdd(float val, bool selected, int x1, int y1, int x2, int y2) const;

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

	HIDDEN static void OnActivateHandler(ViewBase* view);
	HIDDEN static void OnDrawHandler(ViewBase* view);
	HIDDEN static void OnEncoder1TurnHandler(ViewBase* view, int8_t x);
	HIDDEN static void OnEncoder2TurnHandler(ViewBase* view, int8_t x);
	HIDDEN static void OnEncoder2ShortPressHandler(ViewBase* view);
	HIDDEN static void OnEncoder2LongPressHandler(ViewBase* view);
	HIDDEN static void OnPot1TurnHandler(ViewBase* view, float val);
	HIDDEN static void OnPot2TurnHandler(ViewBase* view, float val);
	HIDDEN static void OnPot3TurnHandler(ViewBase* view, float val);
	HIDDEN static void OnPot3ShortPressHandler(ViewBase* view);
	HIDDEN static void OnPot3LongPressHandler(ViewBase* view);
	HIDDEN static void OnFixupPotValuesHandler(ViewBase* view, _NT_float3& pots);

public:

	CellCoords SelectedCell;

	GridView();
	void InjectDependencies(DirSeqAlg* alg);

	void LoadParamForEditing();
};
