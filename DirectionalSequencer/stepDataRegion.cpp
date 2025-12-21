#include <stdio.h>
#include <distingnt/api.h>
#include <distingnt/slot.h>
#include "common.h"
#include "stepDataRegion.h"
#include "cellDefinition.h"


StepDataRegion::StepDataRegion(_NT_algorithm* alg, const CellDefinition* cellDefs) {
	Algorithm = alg;
	CellDefs = cellDefs;
	SetDefaultCellValues();
}


DirSeqModMatrixAlg* StepDataRegion::GetModMatrixAlgorithm(CellDataType ct, int& paramTargetIndex) const {
	paramTargetIndex = -1;
	auto algIndex = NT_algorithmIndex(Algorithm);
	_NT_slot slot;
	for (uint32_t idx = algIndex + 1; idx < NT_algorithmCount(); idx++) {
		if (!NT_getSlot(slot, idx))
			return nullptr;
		if (slot.guid() == NT_MULTICHAR( 'A', 'T', 'd', 'm' )) {
			auto matrix = static_cast<DirSeqModMatrixAlg*>(slot.plugin());
			if (matrix->v[kParamModATarget] - 1 == static_cast<int>(ct)) {
				paramTargetIndex = kParamModATarget;
				return matrix;
			} else if (matrix->v[kParamModBTarget] - 1 == static_cast<int>(ct)) {
				paramTargetIndex = kParamModBTarget;
				return matrix;
			} else if (matrix->v[kParamModCTarget] - 1 == static_cast<int>(ct)) {
				paramTargetIndex = kParamModCTarget;
				return matrix;
			} else if (matrix->v[kParamModDTarget] - 1 == static_cast<int>(ct)) {
				paramTargetIndex = kParamModDTarget;
				return matrix;
			} else if (matrix->v[kParamModETarget] - 1 == static_cast<int>(ct)) {
				paramTargetIndex = kParamModETarget;
				return matrix;
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


float StepDataRegion::GetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct) const {
	const auto& cd = CellDefs[static_cast<size_t>(ct)];
	int16_t multiplier = pow(10, cd.Precision);

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
	int16_t multiplier = pow(10, cd.Precision);

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
	int16_t ival = val * static_cast<int16_t>(pow(10, cd.Precision));

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
