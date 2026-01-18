#include <stdio.h>
#include <string.h>
#include <distingnt/api.h>
#include "common.h"
#include "gridView.h"
#include "helpTextHelper.h"
#include "dirSeqAlg.h"


static constexpr int CellSize = 12;
static constexpr int CellBorderColor = 5;
static constexpr int CellBrightColor = 15;
static constexpr int CellDimColor = 2;
static constexpr int EditableCellBorderColor = 15;
static constexpr int NonEditableCellBorderColor = 7;
static constexpr int SelectedParameterColor = 15;
static constexpr int UnselectedParameterColor = 5;
static constexpr int TextLineHeight = 10;


void GridView::OnActivateHandler(ViewBase* view) {
	auto& grid = *static_cast<GridView*>(view);
	grid.LoadParamForEditing();
}


void GridView::OnDrawHandler(ViewBase* view) {
	auto& grid = *static_cast<GridView*>(view);
	grid.DrawCells();
	grid.DrawInitialCellBorder();
	grid.DrawSelectedCellBorder();
	grid.DrawPlayheadList();
	grid.DrawParams();
	grid.DrawHelpSection();
}


void GridView::OnEncoder1TurnHandler(ViewBase* view, int8_t x) {
	auto& grid = *static_cast<GridView*>(view);
	grid.SelectedCell.x = wrap(grid.SelectedCell.x + x, 0, GridSizeX - 1);
	grid.LoadParamForEditing();
}


void GridView::OnEncoder2TurnHandler(ViewBase* view, int8_t x) {
	auto& grid = *static_cast<GridView*>(view);
	grid.SelectedCell.y = wrap(grid.SelectedCell.y + x, 0, GridSizeY - 1);
	grid.LoadParamForEditing();
}


void GridView::OnEncoder2ShortPressHandler(ViewBase* view) {
	auto& grid = *static_cast<GridView*>(view);
	grid.Editable = !grid.Editable;
}


void GridView::OnEncoder2LongPressHandler(ViewBase* view) {
	auto& grid = *static_cast<GridView*>(view);
	if (grid.Editable) {
		grid.Algorithm->Playheads[grid.SelectedPlayheadIndex].InitialStep = grid.SelectedCell;
	}
}


void GridView::OnPot1TurnHandler(ViewBase* view, float val) {
	auto& grid = *static_cast<GridView*>(view);
	auto old = grid.SelectedPlayheadIndex;
	grid.Algorithm->PotMgr.UpdateValueWithPot(0, val, grid.SelectedPlayheadIndexRaw, 0, grid.Algorithm->Playheads.Count);
	grid.SelectedPlayheadIndexRaw = clamp(grid.SelectedPlayheadIndexRaw, 0.0f, static_cast<float>(grid.Algorithm->Playheads.Count) - 0.001f);
	grid.SelectedPlayheadIndex = grid.SelectedPlayheadIndexRaw;
	if (grid.SelectedPlayheadIndex != old) {
		StringConcat(grid.PlayheadHelpText, 20, "Playhead X Selected");
		grid.PlayheadHelpText[9] = 'A' + grid.SelectedPlayheadIndex;
		grid.Algorithm->HelpText.DisplayHelpText(70, grid.PlayheadHelpText);
	}
}


void GridView::OnPot2TurnHandler(ViewBase* view, float val) {
	auto& grid = *static_cast<GridView*>(view);
	auto old = grid.SelectedParameterIndex;
	grid.Algorithm->PotMgr.UpdateValueWithPot(1, val, grid.SelectedParameterIndexRaw, 0, static_cast<float>(CellDataType::NumCellDataTypes));
	grid.SelectedParameterIndexRaw = clamp(grid.SelectedParameterIndexRaw, 0.0f, static_cast<float>(CellDataType::NumCellDataTypes) - 0.001f);
	grid.SelectedParameterIndex = static_cast<CellDataType>(grid.SelectedParameterIndexRaw);
	if (grid.SelectedParameterIndex != old) {
		grid.LoadParamForEditing();
		auto cd = CellDefinition::All[static_cast<size_t>(grid.SelectedParameterIndex)];
		grid.Algorithm->HelpText.DisplayHelpText(cd.HelpTextX, cd.HelpText);
	}
}


