#pragma once
#include <new>
#include <distingnt/api.h>
#include <distingnt/serialisation.h>
#include "potManager.h"
#include "helpTextHelper.h"
#include "cellDefinition.h"
#include "stepDataRegion.h"
#include "playhead.h"
#include "gridView.h"
#include "timeKeeper.h"

enum {
	// no common parameters as of now, but still define it this way so calcs work out if some are added later
	kNumCommonParameters,
};


enum {	
	kParamClock,
	kParamClockDivisor,
	kParamClockOffset,
	kParamReset,
	kParamValue,
	kParamGate,
	kParamVelocityGateMin,
	kParamVelocity,
	kParamQuantSend,
	kParamQuantReturn,

	kParamGateLengthSource,
	kParamMaxGateLength,
	kParamGateLengthAttenuate,
	kParamHumanizeValue,
	kParamAttenValue,
	kParamOffsetValue,
	kParamVelocityAttenuate,
	kParamVelocityOffset,
	kParamMoveNCells,
	kParamRestAfterNSteps,
	kParamSkipAfterNSteps,
	kParamResetAfterNSteps,
	kParamResetWhenInactive,
	
	kNumPerPlayheadParameters,

	kNumIOPlayheadParameters = kParamQuantReturn + 1,
	kNumGeneralPlayheadParameters = kParamResetWhenInactive - kNumIOPlayheadParameters + 1
};


enum class GateLengthSource {
	MaxGateLength,
	Clock
};


struct DirSeqAlg : public _NT_algorithm{
private:
	// NT Parameter Data
	_NT_parameter* ParameterDefs;
	_NT_parameterPages PagesDefs;
	_NT_parameterPage* PageDefs;
	uint8_t* PageParams;
	void BuildParameters();

	void InjectDependencies(const _NT_globals* globals);

	// NT factory "methods"
	static void CalculateRequirements(_NT_algorithmRequirements& req, const int32_t* specifications);
	static _NT_algorithm* Construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t* specifications);
	static void ParameterChanged(_NT_algorithm* self, int p);
	static void Step(_NT_algorithm* self, float* busFrames, int numFramesBy4);
	static bool Draw(_NT_algorithm* self);
	static uint32_t HasCustomUI(_NT_algorithm* self);
	static void SetupUI(_NT_algorithm* self, _NT_float3& pots);
	static void CustomUI(_NT_algorithm* self, const _NT_uiData& data);
	static void Serialise(_NT_algorithm* self, _NT_jsonStream& stream);
	static bool DeserialiseCellCoords(_NT_algorithm* self, _NT_jsonParse& parse, CellCoords& coords);
	static bool DeserialiseGridCellData(_NT_algorithm* self, _NT_jsonParse& parse);
	static bool Deserialise(_NT_algorithm* self, _NT_jsonParse& parse);
	static int ParameterUiPrefix(_NT_algorithm* self, int p, char* buff);

public:
	static constexpr uint32_t Guid = NT_MULTICHAR( 'A', 'T', 'd', 's' );

	bool Loaded;

	HIDDEN static const _NT_factory Factory;

	// TODO:  Maybe find a better naming scheme for types/members
	// order is important here, as some classes depend on others being constructed first
	TimeKeeper Timer;
	RandomGenerator Random;
	PotManager PotMgr;
	HelpTextHelper HelpText;
	StepDataRegion StepData;
	GridView Grid;

	PlayheadList Playheads;

	DirSeqAlg();
	~DirSeqAlg();
	void StepDataChangedHandler();
};
