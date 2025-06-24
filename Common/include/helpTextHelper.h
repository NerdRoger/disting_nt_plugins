#pragma once
#include <math.h>

struct HelpTextHelper {
	static constexpr int DurationFrames = 90;

	const char* HelpText = NULL;
	int RemainingDuration = 0;

	void DisplayHelpText(const char* text);
	bool Draw();
};
