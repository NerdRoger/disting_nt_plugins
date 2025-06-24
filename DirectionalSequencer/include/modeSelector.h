#pragma once
#include "ownedBase.h"
#include "modeBase.h"
#include "gridMode.h"
#include "settingsMode.h"

struct ModeSelector : OwnedBase<DirectionalSequencer> {
private:
	uint8_t	SelectedModeIndex;

public:
	static constexpr int SelectedColor = 15;
	static constexpr int UnselectedColor = 5;

	GridMode Grid;
	SettingsMode Settings;
	ModeBase* Modes[2];
	ModeSelector();
	void SelectModeByIndex(uint8_t index);
	ModeBase& GetSelectedMode() const;
	void Draw() const;
	void FixupPotValues(_NT_float3& pots) const;
	virtual void Initialize(DirectionalSequencer& alg) override;
};

