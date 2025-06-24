#include <stdio.h>
#include <math.h>
#include <distingnt/api.h>
#include "common.h"
#include "gridMode.h"
#include "helpTextHelper.h"
#include "directionalSequencer.h"


GridMode::GridMode() {
	// TODO:  make these sensible
	SelectedCell = { .x = 0, .y = 0};
	SelectedParameterIndex = CellDataType::Direction;
}


Bounds GridMode::CellCoordsToBounds(const CellCoords& coords) const {
	Bounds result; // TODO:  should this be preallocated?
	result.x1 = coords.x * CellSize + GridPosition.x;
	result.y1 = coords.y * CellSize + GridPosition.y;
	result.x2 = coords.x * CellSize + GridPosition.x + CellSize;
	result.y2 = coords.y * CellSize + GridPosition.y + CellSize;
	return result;
}


void GridMode::DrawIcon(int x, int y, int color) const {
	NT_drawShapeI(kNT_box, x + 0, y + 0, x + 16, y +  8, color);
	NT_drawShapeI(kNT_box, x + 0, y + 4, x + 16, y + 12, color);
	NT_drawShapeI(kNT_box, x + 0, y + 8, x + 16, y + 16, color);
	NT_drawShapeI(kNT_box, x + 0, y + 0, x +  8, y + 16, color);
	NT_drawShapeI(kNT_box, x + 4, y + 0, x + 12, y + 16, color);
	NT_drawShapeI(kNT_box, x + 8, y + 0, x + 16, y + 16, color);
}


void GridMode::Draw() const {

	// TODO:  remove this test code ad the end of development
	// NT_drawShapeI(kNT_rectangle, 0, 0, 50, 50, 0);
	// NT_floatToString(&NumToStrBuf[0], p2, 3);
	// NT_drawText(0, 10, NumToStrBuf, 15);
	// NT_floatToString(&NumToStrBuf[0], p3, 3);
	// NT_drawText(0, 20, NumToStrBuf, 15);
	// NT_floatToString(&NumToStrBuf[0], SelectedParameterIndexRaw, 3);
	// NT_drawText(0, 30, NumToStrBuf, 15);


	DrawCells();
	DrawInitialCellBorder();
	DrawSelectedCellBorder();
	DrawParams();
	DrawHelpSection();
}


void GridMode::DrawCells() const {
	for(uint8_t x = 0; x < GridSizeX; x++) {
		for(uint8_t y = 0; y < GridSizeY; y++) {
			// is this cell selected?
			bool selected = (x == SelectedCell.x) && (y == SelectedCell.y);
			// is this the current step?
			bool current = (x == AlgorithmInstance->Seq.CurrentStep.x) && (y == AlgorithmInstance->Seq.CurrentStep.y);

			CellCoords coords { static_cast<int8_t>(x), static_cast<int8_t>(y) };
			auto cb = CellCoordsToBounds(coords);
			
			// draw the inner part of the cell, depending on what is selected/current
			if (current) {
					NT_drawShapeI(kNT_box, cb.x1 + 1, cb.y1 + 1, cb.x2 - 1, cb.y2 - 1, 15);
			}
			const auto& cell = AlgorithmInstance->Seq.Cells[x][y];
			DrawCell(cell, selected, cb.x1, cb.y1, cb.x2, cb.y2);

			// draw the cell border
			NT_drawShapeI(kNT_box, cb.x1, cb.y1, cb.x2, cb.y2, CellBorderColor);

		}
	}
}


