#include "ApplicationState.h"
#include "../utils/ValidationUtils.h"

ApplicationState::ApplicationState()
	: m_clickCount(0)
	, m_currentSkin(Configuration::SKIN_MARSHMALLOW)
	, m_isKeyPressed(false)
	, m_isVisible(true) {

	// Create state machine
	m_stateMachine = std::make_unique<CatStateMachine>();
}

bool ApplicationState::CanUnlockSkin(int skinId) const {
	return ValidationUtils::CanUnlockSkin(skinId, GetClickCount());
}

// Convenience
int ApplicationState::GetCurrentImageIndex() const {
	return static_cast<int>(m_stateMachine->GetCurrentState());
}
