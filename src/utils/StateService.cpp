#include "StateService.h"
#include "SettingsService.h"
#include "ValidationUtils.h"
#include "Configuration.h"
#include "../states/ApplicationState.h"

void StateService::LoadInitialState(std::unique_ptr<ApplicationState>& stateOut) {
	if (!stateOut) return;
	stateOut->SetClickCount(SettingsService::ReadClickCount());
	int skin = SettingsService::ReadSkin();
	if (!ValidationUtils::IsValidSkin(skin)) {
		skin = Configuration::SKIN_MARSHMALLOW;
	}
	stateOut->SetCurrentSkin(skin);
}

void StateService::ValidateSkinAccess(std::unique_ptr<ApplicationState>& state) {
	if (!state) return;
	const int currentSkin = state->GetCurrentSkin();
	const int clickCount = state->GetClickCount();
	if (!ValidationUtils::CanUnlockSkin(currentSkin, clickCount)) {
		state->SetCurrentSkin(Configuration::SKIN_MARSHMALLOW);
		SettingsService::WriteSkin(Configuration::SKIN_MARSHMALLOW);
	}
}

void StateService::PersistOnExit(const std::unique_ptr<ApplicationState>& state) {
	if (!state) return;
	SettingsService::WriteClickCount(state->GetClickCount());
}
