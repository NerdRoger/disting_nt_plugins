#pragma once

#include "modeBase.h"

struct SettingsMode : ModeBase {
private:

	struct Control {
		int16_t     ParameterIndex;
		const char* HelpText;
	};

	static const Control Controls[];
	static const Control GateControls[];

	size_t SelectedControlIndex = 0;
	float SelectedControlIndexRaw = 0.0f;
	float SelectedControlValueRaw = 0.0f;

	const Control& GetControlByOrdinalIndex(size_t idx) const;
	const Control& FindControlByParameterIndex(uint8_t idx) const;
	void DrawParameter(uint8_t labelX, uint8_t editBoxX, uint8_t editBoxWidth, uint8_t y, const char* label, int paramIdx, uint8_t decimalPlaces, const char* suffix) const;
	void DrawParameters() const;
	void DrawHelpSection() const;
	
public:
	void DrawIcon(int x, int y, int color) const override;
	void Draw() const override;

	void Encoder2ShortPress() override;
	void Pot2Turn(float val) override;
	void Pot3Turn(float val) override;
	void Pot3ShortPress() override;
	void FixupPotValues(_NT_float3& pots) override;
//	void ParameterChanged(int paramIndex) override;

	void LoadControlForEditing();
	void Activate() override;
	void FixupParameters();
};
