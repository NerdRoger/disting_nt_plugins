#include <stdio.h>
#include <distingnt/api.h>
#include <distingnt/slot.h>
#include "common.h"
#include "stepDataRegion.h"
#include "cellDefinition.h"


StepDataRegion::StepDataRegion() {
	
}


void StepDataRegion::InjectDependencies(_NT_algorithm* alg, const CellDefinition* cellDefs, RandomGenerator* random, void (*onDataChanged)(_NT_algorithm* alg)) {
	Algorithm = alg;
	CellDefs = cellDefs;
	Random = random;
	OnDataChangedCallback = onDataChanged;
}


void StepDataRegion::DoDataChanged() {
	if (OnDataChangedCallback) {
		OnDataChangedCallback(Algorithm);
	}
}


DirSeqModMatrixAlg* StepDataRegion::GetModMatrixAlgorithm(CellDataType ct, int& paramTargetIndex) const {
	paramTargetIndex = -1;
	auto algIndex = NT_algorithmIndex(Algorithm);
	_NT_slot slot;
	for (uint32_t idx = algIndex + 1; idx < NT_algorithmCount(); idx++) {
		if (!NT_getSlot(slot, idx))
			return nullptr;
		if (slot.guid() == DirSeqModMatrixAlg::Factory.guid) {
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


void StepDataRegion::SetDefaultCellValues() {
	// set the default cell values
	for (size_t i = 0; i < static_cast<size_t>(CellDataType::NumCellDataTypes); i++) {
		auto& cd = CellDefs[i];
		for (int x = 0; x < GridSizeX; x++) {
			for (int y = 0; y < GridSizeY; y++) {
				SetBaseCellValue(x, y, static_cast<CellDataType>(i), cd.Default, true);
			}
		}
	}
	// default state for direction should give us an initial direction (east)
	SetBaseCellValue(0, 0, CellDataType::Direction, 3, true);
}


bool StepDataRegion::CellTypeHasMapping(CellDataType ct) const {
	int paramTargetIndex;
	auto matrix = GetModMatrixAlgorithm(ct, paramTargetIndex);
	return (matrix != nullptr);
}


float StepDataRegion::GetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct) const {
	const auto& cd = CellDefs[static_cast<size_t>(ct)];
	int16_t multiplier = pow(10, cd.Scaling);

	// our internal cell data should always reflect the base value, because we keep it in sync even if it's updated from a mod matrix parameter
	int result;
	auto& cell = Cells[x][y];
	switch (ct)
	{
		case CellDataType::Direction:   result = cell.Direction;        break;
		case CellDataType::Value:       result = cell.StepValue;        break;
		case CellDataType::Velocity:    result = cell.Velocity;         break;
		case CellDataType::Probability: result = cell.Probability;      break;
		case CellDataType::Ratchets:    result = cell.RatchetCount;     break;
		case CellDataType::RestAfter:   result = cell.RestAfter;        break;
		case CellDataType::GateLength:  result = cell.GateLength;       break;
		case CellDataType::DriftProb:   result = cell.DriftProbability; break;
		case CellDataType::MaxDrift:    result = cell.MaxDriftAmount;   break;
		case CellDataType::Repeats:     result = cell.RepeatCount;      break;
		case CellDataType::Glide:       result = cell.GlidePercent;     break;
		case CellDataType::AccumAdd:    result = cell.AccumulatorAdd;   break;
		case CellDataType::AccumTimes:  result = cell.AccumulatorTimes; break;
		default: result = 0; break;
	}
	float fresult = static_cast<float>(result) / multiplier;
	return fresult;
}


float StepDataRegion::GetAdjustedCellValue(uint8_t x, uint8_t y, CellDataType ct) const {
	const auto& cd = CellDefs[static_cast<size_t>(ct)];
	int16_t multiplier = pow(10, cd.Scaling);

	// if we have a mod matrix assigned to this cell type, get the value from there...
	int paramTargetIndex;
	auto matrix = GetModMatrixAlgorithm(ct, paramTargetIndex);
	if (matrix != nullptr) {
		auto matrixIndex = NT_algorithmIndex(matrix);
		_NT_slot slot;
		NT_getSlot(slot, matrixIndex);
		auto cellIndex = y * GridSizeX + x;
		auto idx = paramTargetIndex + 1 + cellIndex + NT_parameterOffset();
		auto val = static_cast<float>(slot.parameterValue(idx)) / multiplier;
		return val;
	}

	// otherwise take it from our internal cell data
	return GetBaseCellValue(x, y, ct);
}


void StepDataRegion::SetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct, float val, bool updateMatrix) {
	const auto& cd = CellDefs[static_cast<size_t>(ct)];
	auto& cell = Cells[x][y];
	val = clamp(val, cd.Min, cd.Max);
	int16_t ival = val * static_cast<int16_t>(pow(10, cd.Scaling));

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
			NT_setParameterFromAudio(matrixIndex, idx, ival);
		}
	}
}


