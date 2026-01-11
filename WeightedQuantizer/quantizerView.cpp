#include <string.h>
#include "quantizerView.h"
#include "weightedQuantizerAlg.h"
#include "common.h"


static constexpr int KeyWidth = 13;
static constexpr int KeyHeight = 15;
static constexpr int KeySpacing = 3;

static constexpr int KeyBackgroundColor = 1;
static constexpr int KeyForegroundColor = 8;
static constexpr int SelectedKeyBorderColor = 15;
static constexpr float MaxSliderValue = 10.0f;

struct Control {
	uint8_t     ParameterIndex;
	const char* HelpText;
};

static const Control KeyControls[] = {
	{ kWQParamQuantWeightC,      "Adjust the attraction weighting of note C"  },
	{ kWQParamQuantWeightCSharp, "Adjust the attraction weighting of note C#" },
	{ kWQParamQuantWeightD,      "Adjust the attraction weighting of note D"  },
	{ kWQParamQuantWeightDSharp, "Adjust the attraction weighting of note D#" },
	{ kWQParamQuantWeightE,      "Adjust the attraction weighting of note E"  },
	{ kWQParamQuantWeightF,      "Adjust the attraction weighting of note F"  },
	{ kWQParamQuantWeightFSharp, "Adjust the attraction weighting of note F#" },
	{ kWQParamQuantWeightG,      "Adjust the attraction weighting of note G"  },
	{ kWQParamQuantWeightGSharp, "Adjust the attraction weighting of note G#" },
	{ kWQParamQuantWeightA,      "Adjust the attraction weighting of note A"  },
	{ kWQParamQuantWeightASharp, "Adjust the attraction weighting of note A#" },
	{ kWQParamQuantWeightB,      "Adjust the attraction weighting of note B"  },
};


void QuantizerView::OnActivateHandler(ViewBase* view) {
	auto& qv = *static_cast<QuantizerView*>(view);
	qv.LoadKeyControlForEditing();
}


void QuantizerView::OnDrawHandler(ViewBase* view) {
	auto& qv = *static_cast<QuantizerView*>(view);
	qv.DrawBanks();
	qv.DrawKeyboard();
	if (qv.BankPeeking) {
		qv.DrawPeek();
	} else {
		qv.DrawResults();
	}
	qv.DrawHelpSection();
}


void QuantizerView::OnEncoder1TurnHandler(ViewBase* view, int8_t x) {
	auto& qv = *static_cast<QuantizerView*>(view);
	if (!qv.BankPeeking) {
		qv.SelectedKeyIndex = wrap(static_cast<int>(qv.SelectedKeyIndex) + x, 0, 11);
		qv.LoadKeyControlForEditing();
		qv.Algorithm->HelpText.DisplayHelpText(10, KeyControls[qv.SelectedKeyIndex].HelpText);
	}
}


void QuantizerView::OnEncoder2TurnHandler(ViewBase* view, int8_t x) {
	auto& qv = *static_cast<QuantizerView*>(view);
	if (!qv.BankPeeking) {
		qv.SelectedBankIndex = wrap(static_cast<int>(qv.SelectedBankIndex) + x, 0, 9);
		qv.Algorithm->HelpText.DisplayHelpText(15, "Short press to load, long press to save");
	}
}


void QuantizerView::OnEncoder2ShortPressHandler(ViewBase* view) {
	auto& qv = *static_cast<QuantizerView*>(view);
	auto algIndex = NT_algorithmIndex(qv.Algorithm);
	auto& param = qv.Algorithm->parameters[kWQParamBankScanPosition];
	auto val = (qv.SelectedBankIndex + 1) * CalculateScaling(param.scaling);
	NT_setParameterFromUi(algIndex, kWQParamBankScanPosition + NT_parameterOffset(), val);

	qv.Algorithm->Banks.LoadNotesFromBank(qv.SelectedBankIndex);
	qv.LoadKeyControlForEditing();
	qv.Algorithm->HelpText.DisplayHelpText(60, "Loaded notes from bank");
}


void QuantizerView::OnEncoder2LongPressHandler(ViewBase* view) {
	auto& qv = *static_cast<QuantizerView*>(view);
	qv.Algorithm->Banks.SaveNotesToBank(qv.SelectedBankIndex);
	qv.Algorithm->HelpText.DisplayHelpText(65, "Saved notes to bank");
}


