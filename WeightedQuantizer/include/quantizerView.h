#pragma once
#include <distingnt/api.h>
#include <stddef.h>
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

public:
	QuantizerView();
	void InjectDependencies(WeightedQuantizerAlg* alg);
	void Draw() const override;
	void Encoder1Turn(int8_t x) override;
	void Encoder2Turn(int8_t x) override;
	void Encoder2ShortPress() override;
	void Encoder2LongPress() override;
	void Pot1Turn(float val) override;
	void Pot3Turn(float val) override;
	void Pot3ShortPress() override;
	void Button3Push() override;
	void Button3Release() override;
	void FixupPotValues(_NT_float3& pots) override;
	void Activate() override;
	void ParameterChanged(int paramIndex) override;
};