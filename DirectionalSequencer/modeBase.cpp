#include <cstring>
#include <distingnt/api.h>
#include "modeBase.h"



void ModeBase::FixFloatBuf() const {
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


void ModeBase::AddSuffixToBuf(const char* suffix) const {
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


void ModeBase::DrawEditBox(uint8_t x, uint8_t y, uint8_t width, const char* text, bool selected, bool editable) const {
	auto allowEdit = selected && editable;
	auto backgroundColor = allowEdit ? EditBoxSelectedBackgroundColor : EditBoxUnselectedBackgroundColor;
	NT_drawShapeI(kNT_rectangle, x - 1, y, x + width + 1, y + 8, backgroundColor);
	auto textColor = allowEdit ? EditBoxSelectedTextColor : EditBoxUnselectedTextColor;
	NT_drawText(x, y + 8, text, textColor);
	if (allowEdit) {
		NT_drawShapeI(kNT_box, x - 2, y - 1, x + width + 2, y + 9, EditBoxSelectedBorderColor);
	}
}