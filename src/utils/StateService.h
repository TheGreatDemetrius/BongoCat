#pragma once
#include <windows.h>
#include <memory>
#include "SettingsService.h"
#include "ValidationUtils.h"
#include "Configuration.h"
#include "../states/ApplicationState.h"

class ApplicationState;

// Load/validate/persist application state
class StateService {
public:
	static void LoadInitialState(std::unique_ptr<ApplicationState>& stateOut);

	static void ValidateSkinAccess(std::unique_ptr<ApplicationState>& state);

	static void PersistOnExit(const std::unique_ptr<ApplicationState>& state);
};
