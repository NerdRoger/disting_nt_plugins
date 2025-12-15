#include <stdio.h>
#include <distingnt/api.h>
#include <distingnt/slot.h>
#include "directionalSequencer.h"
#include "cellDefinition.h"


void StepDataRegion::SetDefaultCellValues() {
	// set the default cell values
	for (size_t i = 0; i < ARRAY_SIZE(CellDefinitions); i++) {
		auto& cd = CellDefinitions[i];
		for (int x = 0; x < GridSizeX; x++) {
			for (int y = 0; y < GridSizeY; y++) {
				SetBaseCellValue(x, y, static_cast<CellDataType>(i), cd.Default);
			}
		}
	}
	// default state for direction should give us an initial direction (east)
	SetBaseCellValue(0, 0, CellDataType::Direction, 3);
}


float StepDataRegion::GetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct, bool readFromParam) const {
	// TODO:  make these methods better and not just copy/paste
	const auto& cd = CellDefinitions[static_cast<size_t>(ct)];

	if (readFromParam) {
		int16_t modATarget = AlgorithmInstance->v[kParamModATarget];
		// if this cell type is wired up to an NT parameter, get info from there
		if (ct == static_cast<CellDataType>(modATarget)) {
			auto algIndex = NT_algorithmIndex(AlgorithmInstance);
			_NT_slot slot;
			NT_getSlot(slot, algIndex);
			auto cellIndex = y * GridSizeX + x;
			auto val = static_cast<float>(slot.parameterPresetValue(kParamModATargetCell1 + cellIndex + 1)) / static_cast<int16_t>(pow(10, cd.Precision));
			return val;
		}
	}

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
	float fresult = static_cast<float>(result) / static_cast<int16_t>(pow(10, cd.Precision));
	return fresult;
}


float StepDataRegion::GetAdjustedCellValue(uint8_t x, uint8_t y, CellDataType ct) const {
	const auto& cd = CellDefinitions[static_cast<size_t>(ct)];
	auto& cell = Cells[x][y];
	int result;

	int16_t modATarget = AlgorithmInstance->v[kParamModATarget];
	// if this cell type is wired up to an NT parameter, get info from there
	if (ct == static_cast<CellDataType>(modATarget)) {
		auto algIndex = NT_algorithmIndex(AlgorithmInstance);
		_NT_slot slot;
		NT_getSlot(slot, algIndex);
		auto cellIndex = y * GridSizeX + x;
		auto val = static_cast<float>(slot.parameterValue(kParamModATargetCell1 + cellIndex + 1)) / static_cast<int16_t>(pow(10, cd.Precision));
		return val;
	}

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
	float fresult = static_cast<float>(result) / static_cast<int16_t>(pow(10, cd.Precision));
	return fresult;
}


void StepDataRegion::SetBaseCellValue(uint8_t x, uint8_t y, CellDataType ct, float val, bool updateParam) {
	const auto& cd = CellDefinitions[static_cast<size_t>(ct)];
	auto& cell = Cells[x][y];
	val = clamp(val, cd.Min, cd.Max);
	int16_t ival = val * static_cast<int16_t>(pow(10, cd.Precision));
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

	// if we have an NT parameter mapped to the value we are changing, also change the parameter value
	if (updateParam) {
		int16_t modATarget = AlgorithmInstance->v[kParamModATarget];
		if (ct == static_cast<CellDataType>(modATarget)) {
			auto algIndex = NT_algorithmIndex(AlgorithmInstance);
			auto cellIndex = y * GridSizeX + x;
			NT_setParameterFromAudio(algIndex, kParamModATargetCell1 + cellIndex + NT_parameterOffset(), ival);
//			NT_setParameterFromUi(algIndex, kParamModATargetCell1 + cellIndex + NT_parameterOffset(), ival);
		}
	}
}
