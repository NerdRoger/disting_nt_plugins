#include <stdio.h>
#include <math.h>
#include <distingnt/api.h>
#include "common.h"
#include "gridView.h"
#include "helpTextHelper.h"
#include "dirSeqAlg.h"


GridView::GridView() {
	SelectedCell = { .x = 0, .y = 0};
	SelectedParameterIndex = CellDataType::Direction;
}


void GridView::InjectDependencies(const CellDefinition* cellDefs, TimeKeeper* timer, StepDataRegion* stepData, HelpTextHelper* helpText, PotManager* potMgr, PlayheadList* playheads) {
	ViewBase::InjectDependencies(timer);
	CellDefs = cellDefs;
	StepData = stepData;
	HelpText = helpText;
	PotMgr = potMgr;
	Playheads = playheads;
}


Bounds GridView::CellCoordsToBounds(const CellCoords& coords) const {
	Bounds result; // TODO:  should this be preallocated?
	result.x1 = coords.x * CellSize + GridPosition.x;
	result.y1 = coords.y * CellSize + GridPosition.y;
	result.x2 = coords.x * CellSize + GridPosition.x + CellSize;
	result.y2 = coords.y * CellSize + GridPosition.y + CellSize;
	return result;
}


void GridView::Draw() const {
	DrawCells();
	DrawInitialCellBorder();
	DrawSelectedCellBorder();
	DrawPlayheadList();
	DrawParams();
	DrawHelpSection();


	// TODO:  remove this test code ad the end of development
	// NT_drawShapeI(kNT_rectangle, 0, 0, 50, 50, 0);
	// NT_floatToString(&NumToStrBuf[0], ParamEditRaw, 3);
	// NT_drawText(0, 10, NumToStrBuf, 15);
	// NT_floatToString(&NumToStrBuf[0], tempval, 3);
	// NT_drawText(0, 20, NumToStrBuf, 15);


	// NT_floatToString(&NumToStrBuf[0], p3, 3);
	// NT_drawText(0, 20, NumToStrBuf, 15);
	// NT_floatToString(&NumToStrBuf[0], SelectedParameterIndexRaw, 3);
	// NT_drawText(0, 30, NumToStrBuf, 15);
}


void GridView::DrawCells() const {
	for(uint8_t x = 0; x < GridSizeX; x++) {
		for(uint8_t y = 0; y < GridSizeY; y++) {
			// is this cell selected?
			bool selected = (x == SelectedCell.x) && (y == SelectedCell.y);
			// is this the current step?
			bool current = (x == (*Playheads)[SelectedPlayheadIndex].CurrentStep.x) && (y == (*Playheads)[SelectedPlayheadIndex].CurrentStep.y);

			CellCoords coords { static_cast<int8_t>(x), static_cast<int8_t>(y) };
			auto cb = CellCoordsToBounds(coords);
			
			// draw the inner part of the cell, depending on what is selected/current
			if (current) {
					NT_drawShapeI(kNT_box, cb.x1 + 1, cb.y1 + 1, cb.x2 - 1, cb.y2 - 1, 15);
			}
			DrawCell(x, y, selected, cb.x1, cb.y1, cb.x2, cb.y2);

			// draw the cell border
			NT_drawShapeI(kNT_box, cb.x1, cb.y1, cb.x2, cb.y2, CellBorderColor);

		}
	}
}