void QuantizerView::OnPot1TurnHandler(ViewBase* view, float val) {
	auto& qv = *static_cast<QuantizerView*>(view);
	if (!qv.BankPeeking) {
		if (!qv.Algorithm->Banks.ScanningLocked) {
			qv.Algorithm->HelpText.DisplayHelpText(20, "Disable bank scanning to change weights");
		} else {
			auto algIndex = NT_algorithmIndex(qv.Algorithm);
			auto parameterIndex = KeyControls[qv.SelectedKeyIndex].ParameterIndex;
			auto& param = qv.Algorithm->parameters[parameterIndex];
			qv.Algorithm->PotMgr.UpdateValueWithPot(0, val, qv.SelectedKeyValueRaw, param.min, param.max);
			NT_setParameterFromUi(algIndex, parameterIndex + NT_parameterOffset(), qv.SelectedKeyValueRaw);
			qv.Algorithm->HelpText.DisplayHelpText(10, KeyControls[qv.SelectedKeyIndex].HelpText);
		}
	}
}


void QuantizerView::OnPot3TurnHandler(ViewBase* view, float val) {
	auto& qv = *static_cast<QuantizerView*>(view);
	if (!qv.BankPeeking) {
		if (!qv.Algorithm->Banks.ScanningLocked) {
			auto algIndex = NT_algorithmIndex(qv.Algorithm);
			auto& param = qv.Algorithm->parameters[kWQParamBankScanPosition];
			auto scanPos = GetScaledParameterValue(*qv.Algorithm, kWQParamBankScanPosition);
			auto scaling = CalculateScaling(param.scaling);
			auto min = param.min / scaling;
			auto max = param.max / scaling;
			qv.Algorithm->PotMgr.UpdateValueWithPot(2, val, scanPos, min, max);
			auto paramVal = scanPos * scaling;
			NT_setParameterFromUi(algIndex, kWQParamBankScanPosition + NT_parameterOffset(), paramVal);
		} else {
			qv.Algorithm->HelpText.DisplayHelpText(45, "Push to enable bank scanning");
		}
	}
}


void QuantizerView::OnPot3ShortPressHandler(ViewBase* view) {
	auto& qv = *static_cast<QuantizerView*>(view);
	if (!qv.BankPeeking) {
		qv.Algorithm->Banks.ScanningLocked = !qv.Algorithm->Banks.ScanningLocked;
		if (!qv.Algorithm->Banks.ScanningLocked) {
			qv.Algorithm->Banks.DoBankScan(qv.Algorithm->v[kWQParamBankScanPosition]);
			qv.Algorithm->HelpText.DisplayHelpText(80, "Bank scanning on");
		} else {
			qv.Algorithm->HelpText.DisplayHelpText(80, "Bank scanning off");
		}
	}
}


void QuantizerView::OnButton3PushHandler(ViewBase* view) {
	auto& qv = *static_cast<QuantizerView*>(view);
	qv.BankPeeking = true;
}


void QuantizerView::OnButton3ReleaseHandler(ViewBase* view) {
	auto& qv = *static_cast<QuantizerView*>(view);
	qv.BankPeeking = false;
}


void QuantizerView::OnFixupPotValuesHandler(ViewBase* view, _NT_float3& pots) {
	auto& qv = *static_cast<QuantizerView*>(view);
	// calculate p1
	auto parameterIndex = KeyControls[qv.SelectedKeyIndex].ParameterIndex;
	auto& keyParam = qv.Algorithm->parameters[parameterIndex];
	auto keyVal = qv.Algorithm->v[parameterIndex];
	bool isEnum = keyParam.unit == kNT_unitEnum;
	auto min = keyParam.min;
	auto max = keyParam.max + (isEnum ? 0 : 0.99f);
	auto range = max - min;
	pots[0] = (keyVal - min) / range;

	// calculate p3
	auto& scanParam = qv.Algorithm->parameters[kWQParamBankScanPosition];
	auto scanVal = qv.Algorithm->v[kWQParamBankScanPosition];
	pots[2] = static_cast<float>(scanVal - scanParam.min) / static_cast<float>(scanParam.max - scanParam.min);
	// we need to reload our control for editing because the underlying parameter value could have changed while we were "sleeping"
	// this works because this event is fired when returning to our algorithm from other disting screens
	qv.LoadKeyControlForEditing();
}


void QuantizerView::OnParameterChangedHandler(ViewBase* view, int paramIndex) {
	auto& qv = *static_cast<QuantizerView*>(view);
	qv.LoadKeyControlForEditing();
}