void GridMode::DrawInitialCellBorder() const {
	auto cb = CellCoordsToBounds(AlgorithmInstance->Seq.InitialStep);
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


void GridMode::DrawSelectedCellBorder() const {
	auto cb = CellCoordsToBounds(SelectedCell);
	auto color = Editable ? EditableCellBorderColor : NonEditableCellBorderColor;
	NT_drawShapeI(kNT_box, cb.x1 - 2, cb.y1 - 2, cb.x2 + 2, cb.y2 + 2, NonEditableCellBorderColor);
	NT_drawShapeI(kNT_box, cb.x1 - 1, cb.y1 - 1, cb.x2 + 1, cb.y2 + 1, color);
	auto& initial = AlgorithmInstance->Seq.InitialStep;
	if (SelectedCell.x != initial.x || SelectedCell.y != initial.y) {
		NT_drawShapeI(kNT_box, cb.x1, cb.y1, cb.x2, cb.y2, 0);
	}
}


void GridMode::DrawBullet(int x, int y, int color) const {
	NT_drawShapeI(kNT_rectangle, x, y, x + 2, y + 2, color * 0.4);
	NT_drawShapeI(kNT_line, x + 1, y, x + 1, y + 2, color);
	NT_drawShapeI(kNT_line, x, y + 1, x + 2, y + 1, color);
}


void GridMode::DrawParamLine(int paramIndex, int top) const {
	auto paramListX = GridPosition.x + (GridSizeX * CellSize) + 5;
	auto paramNameX = paramListX + 5;
	auto paramValueX = paramListX + 5 + 68;

	auto lineHeight = 10;  // TODO:  look for variables like this one that can become constants
	auto yoffset = (top) * lineHeight + 2;
	auto y = lineHeight * (paramIndex + 1) - yoffset;

	auto selected = paramIndex == static_cast<int>(SelectedParameterIndex);
	auto color = (selected && Editable) ? SelectedParameterColor : UnselectedParameterColor;
	if (selected) {
		DrawBullet(paramListX, y - 5, color);
	}

	auto idx = static_cast<CellDataType>(paramIndex);
	const auto& cd = CellDefinitions[paramIndex];
	NT_drawText(paramNameX, y, cd.DisplayName, color);
	DrawParamLineValue(paramValueX, y, color, idx, cd);
}


void GridMode::DrawParamLineValue(int x, int y, int color, CellDataType ct, const CellDefinition& cd) const {
	const auto& cell = AlgorithmInstance->Seq.Cells[SelectedCell.x][SelectedCell.y];
	float fval = cell.GetField(*AlgorithmInstance, ct);


	// if the value is negativem keep it lined up with the others
	if (fval < 0) {
		x -= 6;
	}


	int ival = static_cast<int>(fval);
	switch (ct)
	{
		using enum CellDataType;
		// TODO:  consolidate cases once this is working
		case Direction:
			DrawDirectionArrow(ival, x - 3, y - 10, color);
			break;
		case Value:
			NT_floatToString(&NumToStrBuf[0], fval, cd.Precision);
			FixFloatBuf();
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
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case RestAfter:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, NumToStrBuf, color);
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
			NT_floatToString(&NumToStrBuf[0], fval, cd.Precision);
			FixFloatBuf();
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case Repeats:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case Glide:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case AccumAdd:
			NT_floatToString(&NumToStrBuf[0], fval, cd.Precision);
			FixFloatBuf();
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		case AccumTimes:
			NT_intToString(&NumToStrBuf[0], ival);
			NT_drawText(x, y, NumToStrBuf, color);
			break;
		default:
			break;
	}
}


void GridMode::DrawParams() const {
	auto top = max(static_cast<int>(SelectedParameterIndex) - 2, 0);
	int paramCount = ARRAY_SIZE(CellDefinitions);
	top = min(top, paramCount - 5);
	for (int i = top; i < top + 5; i++) {
		DrawParamLine(i, top);
	}
}


