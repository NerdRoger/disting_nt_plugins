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
	if (data.encoders[0] && OnEncoder1Turn) {
		OnEncoder1Turn(this, data.encoders[0]);
	}

	if (data.encoders[1] && OnEncoder2Turn) {
		OnEncoder2Turn(this, data.encoders[1]);
	}

	if (data.controls & kNT_potL && OnPot1Turn) {
		OnPot1Turn(this, data.pots[0]);
	}

	if (data.controls & kNT_potC && OnPot2Turn) {
		OnPot2Turn(this, data.pots[1]);
	}

	if (data.controls & kNT_potR) {
		// don't register turns during the brief period where we are lifting our finger after a press
		if (Pot3DownTime == 0 && BlockPot3ChangesUntil <= Timer->TotalMs && OnPot3Turn) {
			OnPot3Turn(this, data.pots[2]);
		}
	}

	if ((data.controls & kNT_encoderButtonR) && !(data.lastButtons & kNT_encoderButtonR)) {
		Encoder2DownTime = Timer->TotalMs;
		if (OnEncoder2Push)
			OnEncoder2Push(this);
	}

	if (!(data.controls & kNT_encoderButtonR) && (data.lastButtons & kNT_encoderButtonR)) {

		if (Encoder2DownTime > 0) {
			// calculate how long we held the encoder down (in ms)
			auto totalDownTime = Timer->TotalMs - Encoder2DownTime;
			if (totalDownTime < ShortPressThreshold) {
				if (OnEncoder2ShortPress)
					OnEncoder2ShortPress(this);
			} else {
				// reset to prepare for another long press
				// we don't fire LongPress from here, because that can fire before even lifting
				Encoder2LongPressFired = false;
			}
			Encoder2DownTime = 0;
		}

		if (OnEncoder2Release)
			OnEncoder2Release(this);
	}

	if ((data.controls & kNT_potButtonR) && !(data.lastButtons & kNT_potButtonR)) {
		Pot3DownTime = Timer->TotalMs;
		if (OnPot3Push)
			OnPot3Push(this);
	}

	if (!(data.controls & kNT_potButtonR) && (data.lastButtons & kNT_potButtonR)) {

		if (Pot3DownTime > 0) {
			// calculate how long we held the encoder down (in ms)
			auto totalDownTime = Timer->TotalMs - Pot3DownTime;
			if (totalDownTime < ShortPressThreshold) {
				if (OnPot3ShortPress)
					OnPot3ShortPress(this);
			} else {
				// reset to prepare for another long press
				// we don't fire LongPress from here, because that can fire before even lifting
				Pot3LongPressFired = false;
			}
			Pot3DownTime = 0;
			// block any changes from taking place for a brief period afterward, because lifting finger from the pot can cause minute changes otherwise
			BlockPot3ChangesUntil = Timer->TotalMs + 100;
		}

		if (OnPot3Release)
			OnPot3Release(this);
	}

	if ((data.controls & kNT_button3) && !(data.lastButtons & kNT_button3) && OnButton3Push) {
		OnButton3Push(this);
	}

	if (!(data.controls & kNT_button3) && (data.lastButtons & kNT_button3) && OnButton3Release) {
		OnButton3Release(this);
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
				if (OnPot3LongPress)
					OnPot3LongPress(this);
				Pot3LongPressFired = true;
			}
		}
	}
	if (Encoder2DownTime > 0) {
		if (!Encoder2LongPressFired) {
			// calculate how long we held the pot down (in ms)
			auto totalDownTime = Timer->TotalMs - Encoder2DownTime;
			if (totalDownTime >= ShortPressThreshold) {
				if (OnEncoder2LongPress)
					OnEncoder2LongPress(this);
				Encoder2LongPressFired = true;
			}
		}
	}
}


void ViewBase::Activate() {
	if (OnActivate)
		OnActivate(this);
}


void ViewBase::Draw() {
	ProcessLongPresses();
	if (OnDraw)
		OnDraw(this);
}


void ViewBase::FixupPotValues(_NT_float3& pots) {
	if (OnFixupPotValues)
		OnFixupPotValues(this, pots);
}


void ViewBase::ParameterChanged(int paramIndex) {
	if (OnParameterChanged)
		OnParameterChanged(this, paramIndex);
}
