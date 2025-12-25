#include <cstring>
#include <distingnt/api.h>
#include "viewBase.h"


void ViewBase::InjectDependencies(TimeKeeper* timer) {
	Timer = timer;
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


void ViewBase::ProcessControlInput(const _NT_uiData& data) {
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
		if (Pot3DownTime == 0 && BlockPot3ChangesUntil <= Timer->TotalMs) {
			Pot3Turn(data.pots[2]);
		}
	}

	if ((data.controls & kNT_encoderButtonR) && !(data.lastButtons & kNT_encoderButtonR)) {
		Encoder2DownTime = Timer->TotalMs;
		Encoder2Push();
	}

	if (!(data.controls & kNT_encoderButtonR) && (data.lastButtons & kNT_encoderButtonR)) {

		if (Encoder2DownTime > 0) {
			// calculate how long we held the encoder down (in ms)
			auto totalDownTime = Timer->TotalMs - Encoder2DownTime;
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
		Pot3DownTime = Timer->TotalMs;
		Pot3Push();
	}

	if (!(data.controls & kNT_potButtonR) && (data.lastButtons & kNT_potButtonR)) {

		if (Pot3DownTime > 0) {
			// calculate how long we held the encoder down (in ms)
			auto totalDownTime = Timer->TotalMs - Pot3DownTime;
			if (totalDownTime < ShortPressThreshold) {
				Pot3ShortPress();
			} else {
				// reset to prepare for another long press
				// we don't fire LongPress from here, because that can fire before even lifting
				Pot3LongPressFired = false;
			}
			Pot3DownTime = 0;
			// block any changes from taking place for a brief period afterward, because lifting finger from the pot can cause minute changes otherwise
			BlockPot3ChangesUntil = Timer->TotalMs + 100;
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
void ViewBase::ProcessLongPresses() {
	if (Pot3DownTime > 0) {
		if (!Pot3LongPressFired) {
			// calculate how long we held the pot down (in ms)
			auto totalDownTime = Timer->TotalMs - Pot3DownTime;
			if (totalDownTime >= ShortPressThreshold) {
				Pot3LongPress();
				Pot3LongPressFired = true;
			}
		}
	}
	if (Encoder2DownTime > 0) {
		if (!Encoder2LongPressFired) {
			// calculate how long we held the pot down (in ms)
			auto totalDownTime = Timer->TotalMs - Encoder2DownTime;
			if (totalDownTime >= ShortPressThreshold) {
				Encoder2LongPress();
				Encoder2LongPressFired = true;
			}
		}
	}
}