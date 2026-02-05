#include <stdio.h>
#include <distingnt/api.h>
#include <distingnt/slot.h>
#include "common.h"
#include "stepDataRegion.h"
#include "cellDefinition.h"
#include "dirSeqAlg.h"


StepDataRegion::StepDataRegion() {
	
}


void StepDataRegion::InjectDependencies(DirSeqAlg* alg) {
	Algorithm = alg;
}


void StepDataRegion::DoDataChanged() {
	Algorithm->StepDataChangedHandler();
}


DirSeqModMatrixAlg* StepDataRegion::GetModMatrixAlgorithm(CellDataType ct, int& paramTargetIndex) const {
	paramTargetIndex = -1;
	auto algIndex = NT_algorithmIndex(Algorithm);
	_NT_slot slot;
	for (uint32_t idx = algIndex + 1; idx < NT_algorithmCount(); idx++) {
		if (!NT_getSlot(slot, idx))
			return nullptr;
		if (slot.guid() == DirSeqModMatrixAlg::Guid) {
			auto matrix = static_cast<DirSeqModMatrixAlg*>(slot.plugin());

			for (int m = 0; m < DirSeqModMatrixAlg::NumMatrices; m++) {
				if (matrix->v[m * kParamModTargetStride + kParamModTarget] - 1 == static_cast<int>(ct)) {
					paramTargetIndex = m * kParamModTargetStride + kParamModTarget;
					return matrix;
				}
			}
		} else {
			// the first non-matrix algorithm we encounter, bail out, as we need them to be contiguous
			return nullptr;
		}
	}
	return nullptr;
}


void StepDataRegion::SetDefaultCellValues(CallingContext ctx) {
	// set the default cell values
	for (size_t i = 0; i < static_cast<size_t>(CellDataType::NumCellDataTypes); i++) {
		auto cd = CellDefinition::All[i];
		for (int x = 0; x < GridSizeX; x++) {
			for (int y = 0; y < GridSizeY; y++) {
				SetBaseCellValue(x, y, static_cast<CellDataType>(i), cd.ScaledDefault(), true, ctx);
			}
		}
	}
	// default state for direction should give us an initial direction (east)
	SetBaseCellValue(0, 0, CellDataType::Direction, 3, true, ctx);
}


bool StepDataRegion::CellTypeHasMapping(CellDataType ct) const {
	int paramTargetIndex;
	auto matrix = GetModMatrixAlgorithm(ct, paramTargetIndex);
	return (matrix != nullptr);
}


float StepDataRegion::GetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct) const {
	auto cd = CellDefinition::All[static_cast<size_t>(ct)];

	// our internal cell data should always reflect the base value, because we keep it in sync even if it's updated from a mod matrix parameter
	auto& cell = Cells[x][y];
	switch (ct)
	{
		case CellDataType::Direction:   return cd.CellStorageToCellValue(cell.Direction);
		case CellDataType::Value:       return cd.CellStorageToCellValue(cell.StepValue);
		case CellDataType::Velocity:    return cd.CellStorageToCellValue(cell.Velocity);
		case CellDataType::Probability: return cd.CellStorageToCellValue(cell.Probability);
		case CellDataType::Ratchets:    return cd.CellStorageToCellValue(cell.RatchetCount);
		case CellDataType::RestAfter:   return cd.CellStorageToCellValue(cell.RestAfter);
		case CellDataType::GateLength:  return cd.CellStorageToCellValue(cell.GateLength);
		case CellDataType::DriftProb:   return cd.CellStorageToCellValue(cell.DriftProbability);
		case CellDataType::MaxDrift:    return cd.CellStorageToCellValue(cell.MaxDriftAmount);
		case CellDataType::Repeats:     return cd.CellStorageToCellValue(cell.RepeatCount);
		case CellDataType::Glide:       return cd.CellStorageToCellValue(cell.GlidePercent);
		case CellDataType::AccumAdd:    return cd.CellStorageToCellValue(cell.AccumulatorAdd);
		case CellDataType::AccumTimes:  return cd.CellStorageToCellValue(cell.AccumulatorTimes);
		case CellDataType::TieSteps:    return cd.CellStorageToCellValue(cell.TieStepCount);
		case CellDataType::Mute:        return cd.CellStorageToCellValue(cell.Mute);
		default: return cd.CellStorageToCellValue(0);
	}
}


