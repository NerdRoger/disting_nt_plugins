#pragma once
#include <distingnt/api.h>
#include <stddef.h>
#include "viewBase.h"
#include "helpTextHelper.h"
#include "potManager.h"
#include "noteBanks.h"
#include "timeKeeper.h"
#include "quantizer.h"


struct WeightedQuantizerAlg;


// TODO:  derive from ViewBase also
struct QuantizerView : ViewBase {
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
	
	_NT_algorithm* Algorithm = nullptr;
	Quantizer::QuantResult*& QuantResults;
	uint16_t NumChannels;
	HelpTextHelper* HelpText = nullptr;
	PotManager* PotMgr = nullptr;
	NoteBanks* Banks = nullptr;

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
	QuantizerView(_NT_algorithm* alg, TimeKeeper* timer, uint16_t numChannels, HelpTextHelper* helpText, PotManager* potMgr, NoteBanks* banks, Quantizer::QuantResult*& quantResults);
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