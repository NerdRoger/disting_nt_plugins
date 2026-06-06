# Directional Sequencer Bugs / Investigation Notes

## Intermittent pot value jump on screen load

Symptom: occasionally, when the Directional Sequencer screen is first loaded and a pot is turned, the edited cell value jumps unexpectedly to one extreme or another. This has been observed sporadically, not every time.

Suspected area: custom UI setup, soft-takeover synchronization, and the editor cache value used by pot 3 (`GridView::ParamEditRaw`). The setup path reports pot values to the NT firmware via `SetupUI()` / `FixupPotValues()`, while `LoadParamForEditing()` initializes the internal edit baseline. If those become out of sync during initial screen load, the next pot movement may apply from an unexpected baseline.

Current mitigation: `DirSeqAlg::SetupUI()` was changed to remove a duplicate `Grid.LoadParamForEditing()` call. `GridView::OnFixupPotValuesHandler()` already calls `LoadParamForEditing()` after setting pot 3 to midpoint for soft takeover, so the extra call was redundant and may have contributed to ordering-sensitive jumps.

If this persists: inspect other paths that call `Grid.LoadParamForEditing()`, especially `DirSeqAlg::StepDataChangedHandler()` via `StepDataRegion::DoDataChanged()`. Mod-matrix parameter changes may occur from the audio/parameter callback path and can refresh `ParamEditRaw` asynchronously relative to UI setup or pot movement. A possible follow-up is to defer editor-cache refreshes with a dirty flag consumed from `draw()`/`customUi()`, instead of refreshing directly from data-change callbacks.