void GridView::OnPot3TurnHandler(ViewBase* view, float val) {
	auto& grid = *static_cast<GridView*>(view);
	if (grid.Editable) {
		auto cd = CellDefinition::All[static_cast<size_t>(grid.SelectedParameterIndex)];
		grid.Algorithm->PotMgr.UpdateValueWithPot(2, val, grid.ParamEditRaw, cd.ScaledMin(), cd.ScaledMax() + cd.Epsilon());
		grid.Algorithm->StepData.SetBaseCellValue(grid.SelectedCell.x, grid.SelectedCell.y, grid.SelectedParameterIndex, grid.ParamEditRaw, true);
		grid.Algorithm->HelpText.DisplayHelpText(cd.HelpTextX, cd.HelpText);
	}
}


void GridView::OnPot3ShortPressHandler(ViewBase* view) {
	auto& grid = *static_cast<GridView*>(view);
		// only change values if we are editable
	if (grid.Editable) {
		auto cd = CellDefinition::All[static_cast<size_t>(grid.SelectedParameterIndex)];
		grid.ParamEditRaw = cd.ScaledDefault() + cd.Epsilon();
		grid.Algorithm->StepData.SetBaseCellValue(grid.SelectedCell.x, grid.SelectedCell.y, grid.SelectedParameterIndex, grid.ParamEditRaw, true);
		grid.Algorithm->HelpText.DisplayHelpText(cd.HelpTextX, cd.HelpText);
		grid.LoadParamForEditing();
	}
}


void GridView::OnPot3LongPressHandler(ViewBase* view) {
	auto& grid = *static_cast<GridView*>(view);
	if (grid.Editable) {
		auto cd = CellDefinition::All[static_cast<size_t>(grid.SelectedParameterIndex)];
		grid.ParamEditRaw = cd.ScaledDefault() + cd.Epsilon();
		for (int x = 0; x < GridSizeX; x++) {
			for (int y = 0; y < GridSizeY; y++) {
				grid.Algorithm->StepData.SetBaseCellValue(x, y, grid.SelectedParameterIndex, grid.ParamEditRaw, true);
			}
		}
		grid.Algorithm->HelpText.DisplayHelpText(cd.HelpTextX, cd.HelpText);
		grid.LoadParamForEditing();
	}
}


void GridView::OnFixupPotValuesHandler(ViewBase* view, _NT_float3& pots) {
	auto& grid = *static_cast<GridView*>(view);
	// calculate an epsilon that we can add to our value to put it exactly in between pot "ticks"
	// this way we aren't right on the edge, where a slight pot bump could change the value
	auto epsilon2 = 0.5 / static_cast<int>(CellDataType::NumCellDataTypes);
	pots[1] = static_cast<float>(grid.SelectedParameterIndex) / static_cast<int>(CellDataType::NumCellDataTypes) + epsilon2;

	// we don't need to calculate anything here, just pass back a 0.5, and our soft takeover logic will handle it from there.
	// If we try to calculate it, it can go wrong thusly:
	// 1. We pass a zero because the cell we are on has a zero value
	// 2. We change cell types to another cell which actually does have a value.
	// 3. We already told the NT the value was zero, and have no way to update what NT thinks the value is
	// 4. User spins the pot, and value jumps to zero because NT thinks there is no room left on the low end of the knob
	// 5. In reality, we are on a new cell that CAN be adjusted down
	// Therefore we stick the value to the midpoint, and let our soft takeover logic handle it!
	pots[2] = 0.5;
	grid.LoadParamForEditing();
}


