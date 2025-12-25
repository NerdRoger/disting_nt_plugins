#include <stddef.h>
#include <distingnt/api.h>
#include "helpTextHelper.h"


void HelpTextHelper::DisplayHelpText(int xPos, const char* text) {
	HelpText = text;
	RemainingDuration = DurationFrames;
	xPosition = xPos;
}


bool HelpTextHelper::Draw() {
	if (HelpText) {
		NT_drawText(xPosition, 62, HelpText);
		RemainingDuration--;
		if (RemainingDuration <= 0) {
			HelpText = NULL;
		}
		return true;
	}
	return false;
}