void GridView::DrawInitialCellBorder() const {
	auto cb = CellCoordsToBounds((*Playheads)[SelectedPlayheadIndex].InitialStep);
	NT_drawShapeI(kNT_box, cb.x1, cb.y1, cb.x2, cb.y2, CellBorderColor);
	auto marqueeColor = CellBorderColor + 5;
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
	auto& initial = (*Playheads)[SelectedPlayheadIndex].InitialStep;
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
	const auto& cd = CellDefs[paramIndex];
	NT_drawText(paramNameX, y, cd.DisplayName, color);

	float base = StepData->GetBaseCellValue(SelectedCell.x, SelectedCell.y, idx);
	float adjusted = StepData->GetAdjustedCellValue(SelectedCell.x, SelectedCell.y, idx);
	DrawParamLineValue(paramValueX, y, color, idx, cd, base);
//	if (StepData->CellTypeHasMapping(idx)) {
	if (base != adjusted) {
		DrawParamLineValue(paramValueX + 37, y, color, idx, cd, adjusted);
	}
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


void GridView::DrawPlayheadIcon(int x, int y, int color) const {
	NT_drawShapeI(kNT_rectangle, x,      y - TextLineHeight + 2, x + 6,  y,     color);
	NT_drawShapeI(kNT_rectangle, x +  7, y - TextLineHeight + 3, x + 7,  y - 1, color);
	NT_drawShapeI(kNT_rectangle, x +  8, y - TextLineHeight + 4, x + 8,  y - 2, color);
	NT_drawShapeI(kNT_rectangle, x +  9, y - TextLineHeight + 5, x + 9,  y - 3, color);
	NT_drawShapeI(kNT_rectangle, x + 10, y - TextLineHeight + 6, x + 10, y - 4, color);
}


void GridView::DrawPlayheadLine(int playheadIndex, int top) const {
	if (playheadIndex >= Playheads->Count)
		return;

	auto x = 0;

	auto yoffset = (top) * TextLineHeight + 2;
	auto y = TextLineHeight * (playheadIndex + 1) - yoffset;

	auto selected = playheadIndex == SelectedPlayheadIndex;
	auto color = (selected && Editable) ? SelectedParameterColor : UnselectedParameterColor;
	if (selected) {
		DrawPlayheadIcon(x, y, 2);
	}

	char buf[2];
	buf[0] = 'A' + playheadIndex;
	buf[1] = 0;
	NT_drawText(x + 1, y, buf, color);
}


void GridView::DrawPlayheadList() const {
	auto top = max(SelectedPlayheadIndex - 2, 0);
	top = min(top, Playheads->Count - 5);
	top = max(top, 0);
	for (int i = top; i < top + 5; i++) {
		DrawPlayheadLine(i, top);
	}
}


void GridView::DrawHelpSection() const {
	NT_drawShapeI(kNT_rectangle, 0, 50, 255, 63, 0);
	if (!HelpText->Draw()) {
		if (Editable) {
			NT_drawText(142, 58, "Q: Lock, L: Set Start", 15, kNT_textLeft, kNT_textTiny);
		} else {
			NT_drawText(138, 58, "Q: Unlock, L: Set Start", 15, kNT_textLeft, kNT_textTiny);
		}
		NT_drawText(55, 64, "Move X", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(175, 64, "Move Y", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(2, 64, "Select Head", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(107, 64, "Select Opt.", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(226, 64, "Set Opt.", 15, kNT_textLeft, kNT_textTiny);
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



void GridView::DrawCell(uint8_t cx, uint8_t cy, bool selected, int x1, int y1, int x2, int y2) const {
	using enum CellDataType;
	float val = StepData->GetAdjustedCellValue(cx, cy, SelectedParameterIndex);
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
			DrawCellBipolarValue(val, selected, x1, y1, x2, y2);
			break;
		case AccumTimes:
			DrawCellNumber(val, selected, x1, y1, x2, y2, 0, "");
			break;
		default:
			break;
	}
}


void GridView::DrawDirectionArrow(unsigned int dir, int x, int y, int color) const {
	switch (dir)
	{
		case 1: // North
			NT_drawShapeI(kNT_line, x + 6, y + 2, x + 6, y + 10, color);
			NT_drawShapeI(kNT_line, x + 5, y + 3, x + 7, y +  3, color);
			NT_drawShapeI(kNT_line, x + 4, y + 4, x + 8, y +  4, color);
			NT_drawShapeI(kNT_line, x + 3, y + 5, x + 9, y +  5, color);
			break;
		case 2: // NorthEast
			NT_drawShapeI(kNT_line, x + 3, y + 9, x + 9, y + 3, color);
			NT_drawShapeI(kNT_line, x + 5, y + 3, x + 9, y + 3, color);
			NT_drawShapeI(kNT_line, x + 6, y + 4, x + 9, y + 4, color);
			NT_drawShapeI(kNT_line, x + 9, y + 3, x + 9, y + 7, color);
			NT_drawShapeI(kNT_line, x + 8, y + 3, x + 8, y + 6, color);
			break;
		case 3: // East
			NT_drawShapeI(kNT_line, x + 2, y + 6, x + 10, y + 6, color);
			NT_drawShapeI(kNT_line, x + 9, y + 5, x +  9, y + 7, color);
			NT_drawShapeI(kNT_line, x + 8, y + 4, x +  8, y + 8, color);
			NT_drawShapeI(kNT_line, x + 7, y + 3, x +  7, y + 9, color);
			break;
		case 4: // SouthEast
			NT_drawShapeI(kNT_line, x + 3, y + 3, x + 9, y + 9, color);
			NT_drawShapeI(kNT_line, x + 9, y + 5, x + 9, y + 9, color);
			NT_drawShapeI(kNT_line, x + 8, y + 6, x + 8, y + 9, color);
			NT_drawShapeI(kNT_line, x + 5, y + 9, x + 9, y + 9, color);
			NT_drawShapeI(kNT_line, x + 6, y + 8, x + 9, y + 8, color);
			break;
		case 5: // South
			NT_drawShapeI(kNT_line, x + 6, y + 2, x + 6, y + 10, color);
			NT_drawShapeI(kNT_line, x + 5, y + 9, x + 7, y +  9, color);
			NT_drawShapeI(kNT_line, x + 4, y + 8, x + 8, y +  8, color);
			NT_drawShapeI(kNT_line, x + 3, y + 7, x + 9, y +  7, color);
			break;
		case 6: // SouthWest
			NT_drawShapeI(kNT_line, x + 3, y + 9, x + 9, y + 3, color);
			NT_drawShapeI(kNT_line, x + 3, y + 5, x + 3, y + 9, color);
			NT_drawShapeI(kNT_line, x + 4, y + 6, x + 4, y + 9, color);
			NT_drawShapeI(kNT_line, x + 3, y + 8, x + 6, y + 8, color);
			NT_drawShapeI(kNT_line, x + 3, y + 9, x + 7, y + 9, color);
			break;
		case 7: // West
			NT_drawShapeI(kNT_line, x + 2, y + 6, x + 10, y + 6, color);
			NT_drawShapeI(kNT_line, x + 3, y + 5, x +  3, y + 7, color);
			NT_drawShapeI(kNT_line, x + 4, y + 4, x +  4, y + 8, color);
			NT_drawShapeI(kNT_line, x + 5, y + 3, x +  5, y + 9, color);
			break;
		case 8: // NorthWest
			NT_drawShapeI(kNT_line, x + 3, y + 3, x + 9, y + 9, color);
			NT_drawShapeI(kNT_line, x + 3, y + 3, x + 7, y + 3, color);
			NT_drawShapeI(kNT_line, x + 3, y + 4, x + 6, y + 4, color);
			NT_drawShapeI(kNT_line, x + 3, y + 3, x + 3, y + 7, color);
			NT_drawShapeI(kNT_line, x + 4, y + 3, x + 4, y + 6, color);
			break;
		default:
			break;
	}
}

// calculate an epsilon for a given cell parameter that we can add to our value to put it exactly in between pot "ticks"
float GridView::CalculateEpsilon(const CellDefinition& cd) const {
	return 0.5 * static_cast<int16_t>(pow(10, -cd.Scaling));
}


void GridView::LoadParamForEditing() {
	const auto& cd = CellDefs[static_cast<size_t>(SelectedParameterIndex)];
	ParamEditRaw = StepData->GetBaseCellValue(SelectedCell.x, SelectedCell.y, SelectedParameterIndex) + CalculateEpsilon(cd);
}


void GridView::Encoder1Turn(int8_t x) {
	SelectedCell.x = wrap(SelectedCell.x + x, 0, GridSizeX - 1);
	LoadParamForEditing();
}


void GridView::Encoder2Turn(int8_t x) {
	SelectedCell.y = wrap(SelectedCell.y + x, 0, GridSizeY - 1);
	LoadParamForEditing();
}


void GridView::Encoder2ShortPress() {
	Editable = !Editable;
}


void GridView::Encoder2LongPress() {
	(*Playheads)[SelectedPlayheadIndex].InitialStep = SelectedCell;
}


void GridView::Pot1Turn(float val) {
	auto old = SelectedPlayheadIndex;
	PotMgr->UpdateValueWithPot(0, val, SelectedPlayheadIndexRaw, 0, Playheads->Count);
	SelectedPlayheadIndexRaw = clamp(SelectedPlayheadIndexRaw, 0.0f, static_cast<float>(Playheads->Count) - 0.001f);
	SelectedPlayheadIndex = SelectedPlayheadIndexRaw;
	if (SelectedPlayheadIndex != old) {
		StringConcat(PlayheadHelpText, 20, "Playhead X Selected");
		PlayheadHelpText[9] = 'A' + SelectedPlayheadIndex;
		HelpText->DisplayHelpText(70, PlayheadHelpText);
	}
}


void GridView::Pot2Turn(float val) {
	auto old = SelectedParameterIndex;
	PotMgr->UpdateValueWithPot(1, val, SelectedParameterIndexRaw, 0, static_cast<float>(CellDataType::NumCellDataTypes));
	SelectedParameterIndexRaw = clamp(SelectedParameterIndexRaw, 0.0f, static_cast<float>(CellDataType::NumCellDataTypes) - 0.001f);
	SelectedParameterIndex = static_cast<CellDataType>(SelectedParameterIndexRaw);
	if (SelectedParameterIndex != old) {
		LoadParamForEditing();
		const auto& cd = CellDefs[static_cast<size_t>(SelectedParameterIndex)];
		HelpText->DisplayHelpText(cd.HelpTextX, cd.HelpText);
	}
}


void GridView::Pot3Turn(float val) {
	if (Editable) {
		const auto& cd = CellDefs[static_cast<size_t>(SelectedParameterIndex)];
		PotMgr->UpdateValueWithPot(2, val, ParamEditRaw, cd.Min, cd.Max + CalculateEpsilon(cd));
		StepData->SetBaseCellValue(SelectedCell.x, SelectedCell.y, SelectedParameterIndex, ParamEditRaw, true);
		HelpText->DisplayHelpText(cd.HelpTextX, cd.HelpText);
	}
}


void GridView::Pot3ShortPress() {
	// only change values if we are editable
	if (Editable) {
		const auto& cd = CellDefs[static_cast<size_t>(SelectedParameterIndex)];
		ParamEditRaw = cd.Default + CalculateEpsilon(cd);
		StepData->SetBaseCellValue(SelectedCell.x, SelectedCell.y, SelectedParameterIndex, ParamEditRaw, true);
		// default state for direction should give us an initial direction (east)
		if (SelectedParameterIndex == CellDataType::Direction) {
			if ((*Playheads)[SelectedPlayheadIndex].InitialStep == SelectedCell) {
				StepData->SetBaseCellValue(SelectedCell.x, SelectedCell.y, SelectedParameterIndex, 3, true);
			}
		}
		HelpText->DisplayHelpText(cd.HelpTextX, cd.HelpText);
		LoadParamForEditing();
	}
}


void GridView::Pot3LongPress() {
	if (Editable) {
		const auto& cd = CellDefs[static_cast<size_t>(SelectedParameterIndex)];
		ParamEditRaw = cd.Default + CalculateEpsilon(cd);
		for (int x = 0; x < GridSizeX; x++) {
			for (int y = 0; y < GridSizeY; y++) {
				StepData->SetBaseCellValue(x, y, SelectedParameterIndex, ParamEditRaw, true);
			}
		}
		// default state for direction should give us an initial direction (east)
		if (SelectedParameterIndex == CellDataType::Direction) {
			StepData->SetBaseCellValue((*Playheads)[SelectedPlayheadIndex].InitialStep.x, (*Playheads)[SelectedPlayheadIndex].InitialStep.y, SelectedParameterIndex, 3, true);
		}
		HelpText->DisplayHelpText(cd.HelpTextX, cd.HelpText);
		LoadParamForEditing();
	}
}


void GridView::FixupPotValues(_NT_float3& pots) {
	// calculate an epsilon that we can add to our value to put it exactly in between pot "ticks"
	// this way we aren't right on the edge, where a slight pot bump could change the value
	auto epsilon2 = 0.5 / static_cast<int>(CellDataType::NumCellDataTypes);
	pots[1] = static_cast<float>(SelectedParameterIndex) / static_cast<int>(CellDataType::NumCellDataTypes) + epsilon2;

	const auto& cd = CellDefs[static_cast<size_t>(SelectedParameterIndex)];
	auto range = cd.Max - cd.Min;
	pots[2] = StepData->GetBaseCellValue(SelectedCell.x, SelectedCell.y, SelectedParameterIndex) / range;
}


void GridView::Activate() {
	LoadParamForEditing();
}