GridView::GridView() {
	SelectedCell = { .x = 0, .y = 0};
	SelectedParameterIndex = CellDataType::Direction;

	OnActivate = OnActivateHandler;
	OnDraw = OnDrawHandler;
	OnEncoder1Turn = OnEncoder1TurnHandler;
	OnEncoder2Turn = OnEncoder2TurnHandler;
	OnEncoder2ShortPress = OnEncoder2ShortPressHandler;
	OnEncoder2LongPress = OnEncoder2LongPressHandler;
	OnPot1Turn = OnPot1TurnHandler;
	OnPot2Turn = OnPot2TurnHandler;
	OnPot3Turn = OnPot3TurnHandler;
	OnPot3ShortPress = OnPot3ShortPressHandler;
	OnPot3LongPress = OnPot3LongPressHandler;
	OnFixupPotValues = OnFixupPotValuesHandler;
}


void GridView::InjectDependencies(DirSeqAlg* alg) {
	Algorithm = alg;
	ViewBase::InjectDependencies(&alg->Timer);
}


Bounds GridView::CellCoordsToBounds(const CellCoords& coords) const {
	Bounds result; // TODO:  should this be preallocated?
	result.x1 = coords.x * CellSize + GridPosition.x;
	result.y1 = coords.y * CellSize + GridPosition.y;
	result.x2 = coords.x * CellSize + GridPosition.x + CellSize;
	result.y2 = coords.y * CellSize + GridPosition.y + CellSize;
	return result;
}


void GridView::DrawCells() const {
	for(uint8_t x = 0; x < GridSizeX; x++) {
		for(uint8_t y = 0; y < GridSizeY; y++) {
			// is this cell selected?
			bool selected = (x == SelectedCell.x) && (y == SelectedCell.y);
			// is this the current step?
			bool current = (x == Algorithm->Playheads[SelectedPlayheadIndex].CurrentStep.x) && (y == Algorithm->Playheads[SelectedPlayheadIndex].CurrentStep.y);

			CellCoords coords { static_cast<int8_t>(x), static_cast<int8_t>(y) };
			auto cb = CellCoordsToBounds(coords);
			
			// draw the inner part of the cell, depending on what is selected/current
			DrawCell(x, y, selected, cb.x1, cb.y1, cb.x2, cb.y2);
			if (current) {
				NT_drawShapeI(kNT_box, cb.x1 + 1, cb.y1 + 1, cb.x2 - 1, cb.y2 - 1, 15);
				bool tie = (Algorithm->Playheads[SelectedPlayheadIndex].Tie != Playhead::TieMode::None);
				if (tie) {
					NT_drawShapeI(kNT_box, cb.x1 + 2, cb.y1 + 2, cb.x2 - 2, cb.y2 - 2, 10);
					NT_drawShapeI(kNT_box, cb.x1 + 3, cb.y1 + 3, cb.x2 - 3, cb.y2 - 3, 5);
				}

			}

			// draw the cell border
			NT_drawShapeI(kNT_box, cb.x1, cb.y1, cb.x2, cb.y2, CellBorderColor);

		}
	}
}


void GridView::DrawInitialCellBorder() const {
	auto cb = CellCoordsToBounds(Algorithm->Playheads[SelectedPlayheadIndex].InitialStep);
	NT_drawShapeI(kNT_box, cb.x1, cb.y1, cb.x2, cb.y2, CellBorderColor);
	auto marqueeColor = CellBorderColor + (Editable ? 10 : 5);
	for (int x = cb.x1; x <= cb.x2; x+=2)	{
		NT_drawShapeI(kNT_line, x, cb.y1, x, cb.y1, marqueeColor);
		NT_drawShapeI(kNT_line, x, cb.y2, x, cb.y2, marqueeColor);
	}
	for (int y = cb.y1; y <= cb.y2; y+=2)	{
		NT_drawShapeI(kNT_line, cb.x1, y, cb.x1, y, marqueeColor);
		NT_drawShapeI(kNT_line, cb.x2, y, cb.x2, y, marqueeColor);
	}
}