float StepDataRegion::GetAdjustedCellValue(uint8_t x, uint8_t y, CellDataType ct) const {
	auto cd = CellDefinition::All[static_cast<size_t>(ct)];

	// if we have a mod matrix assigned to this cell type, get the value from there...
	int paramTargetIndex;
	auto matrix = GetModMatrixAlgorithm(ct, paramTargetIndex);
	if (matrix != nullptr) {
		auto matrixIndex = NT_algorithmIndex(matrix);
		_NT_slot slot;
		NT_getSlot(slot, matrixIndex);
		auto cellIndex = y * GridSizeX + x;
		auto idx = paramTargetIndex + 1 + cellIndex + NT_parameterOffset();
		return cd.NTValueToCellValue(slot.parameterValue(idx));
	}

	// otherwise take it from our internal cell data
	return GetBaseCellValue(x, y, ct);
}


void StepDataRegion::SetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct, float val, bool updateMatrix, CallingContext ctx) {
	auto cd = CellDefinition::All[static_cast<size_t>(ct)];
	auto& cell = Cells[x][y];
	val = clamp(val, cd.ScaledMin(), cd.ScaledMax());
	int16_t ival = cd.CellValueToCellStorage(val);

	// always update our internal cell data...
	switch (ct)
	{
		case CellDataType::Direction:   cell.Direction = ival;        break;
		case CellDataType::Value:       cell.StepValue = ival;        break;
		case CellDataType::Velocity:    cell.Velocity = ival;         break;
		case CellDataType::Probability: cell.Probability = ival;      break;
		case CellDataType::Ratchets:    cell.RatchetCount = ival;     break;
		case CellDataType::RestAfter:   cell.RestAfter = ival;        break;
		case CellDataType::GateLength:  cell.GateLength = ival;       break;
		case CellDataType::DriftProb:   cell.DriftProbability = ival; break;
		case CellDataType::MaxDrift:    cell.MaxDriftAmount = ival;   break;
		case CellDataType::Repeats:     cell.RepeatCount = ival;      break;
		case CellDataType::Glide:       cell.GlidePercent = ival;     break;
		case CellDataType::AccumAdd:    cell.AccumulatorAdd = ival;   break;
		case CellDataType::AccumTimes:  cell.AccumulatorTimes = ival; break;
		case CellDataType::TieSteps:    cell.TieStepCount = ival;     break;
		case CellDataType::Mute:        cell.Mute = ival;             break;
		default: break;  // do nothing
	}

	// and if we have an NT parameter mapped to the value we are changing in a mod matrix sidecar, also change it's parameter value
	if (updateMatrix)	{
		int paramTargetIndex;
		auto matrix = GetModMatrixAlgorithm(ct, paramTargetIndex);
		if (matrix != nullptr) {
			auto matrixIndex = NT_algorithmIndex(matrix);
			auto cellIndex = y * GridSizeX + x;
			auto idx = paramTargetIndex + 1 + cellIndex + NT_parameterOffset();
			SetParameterValue(matrixIndex, idx, ival, ctx);
		}
	}
}


void StepDataRegion::ScrambleAllCellValues(CellDataType ct, CallingContext ctx) {
	const int totalCells = GridSizeX * GridSizeY;
	for (int i = totalCells - 1; i > 0; --i) {
		int j = Algorithm->Random.Next(0, i);
		
		if (i == j) {
			continue;
		}

		uint8_t x1 = i % GridSizeX;
		uint8_t y1 = i / GridSizeX;
		uint8_t x2 = j % GridSizeX;
		uint8_t y2 = j / GridSizeX;

		float valI = GetBaseCellValue(x1, y1, ct);
		float valJ = GetBaseCellValue(x2, y2, ct);
		SetBaseCellValue(x1, y1, ct, valJ, true, ctx);
		SetBaseCellValue(x2, y2, ct, valI, true, ctx);
	}
	DoDataChanged();
}


void StepDataRegion::InvertCellValue(uint8_t x, uint8_t y, CellDataType ct, CallingContext ctx) {
	auto cd = CellDefinition::All[static_cast<size_t>(ct)];
	float val = GetBaseCellValue(x, y, ct);

	// direction is a special case, we want to invert the cardinal direction, not the index number representing it
	if (ct == CellDataType::Direction) {
		auto ival = static_cast<uint8_t>(val);
		if (ival > 0) {
			val = wrap(ival + 4, 1, 8);
		}
	} else {
		auto delta = val - cd.ScaledMin();
		val = cd.ScaledMax() - delta;
	}

	SetBaseCellValue(x, y, ct, val, true, ctx);
	DoDataChanged();
}


