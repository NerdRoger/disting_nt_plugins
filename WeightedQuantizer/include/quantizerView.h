#pragma once
#include <distingnt/api.h>
#include <stddef.h>
#include "ownedBase.h"


struct WeightedQuantizer;


struct QuantizerView : OwnedBase<WeightedQuantizer> {
private:

	struct Control {
		uint8_t     ParameterIndex;
		const char* HelpText;
	};

	static constexpr int KeyWidth = 13;
	static constexpr int KeyHeight = 15;
	static constexpr int KeySpacing = 3;

	static constexpr int KeyBackgroundColor = 1;
	static constexpr int KeyForegroundColor = 8;
	static constexpr int SelectedKeyBorderColor = 15;
	static constexpr float MaxSliderValue = 10.0f;
	
	bool BankPeeking = false;

	static const Control KeyControls[];
	size_t SelectedKeyIndex = 0;
	float SelectedKeyValueRaw = 0.0f;

	static const Control BankControls[];
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
	void Draw() const;

	void FixupPotValues(_NT_float3& pots);
	void ParameterChanged(int paramIndex);
	void Activate();

	void Encoder1Turn(int8_t x);
	void Encoder2Turn(int8_t x);
	void Pot1Turn(float val);
	void Pot3Turn(float val);
	void Pot3Push();
	void Button3Push();
	void Button3Release();


	void Encoder2ShortPress();
	void Encoder2LongPress();

};