void GridView::DrawSelectedCellBorder() const {
	auto cb = CellCoordsToBounds(SelectedCell);
	auto color = Editable ? EditableCellBorderColor : NonEditableCellBorderColor;
	NT_drawShapeI(kNT_box, cb.x1 - 2, cb.y1 - 2, cb.x2 + 2, cb.y2 + 2, NonEditableCellBorderColor);
	NT_drawShapeI(kNT_box, cb.x1 - 1, cb.y1 - 1, cb.x2 + 1, cb.y2 + 1, color);
	auto& initial = Algorithm->Playheads[SelectedPlayheadIndex].InitialStep;
	if (SelectedCell.x != initial.x || SelectedCell.y != initial.y) {
		NT_drawShapeI(kNT_box, cb.x1, cb.y1, cb.x2, cb.y2, 0);
	}
}


void GridView::DrawBullet(int x, int y, int color) const {
	NT_drawShapeI(kNT_rectangle, x, y, x + 2, y + 2, color * 0.4);
	NT_drawShapeI(kNT_line, x + 1, y, x + 1, y + 2, color);
	NT_drawShapeI(kNT_line, x, y + 1, x + 2, y + 1, color);
}


void GridView::DrawParamLine(int paramIndex, int top) const {
	auto paramListX = GridPosition.x + (GridSizeX * CellSize) + 5;
	auto paramNameX = paramListX + 5;
	auto paramValueX = paramListX + 5 + 68;

	auto yoffset = (top) * TextLineHeight + 2;
	auto y = TextLineHeight * (paramIndex + 1) - yoffset;

	auto selected = paramIndex == static_cast<int>(SelectedParameterIndex);
	auto color = (selected && Editable) ? SelectedParameterColor : UnselectedParameterColor;
	if (selected) {
		DrawBullet(paramListX, y - 5, color);
	}

	auto idx = static_cast<CellDataType>(paramIndex);
	auto cd = CellDefinition::All[paramIndex];
	char buf[15];
	auto len = strlen(cd.DisplayName);
	strncpy(buf, cd.DisplayName, len);

	if (idx == CellDataType::Value && (Algorithm->v[kParamAttenValue] != 1000 || Algorithm->v[kParamOffsetValue] != 0)) {
		buf[len++] = '*';
	} else if (idx == CellDataType::Velocity && (Algorithm->v[kParamVelocityAttenuate] != 1000 || Algorithm->v[kParamVelocityOffset] != 0)) {
		buf[len++] = '*';
	} else if (idx == CellDataType::GateLength && Algorithm->v[kParamGateLengthAttenuate] != 1000) {
		buf[len++] = '*';
	}
	buf[len] = 0;

	NT_drawText(paramNameX, y, buf, color);

	float adjusted = Algorithm->StepData.GetAdjustedCellValue(SelectedCell.x, SelectedCell.y, idx);
	DrawParamLineValue(paramValueX, y, color, idx, cd, adjusted);
}


void GridView::DrawParamLineValue(int x, int y, int color, CellDataType ct, const CellDefinition& cd, float fval) const {
	// if the value is negative, keep it lined up with the others
	if (fval < 0) {
		x -= 6;
	}

	int ival = static_cast<int>(fval);
	switch (ct)
	{
		using enum CellDataType;
		case Direction:
			DrawDirectionArrow(ival, x - 3, y - TextLineHeight, color);
			break;
		case Value:
			NT_floatToString(&NumToStrBuf[0], fval, cd.Scaling);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case Velocity:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case Probability:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case Ratchets:
			// ratchets display is 1-8, but the packed field and underlying calcs use 0-7, so add 1 here when rendering
			NT_intToString(&NumToStrBuf[0], ival + 1);
			NT_drawText(x, y, ival == 0 ? "--" : NumToStrBuf, color);
			break;
		case RestAfter:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, ival == 0 ? "--" : NumToStrBuf, color);
			break;
		case GateLength:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case DriftProb:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case MaxDrift:
			NT_floatToString(&NumToStrBuf[0], fval, cd.Scaling);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case Repeats:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, ival == 0 ? "--" : NumToStrBuf, color);
			break;
		case Glide:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case AccumAdd:
			NT_floatToString(&NumToStrBuf[0], fval, cd.Scaling);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case AccumTimes:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, ival == 0 ? "--" : NumToStrBuf, color);
			break;
		case TieSteps:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, ival == 0 ? "--" : NumToStrBuf, color);
			break;
		case Mute:
			NT_drawText(x, y, ival == 0 ? "--" : "Yes", color);
			break;
		default:
			break;
	}
}