void StepDataRegion::InvertAllCellValues(CellDataType ct, CallingContext ctx) {
	for (int x = 0; x < GridSizeX; x++) {
		for (int y = 0; y < GridSizeY; y++) {
			InvertCellValue(x, y, ct, ctx);
		}
	}
}


void StepDataRegion::SwapWithSurroundingCellValue(uint8_t x, uint8_t y, CellDataType ct, CallingContext ctx) {
	int8_t xOff = Algorithm->Random.Next(0, 2) - 1;
	int8_t yOff = Algorithm->Random.Next(0, 2) - 1;
	uint8_t x2 = wrap(x + xOff, 0, GridSizeX - 1);
	uint8_t y2 = wrap(y + yOff, 0, GridSizeY - 1);
	float v = GetBaseCellValue(x, y, ct);
	float v2 = GetBaseCellValue(x2, y2, ct);
	SetBaseCellValue(x, y, ct, v2, true, ctx);
	SetBaseCellValue(x2, y2, ct, v, true, ctx);
	DoDataChanged();
}


void StepDataRegion::RandomizeCellValue(uint8_t x, uint8_t y, CellDataType ct, float min, float max, CallingContext ctx) {
	auto cd = CellDefinition::All[static_cast<size_t>(ct)];
	int scaledMin = cd.ScaleValue(min);
	int scaledMax = cd.ScaleValue(max);
	auto lo = 0;
	auto hi = scaledMax - scaledMin;
	auto scaledRnd = static_cast<int>(Algorithm->Random.Next(lo, hi)) + scaledMin;
	auto val = cd.UnscaleValue(scaledRnd);
	SetBaseCellValue(x, y, ct, val, true, ctx);
	DoDataChanged();
}


void StepDataRegion::RandomizeAllCellValues(CellDataType ct, float min, float max, CallingContext ctx) {
	for (int x = 0; x < GridSizeX; x++) {
		for (int y = 0; y < GridSizeY; y++) {
			RandomizeCellValue(x, y, ct, min, max, ctx);
		}
	}
}


void StepDataRegion::RotateCellValuesInRow(uint8_t row, CellDataType ct, int8_t rotateBy, CallingContext ctx) {
	// buffer to store rotated values
	float buf[GridSizeX];
	for (int x = 0; x < GridSizeX; x++) {
		float val = GetBaseCellValue(x, row, ct);
		auto newX = wrap(x + rotateBy, 0, static_cast<int>(GridSizeX) - 1);
		buf[newX] = val;
	}
	// move values from rotated buffer into cells
	for (int x = 0; x < GridSizeX; x++) {
		SetBaseCellValue(x, row, ct, buf[x], true, ctx);
	}
	DoDataChanged();
}


void StepDataRegion::RotateCellValuesInColumn(uint8_t col, CellDataType ct, int8_t rotateBy, CallingContext ctx) {
	// buffer to store rotated values
	float buf[GridSizeY];
	for (int y = 0; y < GridSizeY; y++) {
		float val = GetBaseCellValue(col, y, ct);
		auto newY = wrap(y + rotateBy, 0, static_cast<int>(GridSizeY) - 1);
		buf[newY] = val;
	}
	// move values from rotated buffer into cells
	for (int y = 0; y < GridSizeY; y++) {
		SetBaseCellValue(col, y, ct, buf[y], true, ctx);
	}
	DoDataChanged();
}


void StepDataRegion::RandomlyChangeCellValue(uint8_t x, uint8_t y, CellDataType ct, uint8_t deltaPercent, CallingContext ctx) {
	auto cd = CellDefinition::All[static_cast<size_t>(ct)];
	auto rnd = static_cast<float>(Algorithm->Random.Next(0, deltaPercent * 100.0f) / 100.0f) * (Algorithm->Random.Next(0, 1) == 1 ? -1.0f : 1.0f);
	float delta = (cd.ScaledMax() - cd.ScaledMin()) * rnd / 100.0f;
	float oldVal = GetBaseCellValue(x, y, ct);
	float newVal = oldVal + delta;
	SetBaseCellValue(x, y, ct, newVal, true, ctx);
	DoDataChanged();
}


void StepDataRegion::RandomlyChangeAllCellValues(CellDataType ct, uint8_t deltaPercent, CallingContext ctx) {
	for (int x = 0; x < GridSizeX; x++) {
		for (int y = 0; y < GridSizeY; y++) {
			RandomlyChangeCellValue(x, y, ct, deltaPercent, ctx);
		}
	}
}