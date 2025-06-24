#include "modeSelector.h"


ModeSelector::ModeSelector() {
	Modes[0] = &Grid;
	Modes[1] = &Settings;
}


void ModeSelector::SelectModeByIndex(uint8_t index) {
	if (index != SelectedModeIndex) {
		SelectedModeIndex = index;
		Modes[index]->Activate();
	}
}


ModeBase& ModeSelector::GetSelectedMode() const {
	return *Modes[SelectedModeIndex];
}


void ModeSelector::Draw() const {
	Modes[0]->DrawIcon(5,   5, SelectedModeIndex == 0 ? SelectedColor : UnselectedColor);
	Modes[1]->DrawIcon(26,  5, SelectedModeIndex == 1 ? SelectedColor : UnselectedColor);
}


void ModeSelector::FixupPotValues(_NT_float3& pots) const {
	// we fix up pot1 here, and the other pots in the selected mode
	auto size = ARRAY_SIZE(Modes);
	auto epsilon = 0.5f / size;
	pots[0] = static_cast<float>(SelectedModeIndex) / size + epsilon;

	GetSelectedMode().FixupPotValues(pots);
}


void ModeSelector::Initialize(DirectionalSequencer& alg) {
	OwnedBase::Initialize(alg);
	for(auto mode : Modes) {
		mode->Initialize(alg);
	}
}