void GridView::DrawParams() const {
	auto top = max(static_cast<int>(SelectedParameterIndex) - 2, 0);
	int paramCount = static_cast<int>(CellDataType::NumCellDataTypes);
	top = min(top, paramCount - 5);
	for (int i = top; i < top + 5; i++) {
		DrawParamLine(i, top);
	}
}


void GridView::DrawPlayheadIcon(int x, int y, int width, int color) const {
	NT_drawShapeI(kNT_rectangle, x,             y - TextLineHeight + 2, x + width,     y,     color);
	NT_drawShapeI(kNT_rectangle, x + width + 1, y - TextLineHeight + 3, x + width + 1, y - 1, color);
	NT_drawShapeI(kNT_rectangle, x + width + 2, y - TextLineHeight + 4, x + width + 2, y - 2, color);
	NT_drawShapeI(kNT_rectangle, x + width + 3, y - TextLineHeight + 5, x + width + 3, y - 3, color);
	NT_drawShapeI(kNT_rectangle, x + width + 4, y - TextLineHeight + 6, x + width + 4, y - 4, color);
}


void GridView::DrawPlayheadLine(int playheadIndex, int top) const {
	if (playheadIndex >= Algorithm->Playheads.Count)
		return;

	auto x = 0;

	auto yoffset = (top) * TextLineHeight + 2;
	auto y = TextLineHeight * (playheadIndex + 1) - yoffset;

	auto selected = playheadIndex == SelectedPlayheadIndex;
	auto color = (selected && Editable) ? SelectedParameterColor : UnselectedParameterColor;

	char buf[7];
	strncpy(buf, "Head X", 7);
	buf[5] = 'A' + playheadIndex;
	buf[6] = 0;
	NT_drawText(x + 1, y, buf, color);

	if (selected) {
		DrawPlayheadIcon(x + 37, y, 4, color);
	}
}


void GridView::DrawPlayheadList() const {
	auto top = max(SelectedPlayheadIndex - 2, 0);
	top = min(top, Algorithm->Playheads.Count - 5);
	top = max(top, 0);
	for (int i = top; i < top + 5; i++) {
		DrawPlayheadLine(i, top);
	}
}


