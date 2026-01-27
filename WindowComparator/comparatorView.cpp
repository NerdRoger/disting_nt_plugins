#include "comparatorView.h"
#include "windowComparatorAlg.h"
#include "common.h"


static constexpr uint8_t MaxDisplayedLines = 4;
static constexpr uint8_t LinePadding = 2;
static constexpr uint8_t BarHeight = 10;
static constexpr uint8_t BarWidth = 230;
static constexpr uint8_t BarLeft = 15;
static constexpr uint8_t SelectedBarColor = 6;
static constexpr uint8_t UnselectedBarColor = 2;
static constexpr uint8_t SelectedColor = 15;
static constexpr uint8_t UnselectedColor = 4;
static constexpr float   Scale = static_cast<float>(BarWidth) / static_cast<float>(WindowComparatorAlg::Range);


static const char* const ChannelLabels[] = {
	"A", "B", "C", "D", "E", "F", "G", "H",
};


void ComparatorView::OnDrawHandler(ViewBase* view) {
	auto& cv = *static_cast<ComparatorView*>(view);
	cv.DrawComparators();
}


void ComparatorView::OnEncoder1TurnHandler(ViewBase* view, int8_t x) {
	auto& cv = *static_cast<ComparatorView*>(view);
	cv.SelectedComparatorIndex = clamp(cv.SelectedComparatorIndex + x, 0, cv.Algorithm->NumChannels - 1);
}



void ComparatorView::OnPot1TurnHandler(ViewBase* view, float val) {
	auto& cv = *static_cast<ComparatorView*>(view);
	auto algIndex = NT_algorithmIndex(cv.Algorithm);
	auto paramIdx = cv.Algorithm->ChannelOffsets[cv.SelectedComparatorIndex] + kParamBound1;
	auto bound1 = GetScaledParameterValue(*cv.Algorithm, paramIdx);
	float rmin = cv.Algorithm->RangeMin;
	float rmax = cv.Algorithm->RangeMax;
	cv.Algorithm->PotMgr.UpdateValueWithPot(0, val, bound1, rmin, rmax);
	auto unscaled = UnscaleValueForParameter(*cv.Algorithm, paramIdx, bound1);
	NT_setParameterFromAudio(algIndex, paramIdx + NT_parameterOffset(), unscaled);
}


void ComparatorView::OnPot2TurnHandler(ViewBase* view, float val) {
	auto& cv = *static_cast<ComparatorView*>(view);
	auto algIndex = NT_algorithmIndex(cv.Algorithm);
	auto bound1Idx = cv.Algorithm->ChannelOffsets[cv.SelectedComparatorIndex] + kParamBound1;
	auto bound1 = GetScaledParameterValue(*cv.Algorithm, bound1Idx);
	auto bound2Idx = cv.Algorithm->ChannelOffsets[cv.SelectedComparatorIndex] + kParamBound2;
	auto bound2 = GetScaledParameterValue(*cv.Algorithm, bound2Idx);
	auto width = bound2 - bound1;
	auto maxWidth = cv.Algorithm->Range;
	auto oldWidth = width;
	cv.Algorithm->PotMgr.UpdateValueWithPot(1, val, width, 0, maxWidth);
	auto delta = width - oldWidth;
	bound1 -= delta;
	bound2 += delta;
	auto unscaled = UnscaleValueForParameter(*cv.Algorithm, bound1Idx, bound1);
	NT_setParameterFromAudio(algIndex, bound1Idx + NT_parameterOffset(), unscaled);
	unscaled = UnscaleValueForParameter(*cv.Algorithm, bound2Idx, bound2);
	NT_setParameterFromAudio(algIndex, bound2Idx + NT_parameterOffset(), unscaled);
}


void ComparatorView::OnPot3TurnHandler(ViewBase* view, float val) {
	auto& cv = *static_cast<ComparatorView*>(view);
	auto algIndex = NT_algorithmIndex(cv.Algorithm);
	auto paramIdx = cv.Algorithm->ChannelOffsets[cv.SelectedComparatorIndex] + kParamBound2;
	auto bound2 = GetScaledParameterValue(*cv.Algorithm, paramIdx);
	float rmin = cv.Algorithm->RangeMin;
	float rmax = cv.Algorithm->RangeMax;
	cv.Algorithm->PotMgr.UpdateValueWithPot(2, val, bound2, rmin, rmax);
	auto unscaled = UnscaleValueForParameter(*cv.Algorithm, paramIdx, bound2);
	NT_setParameterFromAudio(algIndex, paramIdx + NT_parameterOffset(), unscaled);
}


void ComparatorView::OnFixupPotValuesHandler(ViewBase* view, _NT_float3& pots) {
	// since we are using our own soft takeover logic, always return a midpoint here
	// our logic will take over from there
	pots[0] = 0.5;
	pots[1] = 0.5;
	pots[2] = 0.5;
}


ComparatorView::ComparatorView() {
	OnDraw = OnDrawHandler;
	OnEncoder1Turn = OnEncoder1TurnHandler;
	OnPot1Turn = OnPot1TurnHandler;
	OnPot2Turn = OnPot2TurnHandler;
	OnPot3Turn = OnPot3TurnHandler;
	OnFixupPotValues = OnFixupPotValuesHandler;
}