void GridMode::DrawHelpSection() const {
	NT_drawShapeI(kNT_rectangle, 0, 50, 255, 63, 0);
	if (!AlgorithmInstance->HelpText.Draw()) {
		if (Editable) {
			NT_drawText(142, 58, "Q: Lock, L: Set Start", 15, kNT_textLeft, kNT_textTiny);
		} else {
			NT_drawText(138, 58, "Q: Unlock, L: Set Start", 15, kNT_textLeft, kNT_textTiny);
		}
		NT_drawText(55, 64, "Move X", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(175, 64, "Move Y", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(2, 64, "Select Mode", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(107, 64, "Select Opt.", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(226, 64, "Set Opt.", 15, kNT_textLeft, kNT_textTiny);
	}
	NT_drawShapeI(kNT_line, 0, 50, 255, 50, 15);
}


void GridMode::DrawCellPercentage(float val, bool selected, int x1, int y1, int x2, int y2) const {
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


void GridMode::DrawCellValue(float val, bool selected, int x1, int y1, int x2, int y2) const {
	DrawCellPercentage(val * 10, selected, x1, y1, x2, y2);
}


void GridMode::DrawCellVelocity(float val, bool selected, int x1, int y1, int x2, int y2) const {
	// convert velocity to a percentage
	DrawCellPercentage(val / 1.27, selected, x1, y1, x2, y2);
}


void GridMode::DrawCellNumber(int16_t val, bool selected, int x1, int y1, int x2, int y2) const {
	if (val >= 0 && val <= 9) {
		auto color = selected ? CellBrightColor : CellDimColor;
		int xoff = val == 1 ? 1 : 0;
		char buf[2];
		NT_intToString(buf, val);
		NT_drawText(x1 + 4 + xoff, y1 + 10, buf, color);
	}
}


void GridMode::DrawCellBipolarValue(float val, bool selected, int x1, int y1, int x2, int y2) const {
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



void GridMode::DrawCell(const CellData& cell, bool selected, int x1, int y1, int x2, int y2) const {
	using enum CellDataType;
	float val = cell.GetField(*AlgorithmInstance, SelectedParameterIndex);
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
			DrawCellNumber(val, selected, x1, y1, x2, y2);
			break;
		case RestAfter:
			DrawCellNumber(val, selected, x1, y1, x2, y2);
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
			DrawCellNumber(val, selected, x1, y1, x2, y2);
			break;
		case Glide:
			DrawCellPercentage(val, selected, x1, y1, x2, y2);
			break;
		case AccumAdd:
			DrawCellBipolarValue(val, selected, x1, y1, x2, y2);
			break;
		case AccumTimes:
			DrawCellNumber(val, selected, x1, y1, x2, y2);
			break;
		default:
			break;
	}
}


void GridMode::DrawDirectionArrow(unsigned int dir, int x, int y, int color) const {
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
float GridMode::CalculateEpsilon(const CellDefinition& cd) const {
	return 0.5 * static_cast<int16_t>(pow(10, -cd.Precision));
}


void GridMode::LoadParamForEditing() {
	const auto& cd = CellDefinitions[static_cast<size_t>(SelectedParameterIndex)];
	const auto& cell = AlgorithmInstance->Seq.Cells[SelectedCell.x][SelectedCell.y];
	ParamEditRaw = cell.GetField(*AlgorithmInstance, SelectedParameterIndex) + CalculateEpsilon(cd);
}


void GridMode::Encoder1Turn(int8_t x) {
	SelectedCell.x = wrap(SelectedCell.x + x, 0, GridSizeX - 1);
	LoadParamForEditing();
}


void GridMode::Encoder2Turn(int8_t x) {
	SelectedCell.y = wrap(SelectedCell.y + x, 0, GridSizeY - 1);
	LoadParamForEditing();
}


void GridMode::Encoder2ShortPress() {
	Editable = !Editable;
}


void GridMode::Encoder2LongPress() {
	AlgorithmInstance->Seq.InitialStep = SelectedCell;
}


void GridMode::Pot2Turn(float val) {
//	p2 = val;

	auto old = SelectedParameterIndex;
	AlgorithmInstance->UpdateValueWithPot(1, val, SelectedParameterIndexRaw, 0, ARRAY_SIZE(CellDefinitions));
	SelectedParameterIndexRaw = clamp(SelectedParameterIndexRaw, 0.0f, ARRAY_SIZE(CellDefinitions) - 0.001f);
	SelectedParameterIndex = static_cast<CellDataType>(SelectedParameterIndexRaw);
	if (SelectedParameterIndex != old) {
		LoadParamForEditing();
		const auto& cd = CellDefinitions[static_cast<size_t>(SelectedParameterIndex)];
		AlgorithmInstance->HelpText.DisplayHelpText(cd.HelpText);
	}
}


void GridMode::Pot3Turn(float val) {
//	p3 = val;

	if (Editable) {
		const auto& cd = CellDefinitions[static_cast<size_t>(SelectedParameterIndex)];
		auto& cell = AlgorithmInstance->Seq.Cells[SelectedCell.x][SelectedCell.y];
		AlgorithmInstance->UpdateValueWithPot(2, val, ParamEditRaw, cd.Min, cd.Max + CalculateEpsilon(cd));
		cell.SetField(*AlgorithmInstance, SelectedParameterIndex, ParamEditRaw);
		AlgorithmInstance->HelpText.DisplayHelpText(cd.HelpText);
	}
}


void GridMode::Pot3ShortPress() {
	// only change values if we are editable
	if (Editable) {
		const auto& cd = CellDefinitions[static_cast<size_t>(SelectedParameterIndex)];
		auto& cell = AlgorithmInstance->Seq.Cells[SelectedCell.x][SelectedCell.y];
		ParamEditRaw = cd.Default + CalculateEpsilon(cd);
		cell.SetField(*AlgorithmInstance, SelectedParameterIndex, ParamEditRaw);
		// default state for direction should give us an initial direction (east)
		if (SelectedParameterIndex == CellDataType::Direction) {
			if (AlgorithmInstance->Seq.InitialStep == SelectedCell) {
				cell.SetField(*AlgorithmInstance, SelectedParameterIndex, 3);
			}
		}
		AlgorithmInstance->HelpText.DisplayHelpText(cd.HelpText);
		LoadParamForEditing();
	}
}


void GridMode::Pot3LongPress() {
	if (Editable) {
		const auto& cd = CellDefinitions[static_cast<size_t>(SelectedParameterIndex)];
		ParamEditRaw = cd.Default + CalculateEpsilon(cd);
		for (int x = 0; x < GridSizeX; x++) {
			for (int y = 0; y < GridSizeY; y++) {
				auto& cell = AlgorithmInstance->Seq.Cells[x][y];
				cell.SetField(*AlgorithmInstance, SelectedParameterIndex, ParamEditRaw);
			}
		}
		// default state for direction should give us an initial direction (east)
		if (SelectedParameterIndex == CellDataType::Direction) {
			auto& cell = AlgorithmInstance->Seq.Cells[AlgorithmInstance->Seq.InitialStep.x][AlgorithmInstance->Seq.InitialStep.y];
			cell.SetField(*AlgorithmInstance, SelectedParameterIndex, 3);
		}
		AlgorithmInstance->HelpText.DisplayHelpText(cd.HelpText);
		LoadParamForEditing();
	}
}


void GridMode::FixupPotValues(_NT_float3& pots) {
	// calculate an epsilon that we can add to our value to put it exactly in between pot "ticks"
	// this way we aren't right on the edge, where a slight pot bump could change the value
	auto epsilon2 = 0.5 / ARRAY_SIZE(CellDefinitions);
	pots[1] = static_cast<float>(SelectedParameterIndex) / ARRAY_SIZE(CellDefinitions) + epsilon2;

	const auto& cd = CellDefinitions[static_cast<size_t>(SelectedParameterIndex)];
	const auto& cell = AlgorithmInstance->Seq.Cells[SelectedCell.x][SelectedCell.y];
	auto range = cd.Max - cd.Min;
	pots[2] = cell.GetField(*AlgorithmInstance, SelectedParameterIndex) / range;
}


void GridMode::Activate() {
	LoadParamForEditing();
}