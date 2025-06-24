#include <stddef.h>
#include <distingnt/api.h>
#include "helpTextHelper.h"


void HelpTextHelper::DisplayHelpText(const char* text) {
	HelpText = text;
	RemainingDuration = DurationFrames;
}


bool HelpTextHelper::Draw() {
	if (HelpText) {
		NT_drawText(0, 62, HelpText);
		RemainingDuration--;
		if (RemainingDuration <= 0) {
			HelpText = NULL;
		}
		return true;
	}
	return false;
}