void ComparatorView::InjectDependencies(WindowComparatorAlg* alg) {
	Algorithm = alg;

  // calculate this once here rather than every draw cycle
	FirstLineY = 0;
	if (Algorithm->NumChannels < MaxDisplayedLines) {
		FirstLineY = LinePadding + (MaxDisplayedLines - Algorithm->NumChannels) * (BarHeight + LinePadding) / 2;
	}
}


void ComparatorView::DrawBullet(int x, int y, int color) const {
	NT_drawShapeI(kNT_rectangle, x, y, x + 2, y + 2, color * 0.4);
	NT_drawShapeI(kNT_line, x + 1, y, x + 1, y + 2, color);
	NT_drawShapeI(kNT_line, x, y + 1, x + 2, y + 1, color);
}


void ComparatorView::DrawComparator(uint8_t ch, uint8_t topIndex) const {
	bool selected = (ch == SelectedComparatorIndex);
	uint8_t barColor = selected ? SelectedBarColor : UnselectedBarColor;
	uint8_t color = selected ? SelectedColor : UnselectedColor;

	// calculate y position for this line
	uint8_t y = FirstLineY;
	y += (ch - topIndex) * (BarHeight + LinePadding);

	// draw the selection bullet
	if (selected) {
		DrawBullet(0, y + 4, SelectedColor);
	}

	// draw the channel label
	NT_drawText(6, y + BarHeight - 1, ChannelLabels[ch], color);

	// draw the bar boundary
	NT_drawShapeI(kNT_box, BarLeft, y, BarLeft + BarWidth, y + BarHeight, barColor);

	// draw some ruler ticks
	for (int i = 1; i < Algorithm->Range; i++) {
		int x = i * Scale;
		uint8_t tickLen = (Algorithm->RangeMin + i == 0) ? 2 : 1;
		NT_drawShapeI(kNT_line, BarLeft + x, y + 1, BarLeft + x, y + tickLen, 1);
		NT_drawShapeI(kNT_line, BarLeft + x, y + BarHeight - tickLen, BarLeft + x, y + BarHeight - 1, 1);
	}

	// get the bounds and value
	uint8_t offset = Algorithm->ChannelOffsets[ch];
	auto bound1 = GetScaledParameterValue(*Algorithm, offset + kParamBound1);
	auto bound2 = GetScaledParameterValue(*Algorithm, offset + kParamBound2);
	lohi(bound1, bound2);
	auto val = Algorithm->CurrentValues[ch];
	auto inWindow = (val >= bound1 && val <= bound2);

	// draw the window
	auto leftPos = static_cast<int>((bound1 - Algorithm->RangeMin) * Scale);
	auto rightPos = static_cast<int>((bound2 - Algorithm->RangeMin) * Scale);
	NT_drawShapeI(kNT_box, BarLeft + leftPos, y, BarLeft + rightPos, y + BarHeight, color);
	NT_drawShapeI(kNT_rectangle, BarLeft + leftPos + 1, y + 1, BarLeft + rightPos - 1, y + BarHeight - 1, inWindow ? color / 2 : 0);
	if (selected) {
		NT_drawShapeI(inWindow ? kNT_rectangle : kNT_box, BarLeft + leftPos + 1, y + 1, BarLeft + rightPos - 1, y + BarHeight - 1, color / 2);
	}

  // draw the value marker
	auto valuePos = BarLeft + static_cast<int>((val - Algorithm->RangeMin) * Scale);
	const int* markerPalette = inWindow ? (const int[]){6, 5, 4, 3, 2} : (const int[]){2, 4, 7, 10, 15};
	for (int i = 0; i < 5; i++) {
		NT_drawShapeI(kNT_line, valuePos, y + i + 1, valuePos, y + BarHeight - i - 1, markerPalette[i]);
	}
	// NT_drawShapeI(kNT_line, BarLeft + valuePos, y + 1, BarLeft + valuePos, y + BarHeight - 1, inWindow ? 6 : 2);
	// NT_drawShapeI(kNT_line, BarLeft + valuePos, y + 2, BarLeft + valuePos, y + BarHeight - 2, inWindow ? 5 : 4);
	// NT_drawShapeI(kNT_line, BarLeft + valuePos, y + 3, BarLeft + valuePos, y + BarHeight - 3, inWindow ? 4 : 7);
	// NT_drawShapeI(kNT_line, BarLeft + valuePos, y + 4, BarLeft + valuePos, y + BarHeight - 4, inWindow ? 3 : 10);
	// NT_drawShapeI(kNT_line, BarLeft + valuePos, y + 5, BarLeft + valuePos, y + BarHeight - 5, inWindow ? 2 : 15);
}


void ComparatorView::DrawComparators() const {
	static constexpr uint8_t maxVisibleComparators = 4;
	auto topIndex = max(static_cast<int>(SelectedComparatorIndex) - 2, 0);
	auto visibleComparators = min(Algorithm->NumChannels, maxVisibleComparators);
	topIndex = min(topIndex, Algorithm->NumChannels - visibleComparators);
	for (int i = topIndex; i < topIndex + visibleComparators; i++) {
		DrawComparator(i, topIndex);
	}
}