QuantizerView::QuantizerView() {
	OnActivate = OnActivateHandler;
	OnDraw = OnDrawHandler;
	OnEncoder1Turn = OnEncoder1TurnHandler;
	OnEncoder2Turn = OnEncoder2TurnHandler;
	OnEncoder2ShortPress = OnEncoder2ShortPressHandler;
	OnEncoder2LongPress = OnEncoder2LongPressHandler;
	OnPot1Turn = OnPot1TurnHandler;
	OnPot3Turn = OnPot3TurnHandler;
	OnPot3ShortPress = OnPot3ShortPressHandler;
	OnButton3Push = OnButton3PushHandler;
	OnButton3Release = OnButton3ReleaseHandler;
	OnFixupPotValues = OnFixupPotValuesHandler;
	OnParameterChanged = OnParameterChangedHandler;
}


void QuantizerView::InjectDependencies(WeightedQuantizerAlg* alg) {
	Algorithm = alg;
	ViewBase::InjectDependencies(&alg->Timer);
}


void QuantizerView::DrawBank(size_t bankNum, const char* label) const {
	auto x = bankNum * 28 - (bankNum == 9 ? 3 : 0);
	auto color = (bankNum == SelectedBankIndex) ? KeyForegroundColor : KeyBackgroundColor;
	NT_drawText(x, 5, label, color, kNT_textLeft, kNT_textTiny);
}


void QuantizerView::DrawBanks() const {
	// draw the bank scanner
	if (Algorithm->Banks.ScanningLocked) {
		NT_drawShapeI(kNT_rectangle, 0, 6, 255, 8, KeyForegroundColor);
		NT_drawShapeI(kNT_line, 0, 7, 255, 7, KeyBackgroundColor);
	} else {
		auto scanPos = GetScaledParameterValue(*Algorithm, kWQParamBankScanPosition);
		int barPos = 255 * (scanPos - 1) / 9.0f;
		NT_drawShapeI(kNT_rectangle, 0, 6, 255, 8, KeyBackgroundColor);
		NT_drawShapeI(kNT_rectangle, 0, 6, barPos, 8, KeyForegroundColor);
		NT_drawShapeI(kNT_line, barPos, 6, barPos, 8, SelectedKeyBorderColor);
	}

	// then the banks
	DrawBank(0, "1");
	DrawBank(1, "2");
	DrawBank(2, "3");
	DrawBank(3, "4");
	DrawBank(4, "5");
	DrawBank(5, "6");
	DrawBank(6, "7");
	DrawBank(7, "8");
	DrawBank(8, "9");
	DrawBank(9, "10");
}


void QuantizerView::DrawKey(uint8_t x, uint8_t y, uint16_t paramIndex) const {
	auto bank = Algorithm->Banks[SelectedBankIndex];
	auto rawVal = BankPeeking ? bank.NoteValues[paramIndex - kWQParamQuantWeightC] : Algorithm->v[paramIndex];
	float val = static_cast<float>(rawVal) / CalculateScaling(Algorithm->parameters[paramIndex].scaling);
	
	// draw the background color
	NT_drawShapeI(kNT_rectangle, x, y, x + KeyWidth - 1, y + KeyHeight - 1, KeyBackgroundColor);

	// now draw the bar represending the value in the foreground color
	auto scaled = val * KeyHeight / MaxSliderValue;
	auto full = static_cast<int>(scaled);
	auto frac = scaled - full;
	auto fracColor = static_cast<int>(frac * (KeyForegroundColor - KeyBackgroundColor) + KeyBackgroundColor);
	
	// don't let the fractional color get below 1 if this is our only "row"
	if (full == 0 && frac > 0 && fracColor < 1) {
		fracColor = 1;
	}
	if (full > 0) {
		NT_drawShapeI(kNT_rectangle, x, y + KeyHeight - full, x + KeyWidth - 1, y + KeyHeight - 1, KeyForegroundColor);
	}
	if (frac > 0) {
		NT_drawShapeI(kNT_rectangle, x, y + KeyHeight - full - 1, x + KeyWidth - 1, y + KeyHeight - full - 1, fracColor);
	}

	// finally draw the border if selected
	if (KeyControls[SelectedKeyIndex].ParameterIndex == paramIndex && !BankPeeking) {
		NT_drawShapeI(kNT_box, x - 2, y - 2, x + KeyWidth - 1 + 2, y + KeyHeight - 1 + 2, SelectedKeyBorderColor);
	}
}