void StepDataRegion::ScrambleAllCellValues(CellDataType ct) {
	const int totalCells = GridSizeX * GridSizeY;
	for (int i = totalCells - 1; i > 0; --i) {
		int j = Random->Next(0, i);
		
		if (i == j) {
			continue;
		}

		uint8_t x1 = i % GridSizeX;
		uint8_t y1 = i / GridSizeX;
		uint8_t x2 = j % GridSizeX;
		uint8_t y2 = j / GridSizeX;

		float valI = GetBaseCellValue(x1, y1, ct);
		float valJ = GetBaseCellValue(x2, y2, ct);
		SetBaseCellValue(x1, y1, ct, valJ, true);
		SetBaseCellValue(x2, y2, ct, valI, true);
	}
	DoDataChanged();
}


void StepDataRegion::InvertCellValue(uint8_t x, uint8_t y, CellDataType ct) {
	const auto& cd = CellDefs[static_cast<size_t>(ct)];
	float val = GetBaseCellValue(x, y, ct);

	// direction is a special case, we want to invert the cardinal direction, not the index number representing it
	if (ct == CellDataType::Direction) {
		auto ival = static_cast<uint8_t>(val);
		if (ival > 0) {
			val = wrap(ival + 4, 1, 8);
		}
	} else {
		auto delta = val - cd.Min;
		val = cd.Max - delta;
	}

	SetBaseCellValue(x, y, ct, val, true);
	DoDataChanged();
}


void StepDataRegion::InvertAllCellValues(CellDataType ct) {
	for (int x = 0; x < GridSizeX; x++) {
		for (int y = 0; y < GridSizeY; y++) {
			InvertCellValue(x, y, ct);
		}
	}
}


void StepDataRegion::SwapWithSurroundingCellValue(uint8_t x, uint8_t y, CellDataType ct) {
	int8_t xOff = Random->Next(0, 2) - 1;
	int8_t yOff = Random->Next(0, 2) - 1;
	uint8_t x2 = wrap(x + xOff, 0, GridSizeX - 1);
	uint8_t y2 = wrap(y + yOff, 0, GridSizeY - 1);
	float v = GetBaseCellValue(x, y, ct);
	float v2 = GetBaseCellValue(x2, y2, ct);
	SetBaseCellValue(x, y, ct, v2, true);
	SetBaseCellValue(x2, y2, ct, v, true);
	DoDataChanged();
}


void StepDataRegion::RandomizeCellValue(uint8_t x, uint8_t y, CellDataType ct, float min, float max) {
	const auto& cd = CellDefs[static_cast<size_t>(ct)];
	auto scale = pow(10, cd.Scaling);
	auto rval = Random->Next(min * scale, max * scale);
	auto val = rval / scale;
	SetBaseCellValue(x, y, ct, val, true);
	DoDataChanged();
}


void StepDataRegion::RandomizeAllCellValues(CellDataType ct, float min, float max) {
	for (int x = 0; x < GridSizeX; x++) {
		for (int y = 0; y < GridSizeY; y++) {
			RandomizeCellValue(x, y, ct, min, max);
		}
	}
}


void StepDataRegion::RotateCellValuesInRow(uint8_t row, CellDataType ct, int8_t rotateBy) {
	// buffer to store rotated values
	float buf[GridSizeX];
	for (int x = 0; x < GridSizeX; x++) {
		float val = GetBaseCellValue(x, row, ct);
		auto newX = wrap(x + rotateBy, 0, static_cast<int>(GridSizeX) - 1);
		buf[newX] = val;
	}
	// move values from rotated buffer into cells
	for (int x = 0; x < GridSizeX; x++) {
		SetBaseCellValue(x, row, ct, buf[x], true);
	}
	DoDataChanged();
}


void StepDataRegion::RotateCellValuesInColumn(uint8_t col, CellDataType ct, int8_t rotateBy) {
	// buffer to store rotated values
	float buf[GridSizeY];
	for (int y = 0; y < GridSizeY; y++) {
		float val = GetBaseCellValue(col, y, ct);
		auto newY = wrap(y + rotateBy, 0, static_cast<int>(GridSizeY) - 1);
		buf[newY] = val;
	}
	// move values from rotated buffer into cells
	for (int y = 0; y < GridSizeY; y++) {
		SetBaseCellValue(col, y, ct, buf[y], true);
	}
	DoDataChanged();
}


void StepDataRegion::RandomlyChangeCellValue(uint8_t x, uint8_t y, CellDataType ct, uint8_t deltaPercent) {
	const auto& cd = CellDefs[static_cast<size_t>(ct)];
	auto rnd = static_cast<float>(Random->Next(0, deltaPercent * 100.0f) / 100.0f) * (Random->Next(0, 1) == 1 ? -1.0f : 1.0f);
	float delta = (cd.Max - cd.Min) * rnd / 100.0f;
	float oldVal = GetBaseCellValue(x, y, ct);
	float newVal = oldVal + delta;
	SetBaseCellValue(x, y, ct, newVal, true);
	DoDataChanged();
}


void StepDataRegion::RandomlyChangeAllCellValues(CellDataType ct, uint8_t deltaPercent) {
	for (int x = 0; x < GridSizeX; x++) {
		for (int y = 0; y < GridSizeY; y++) {
			RandomlyChangeCellValue(x, y, ct, deltaPercent);
		}
	}
}