#pragma once
#include <stdint.h>
#include <distingnt/api.h>
#include "timeKeeper.h"


struct ViewBase {
private:
	static constexpr int EditBoxUnselectedBackgroundColor = 1;
	static constexpr int EditBoxSelectedBackgroundColor = 1;
	static constexpr int EditBoxUnselectedTextColor = 8;
	static constexpr int EditBoxSelectedTextColor = 15;
	static constexpr int EditBoxSelectedBorderColor = 15;

	static constexpr uint16_t ShortPressThreshold = 250; // How long (in ms) until a short press turns into a long press	
	
	TimeKeeper* Timer = nullptr;

	uint32_t Pot3DownTime = 0;
	uint32_t BlockPot3ChangesUntil = 0;
	uint32_t Encoder2DownTime = 0;
	bool Encoder2LongPressFired = false;
	bool Pot3LongPressFired = false;


protected:
	mutable char NumToStrBuf[20]; // for storing conversion results

	void DrawEditBox(uint8_t x, uint8_t y, uint8_t width, const char* text, bool selected, bool editable) const;
	void InjectDependencies(TimeKeeper* timer);

public:

	void ProcessControlInput(const _NT_uiData& data);
	void ProcessLongPresses();

	virtual void Activate() { }
	virtual void Draw() const { }
	virtual void Encoder1Turn(int8_t x) { }
	virtual void Encoder2Turn(int8_t x) { }
	virtual void Encoder2Push() { }
	virtual void Encoder2Release() { }
	virtual void Encoder2ShortPress() { }
	virtual void Encoder2LongPress() { }
	virtual void Pot1Turn(float val) { }
	virtual void Pot2Turn(float val) { }
	virtual void Pot3Turn(float val) { }
	virtual void Pot3Push() { }
	virtual void Pot3Release() { }
	virtual void Pot3ShortPress() { }
	virtual void Pot3LongPress() { }
	virtual void Button3Push() { }
	virtual void Button3Release() { }

	virtual void FixupPotValues(_NT_float3& pots) { }
	virtual void ParameterChanged(int paramIndex) { }
};