void GridView::DrawHelpSection() const {
	NT_drawShapeI(kNT_rectangle, 0, 50, 255, 63, 0);
	if (!Algorithm->HelpText.Draw()) {
		if (Editable) {
			NT_drawText(142, 58, "Q: Lock, L: Set Start", 15, kNT_textLeft, kNT_textTiny);
		} else {
			NT_drawText(138, 58, "Q: Unlock, L: Set Start", 15, kNT_textLeft, kNT_textTiny);
		}
		NT_drawText(55, 64, "Move X", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(175, 64, "Move Y", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(2, 64, "Select Head", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(105, 64, "Select Attr.", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(220, 64, "Set Attr.", 15, kNT_textLeft, kNT_textTiny);
	}
	NT_drawShapeI(kNT_line, 0, 50, 255, 50, 15);
}


void GridView::DrawCellPercentage(float val, bool selected, int x1, int y1, int x2, int y2) const {
	auto color = selected ? CellBrightColor : CellDimColor;
	auto size = CellSize - 3;
	auto scaled = val * size / 100.0f;
	auto full = static_cast<int>(scaled);
	auto frac = scaled - full;
	auto fracColor = static_cast<int>(frac * color);
	// don't let the fractional color get below 1 if this is our only "row"
	if (full == 0 && frac > 0 && fracColor < 1) {
		fracColor = 1;
	}
	if (full > 0) {
		NT_drawShapeI(kNT_rectangle, x1 + 2, y2 - full - 1, x2 - 2, y2 - 2, color);
	}
	if (frac > 0) {
		NT_drawShapeI(kNT_rectangle, x1 + 2, y2 - full - 2, x2 - 2, y2 - 2 - full, fracColor);
	}
}


void GridView::DrawCellValue(float val, bool selected, int x1, int y1, int x2, int y2) const {
	DrawCellPercentage(val * 10, selected, x1, y1, x2, y2);
}


void GridView::DrawCellVelocity(float val, bool selected, int x1, int y1, int x2, int y2) const {
	// convert velocity to a percentage
	DrawCellPercentage(val / 1.27, selected, x1, y1, x2, y2);
}


void GridView::DrawCellNumber(int16_t val, bool selected, int x1, int y1, int x2, int y2, int16_t specialCase, const char *specialCaseTxt) const {
	if (val >= 0 && val <= 9) {
		auto color = selected ? CellBrightColor : CellDimColor;
		int xoff = val == 1 ? 1 : 0;
		char buf[2];
		NT_intToString(buf, val);
		auto txt = (val == specialCase) ? specialCaseTxt : buf;
		NT_drawText(x1 + 4 + xoff, y1 + 10, txt, color);
	}
}


void GridView::DrawCellMute(int16_t val, bool selected, int x1, int y1, int x2, int y2) const {
	if (val >= 0 && val <= 1) {
		auto color = selected ? CellBrightColor : CellDimColor;
		auto txt = (val == 0) ? "" : "M";
		NT_drawText(x1 + 4, y1 + 10, txt, color);
	}
}


void GridView::DrawCellBipolarValue(float val, bool selected, int x1, int y1, int x2, int y2) const {
	if (val == 0) return;
	auto color = selected ? CellBrightColor : CellDimColor;
	int size = (CellSize - 3 + 1) / 2;
	auto scaled = val * size;
	auto absval = abs(scaled);
	auto full = static_cast<int>(absval);
	auto frac = absval - full;
	auto midy = y1 + (CellSize / 2);
	auto fracColor = static_cast<int>(frac * color);
	if (val > 0) {
		if (full > 0) {
			NT_drawShapeI(kNT_rectangle, x1 + 2, midy - full + 1, x2 - 2, midy, color);
		}
		if (frac > 0) {
			NT_drawShapeI(kNT_rectangle, x1 + 2, midy - full, x2 - 2, midy - full, fracColor);
		}
	} else {
		if (full > 0) {
			NT_drawShapeI(kNT_rectangle, x1 + 2, midy, x2 - 2, midy + full - 1, color);
		}
		if (frac > 0) {
			NT_drawShapeI(kNT_rectangle, x1 + 2, midy + full, x2 - 2, midy + full, fracColor);
		}
	}
}


void GridView::DrawCellAccumAdd(float val, bool selected, int x1, int y1, int x2, int y2) const {
	auto cd = CellDefinition::All[static_cast<size_t>(CellDataType::AccumAdd)];
	DrawCellBipolarValue(val / cd.ScaledMax(), selected, x1, y1, x2, y2);
}


void GridView::DrawCell(uint8_t cx, uint8_t cy, bool selected, int x1, int y1, int x2, int y2) const {
	using enum CellDataType;
	float val = Algorithm->StepData.GetAdjustedCellValue(cx, cy, SelectedParameterIndex);
	switch (SelectedParameterIndex)	{
		case Direction:
			DrawDirectionArrow(val, x1, y1, selected ? CellBrightColor : CellDimColor);
			break;
		case Value:
			DrawCellValue(val, selected, x1, y1, x2, y2);
			break;
		case Velocity:
			DrawCellVelocity(val, selected, x1, y1, x2, y2);
			break;
		case Probability:
			DrawCellPercentage(val, selected, x1, y1, x2, y2);
			break;
		case Ratchets:
			// ratchets display is 1-8, but the packed field and underlying calcs use 0-7, so add 1 here when rendering
			DrawCellNumber(val + 1, selected, x1, y1, x2, y2, 1, "");
			break;
		case RestAfter:
			DrawCellNumber(val, selected, x1, y1, x2, y2, 0, "");
			break;
		case GateLength:
			DrawCellPercentage(val, selected, x1, y1, x2, y2);
			break;
		case DriftProb:
			DrawCellPercentage(val, selected, x1, y1, x2, y2);
			break;
		case MaxDrift:
			DrawCellValue(val, selected, x1, y1, x2, y2);
			break;
		case Repeats:
			DrawCellNumber(val, selected, x1, y1, x2, y2, 0, "");
			break;
		case Glide:
			DrawCellPercentage(val, selected, x1, y1, x2, y2);
			break;
		case AccumAdd:
			DrawCellAccumAdd(val, selected, x1, y1, x2, y2);
			break;
		case AccumTimes:
			DrawCellNumber(val, selected, x1, y1, x2, y2, 0, "");
			break;
		case TieSteps:
			DrawCellNumber(val, selected, x1, y1, x2, y2, 0, "");
			break;
		case Mute:
			DrawCellMute(val, selected, x1, y1, x2, y2);
			break;
		default:
			break;
	}
}


void GridView::DrawDirectionArrow(unsigned int dir, int x, int y, int color) const {
	if (dir < 1 || dir > 8)
		return;

	// relative coordinates for lines making up each arrow direction
	static const int8_t lineData[] = {
		/* N  */ 6,2,6,10, 5,3,7,3, 4,4,8,4, 3,5,9,5,
		/* NE */ 3,9,9,3,  5,3,9,3, 6,4,9,4, 9,3,9,7, 8,3,8,6,
		/* E  */ 2,6,10,6, 9,5,9,7, 8,4,8,8, 7,3,7,9,
		/* SE */ 3,3,9,9,  9,5,9,9, 8,6,8,9, 5,9,9,9, 6,8,9,8,
		/* S  */ 6,2,6,10, 5,9,7,9, 4,8,8,8, 3,7,9,7,
		/* SW */ 3,9,9,3,  3,5,3,9, 4,6,4,9, 3,8,6,8, 3,9,7,9,
		/* W  */ 2,6,10,6, 3,5,3,7, 4,4,4,8, 5,3,5,9,
		/* NW */ 3,3,9,9,  3,3,7,3, 3,4,6,4, 3,3,3,7, 4,3,4,6
	};

	// the offset into the lineData array for the start of each direction's data
	static const uint8_t offsets[] = { 0, 16, 36, 52, 72, 88, 108, 124 };
	// the number of lines to draw for each direction
	static const uint8_t counts[]  = { 4, 5, 4, 5, 4, 5, 4, 5 };

	const int8_t* p = &lineData[offsets[dir - 1]];
	for (int i = 0; i < counts[dir - 1]; i++) {
		NT_drawShapeI(kNT_line, x + p[0], y + p[1], x + p[2], y + p[3], color);
		p += 4;
	}
}


void GridView::LoadParamForEditing() {
	auto cd = CellDefinition::All[static_cast<size_t>(SelectedParameterIndex)];
	ParamEditRaw = Algorithm->StepData.GetBaseCellValue(SelectedCell.x, SelectedCell.y, SelectedParameterIndex) + cd.Epsilon();
}
