#pragma once
#include <stdint.h>
#include <distingnt/api.h>
#include "ownedBase.h"


struct DirectionalSequencer;


struct ModeBase : OwnedBase<DirectionalSequencer> {
private:
	static constexpr int EditBoxUnselectedBackgroundColor = 1;
	static constexpr int EditBoxSelectedBackgroundColor = 1;
	static constexpr int EditBoxUnselectedTextColor = 8;
	static constexpr int EditBoxSelectedTextColor = 15;
	static constexpr int EditBoxSelectedBorderColor = 15;

protected:
	static constexpr int ModeAreaX = 50;
	mutable char NumToStrBuf[20]; // for storing conversion results

	bool Editable = true;

	void FixFloatBuf() const;
	void AddSuffixToBuf(const char* suffix) const;
	void DrawEditBox(uint8_t x, uint8_t y, uint8_t width, const char* text, bool selected, bool editable) const;

public:
	virtual void DrawIcon(int x, int y, int color) const { }
	virtual void Activate() { }
	virtual void Draw() const { }
	virtual void Encoder1Turn(int8_t x) { }
	virtual void Encoder2Turn(int8_t x) { }
	virtual void Encoder2ShortPress() { }
	virtual void Encoder2LongPress() { }
	virtual void Pot2Turn(float val) { }
	virtual void Pot3Turn(float val) { }
	virtual void Pot3ShortPress() { }
	virtual void Pot3LongPress() { }
	virtual void FixupPotValues(_NT_float3& pots) { }
	virtual void ParameterChanged(int paramIndex) { }
};
