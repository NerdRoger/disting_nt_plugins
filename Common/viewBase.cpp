#include <cstring>
#include <distingnt/api.h>
#include "viewBase.h"
#include "baseAlgorithm.h"



void ViewBase::FixFloatBuf() const {
	// find the null terminator
	uint32_t nt = strlen(NumToStrBuf);
	// walk backward from it, setting any '0' to null
	uint32_t end;
	for(end = nt - 1; end > 0; end--) {
		if (NumToStrBuf[end] == '0') {
			NumToStrBuf[end] = '\0';
		} else {
			break;
		}
	}
	// if we backed up all the way to the decimal point, get rid of that too
	if (NumToStrBuf[end] == '.')
		NumToStrBuf[end] = '\0';
}


void ViewBase::AddSuffixToBuf(const char* suffix) const {
	// find the null terminator
	uint32_t nt;
	for(nt = 0; nt < sizeof(NumToStrBuf); nt++) {
		if (NumToStrBuf[nt] == '\0')
			break;
	}
	// add the suffix
	for(uint32_t s = 0; s < strlen(suffix); s++) {
		NumToStrBuf[nt] = suffix[s];
		nt++;
	}
	// add our new null terminator
	NumToStrBuf[nt] = '\0';
}


void ViewBase::DrawEditBox(uint8_t x, uint8_t y, uint8_t width, const char* text, bool selected, bool editable) const {
	auto allowEdit = selected && editable;
	auto backgroundColor = allowEdit ? EditBoxSelectedBackgroundColor : EditBoxUnselectedBackgroundColor;
	NT_drawShapeI(kNT_rectangle, x - 1, y, x + width + 1, y + 8, backgroundColor);
	auto textColor = allowEdit ? EditBoxSelectedTextColor : EditBoxUnselectedTextColor;
	NT_drawText(x, y + 8, text, textColor);
	if (allowEdit) {
		NT_drawShapeI(kNT_box, x - 2, y - 1, x + width + 2, y + 9, EditBoxSelectedBorderColor);
	}
}


void ViewBase::ProcessControlInput(const BaseAlgorithm& alg, const _NT_uiData& data) {
	if (data.encoders[0]) {
		Encoder1Turn(data.encoders[0]);
	}

	if (data.encoders[1]) {
		Encoder2Turn(data.encoders[1]);
	}

	if (data.controls & kNT_potL) {
		Pot1Turn(data.pots[0]);
	}

	if (data.controls & kNT_potC) {
		Pot2Turn(data.pots[1]);
	}

	if (data.controls & kNT_potR) {
		// don't register turns during the brief period where we are lifting our finger after a press
		if (Pot3DownTime == 0 && BlockPot3ChangesUntil <= alg.TotalMs) {
			Pot3Turn(data.pots[2]);
		}
	}

	if ((data.controls & kNT_encoderButtonR) && !(data.lastButtons & kNT_encoderButtonR)) {
		Encoder2DownTime = alg.TotalMs;
		Encoder2Push();
	}

	if (!(data.controls & kNT_encoderButtonR) && (data.lastButtons & kNT_encoderButtonR)) {

		if (Encoder2DownTime > 0) {
			// calculate how long we held the encoder down (in ms)
			auto totalDownTime = alg.TotalMs - Encoder2DownTime;
			if (totalDownTime < ShortPressThreshold) {
				Encoder2ShortPress();
			} else {
				// reset to prepare for another long press
				// we don't fire LongPress from here, because that can fire before even lifting
				Encoder2LongPressFired = false;
			}
			Encoder2DownTime = 0;
		}

		Encoder2Release();
	}

	if ((data.controls & kNT_potButtonR) && !(data.lastButtons & kNT_potButtonR)) {
		Pot3DownTime = alg.TotalMs;
		Pot3Push();
	}

	if (!(data.controls & kNT_potButtonR) && (data.lastButtons & kNT_potButtonR)) {

		if (Pot3DownTime > 0) {
			// calculate how long we held the encoder down (in ms)
			auto totalDownTime = alg.TotalMs - Pot3DownTime;
			if (totalDownTime < ShortPressThreshold) {
				Pot3ShortPress();
			} else {
				// reset to prepare for another long press
				// we don't fire LongPress from here, because that can fire before even lifting
				Pot3LongPressFired = false;
			}
			Pot3DownTime = 0;
			// block any changes from taking place for a brief period afterward, because lifting finger from the pot can cause minute changes otherwise
			BlockPot3ChangesUntil = alg.TotalMs + 100;
		}

		Pot3Release();
	}

	if ((data.controls & kNT_button3) && !(data.lastButtons & kNT_button3)) {
		Button3Push();
	}

	if (!(data.controls & kNT_button3) && (data.lastButtons & kNT_button3)) {
		Button3Release();
	}
}


// this method needs to be called regularly in order to measure time and fire the long press
// generally calling it from draw() is a good idea, because you don't need as frequently as step()
void ViewBase::ProcessLongPresses(const BaseAlgorithm& alg) {
	if (Pot3DownTime > 0) {
		if (!Pot3LongPressFired) {
			// calculate how long we held the pot down (in ms)
			auto totalDownTime = alg.TotalMs - Pot3DownTime;
			if (totalDownTime >= ShortPressThreshold) {
				Pot3LongPress();
				Pot3LongPressFired = true;
			}
		}
	}
	if (Encoder2DownTime > 0) {
		if (!Encoder2LongPressFired) {
			// calculate how long we held the pot down (in ms)
			auto totalDownTime = alg.TotalMs - Encoder2DownTime;
			if (totalDownTime >= ShortPressThreshold) {
				Encoder2LongPress();
				Encoder2LongPressFired = true;
			}
		}
	}
}