void QuantizerView::DrawKeyboard() const {
	int x = 5;
	int y = 31;
	DrawKey(x, y, kWQParamQuantWeightC);
	x += KeyWidth + KeySpacing;
	DrawKey(x, y, kWQParamQuantWeightD);
	x += KeyWidth + KeySpacing;
	DrawKey(x, y, kWQParamQuantWeightE);
	x += KeyWidth + KeySpacing;
	DrawKey(x, y, kWQParamQuantWeightF);
	x += KeyWidth + KeySpacing;
	DrawKey(x, y, kWQParamQuantWeightG);
	x += KeyWidth + KeySpacing;
	DrawKey(x, y, kWQParamQuantWeightA);
	x += KeyWidth + KeySpacing;
	DrawKey(x, y, kWQParamQuantWeightB);

	x = 5 + (KeyWidth + KeySpacing) / 2;
	y -= (KeyHeight + KeySpacing);

	DrawKey(x, y, kWQParamQuantWeightCSharp);
	x += KeyWidth + KeySpacing;
	DrawKey(x, y, kWQParamQuantWeightDSharp);
	x += KeyWidth + KeySpacing;
	x += KeyWidth + KeySpacing;
	DrawKey(x, y, kWQParamQuantWeightFSharp);
	x += KeyWidth + KeySpacing;
	DrawKey(x, y, kWQParamQuantWeightGSharp);
	x += KeyWidth + KeySpacing;
	DrawKey(x, y, kWQParamQuantWeightASharp);
}


void QuantizerView::DrawPeek() const {
	NT_drawText(145, 30, "Peeking at bank", 8);
	char buf[3];
	NT_intToString(buf, SelectedBankIndex + 1);
	NT_drawText(183, 40, buf, 8);
}


void QuantizerView::DrawHelpSection() const {
	// clear the area we're about to draw in, as we are drawing on top of the grid
	NT_drawShapeI(kNT_rectangle, 0, 50, 255, 63, 0);

	// draw the transient help text if present
	if (!Algorithm->HelpText.Draw()) {
		NT_drawText(2, 58, "Set Weight", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(43, 64, "Select Note", 15, kNT_textLeft, kNT_textTiny);


		NT_drawText(162, 64, "Select Bank", 15, kNT_textLeft, kNT_textTiny);
		NT_drawText(219, 58, "Scan Banks", 15, kNT_textLeft, kNT_textTiny);
	}
	NT_drawShapeI(kNT_line, 0, 50, 255, 50, 15);
}


void QuantizerView::DrawArrow(uint8_t x, uint8_t y, uint8_t color) const {
	NT_drawShapeI(kNT_line, x, y - 4, x + 4, y - 4, color);
	NT_drawShapeI(kNT_line, x + 4, y - 4, x + 2, y - 6, color);
	NT_drawShapeI(kNT_line, x + 4, y - 4, x + 2, y - 2, color);
}


void QuantizerView::DrawResult(uint8_t x, uint8_t y, const char* label, size_t channelIndex, uint8_t color) const {
	auto a = Algorithm->QuantResults[channelIndex].QuantizedNoteName;
	auto b = Algorithm->QuantResults[channelIndex].FinalNoteName;
	NT_drawText(x, y, label, 8);
	NT_drawText(x + 28, y, a, 8);
	if (strcmp(a, b) != 0) {
		DrawArrow(x + 42, y, 8);
		NT_drawText(x + 49, y, b, 8);
	}
}


void QuantizerView::DrawResults() const {
	switch (Algorithm->NumChannels)	{
		// these cases explicity fall-through
		case 8:
			DrawResult(192, 49, "Ch 8:", 7, 8);
		case 7:
			DrawResult(192, 39, "Ch 7:", 6, 8);
		case 6:
			DrawResult(192, 29, "Ch 6:", 5, 8);
		case 5:
			DrawResult(192, 19, "Ch 5:", 4, 8);
		case 4:
			DrawResult(123, 49, "Ch 4:", 3, 8);
		case 3:
			DrawResult(123, 39, "Ch 3:", 2, 8);
		case 2:
			DrawResult(123, 29, "Ch 2:", 1, 8);
		case 1:
			DrawResult(123, 19, "Ch 1:", 0, 8);
		default:
			break;
	}
}


void QuantizerView::LoadKeyControlForEditing() {
	auto parameterIndex = KeyControls[SelectedKeyIndex].ParameterIndex;
	auto val = Algorithm->v[parameterIndex];
	// since we are dealing with unscaled numbers here, epsilon is always 0.5
	SelectedKeyValueRaw = val + 0.5f;
}
