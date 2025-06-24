#include "common.h"
#include "cellData.h"
#include "directionalSequencer.h"

float CellData::GetField(const DirectionalSequencer& alg, CellDataType ct) const {
	const auto& cd = CellDefinitions[static_cast<size_t>(ct)];
	int result;
	switch (ct)
	{
		case CellDataType::Direction:   result = Direction;        break;
		case CellDataType::Value:       result = StepValue;        break;
		case CellDataType::Velocity:    result = Velocity;         break;
		case CellDataType::Probability: result = Probability;      break;
		case CellDataType::Ratchets:    result = RatchetCount;     break;
		case CellDataType::RestAfter:   result = RestAfter;        break;
		case CellDataType::GateLength:  result = GateLength;       break;
		case CellDataType::DriftProb:   result = DriftProbability; break;
		case CellDataType::MaxDrift:    result = MaxDriftAmount;   break;
		case CellDataType::Repeats:     result = RepeatCount;      break;
		case CellDataType::Glide:       result = GlidePercent;     break;
		case CellDataType::AccumAdd:    result = AccumulatorAdd;   break;
		case CellDataType::AccumTimes:  result = AccumulatorTimes; break;
		default: result = 0; break;
	}
	float fresult = static_cast<float>(result) / static_cast<int16_t>(pow(10, cd.Precision));
	return fresult;
}


void CellData::SetField(const DirectionalSequencer& alg, CellDataType ct, float val) {
	const auto& cd = CellDefinitions[static_cast<size_t>(ct)];
	val = clamp(val, cd.Min, cd.Max);
	int16_t ival = val * static_cast<int16_t>(pow(10, cd.Precision));
	switch (ct)
	{
		case CellDataType::Direction:   Direction = ival;        break;
		case CellDataType::Value:       StepValue = ival;        break;
		case CellDataType::Velocity:    Velocity = ival;         break;
		case CellDataType::Probability: Probability = ival;      break;
		case CellDataType::Ratchets:    RatchetCount = ival;     break;
		case CellDataType::RestAfter:   RestAfter = ival;        break;
		case CellDataType::GateLength:  GateLength = ival;       break;
		case CellDataType::DriftProb:   DriftProbability = ival; break;
		case CellDataType::MaxDrift:    MaxDriftAmount = ival;   break;
		case CellDataType::Repeats:     RepeatCount = ival;      break;
		case CellDataType::Glide:       GlidePercent = ival;     break;
		case CellDataType::AccumAdd:    AccumulatorAdd = ival;   break;
		case CellDataType::AccumTimes:  AccumulatorTimes = ival; break;
		default: break;  // do nothing
	}
}
