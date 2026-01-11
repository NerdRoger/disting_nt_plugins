#pragma once
#include <distingnt/api.h>
#include <stddef.h>
#include "common.h"
#include "viewBase.h"


struct WeightedQuantizerAlg;


// TODO:  derive from ViewBase also
struct QuantizerView : ViewBase {
private:
	WeightedQuantizerAlg* Algorithm = nullptr;

	bool BankPeeking = false;

	size_t SelectedKeyIndex = 0;
	float SelectedKeyValueRaw = 0.0f;

	size_t SelectedBankIndex = 0;

	void DrawBank(size_t bankNum, const char* label) const;
	void DrawBanks() const;	
	void DrawKey(uint8_t x, uint8_t y, uint16_t paramIndex) const;
	void DrawHelpSection() const;
	void DrawKeyboard() const;
	void DrawPeek() const;
	void DrawArrow(uint8_t x, uint8_t y, uint8_t color) const;
	void DrawResult(uint8_t x, uint8_t y, const char* label, size_t channelIndex, uint8_t color) const;
	void DrawResults() const;

	void LoadKeyControlForEditing();

	HIDDEN static void OnActivateHandler(ViewBase* view);
	HIDDEN static void OnDrawHandler(ViewBase* view);
	HIDDEN static void OnEncoder1TurnHandler(ViewBase* view, int8_t x);
	HIDDEN static void OnEncoder2TurnHandler(ViewBase* view, int8_t x);
	HIDDEN static void OnEncoder2ShortPressHandler(ViewBase* view);
	HIDDEN static void OnEncoder2LongPressHandler(ViewBase* view);
	HIDDEN static void OnPot1TurnHandler(ViewBase* view, float val);
	HIDDEN static void OnPot3TurnHandler(ViewBase* view, float val);
	HIDDEN static void OnPot3ShortPressHandler(ViewBase* view);
	HIDDEN static void OnButton3PushHandler(ViewBase* view);
	HIDDEN static void OnButton3ReleaseHandler(ViewBase* view);
	HIDDEN static void OnFixupPotValuesHandler(ViewBase* view, _NT_float3& pots);
	HIDDEN static void OnParameterChangedHandler(ViewBase* view, int paramIndex);


public:
	QuantizerView();
	void InjectDependencies(WeightedQuantizerAlg* alg);

};