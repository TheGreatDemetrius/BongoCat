#include "CatStateMachine.h"
#include "../utils/Configuration.h"
#include <chrono>

CatStateMachine::CatStateMachine(std::function<void(CatState)> onStateChanged,
	std::function<int64_t()> nowMillis)
	: m_currentState(CatState::Rest)
	, m_nextPaw(CatState::LeftPaw)
	, m_onStateChanged(onStateChanged)
	, m_lastInputTime(0)
	, m_nowMillis(std::move(nowMillis)) {
	if (!m_nowMillis) {
		m_nowMillis = []() -> int64_t {
			auto now = std::chrono::steady_clock::now();
			return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
			};
	}
}

void CatStateMachine::SetCurrentState(CatState state) {
	CatState oldState = m_currentState;
	m_currentState = state;

	if (m_onStateChanged && oldState != state) {
		m_onStateChanged(state);
	}
}

void CatStateMachine::HandleEvent(StateEvent event) {
	CatState currentState = m_currentState;

	// For input events, check if we should ignore rapid inputs
	if (event == StateEvent::InputReceived) {
		const int64_t nowMs = m_nowMillis();
		const int64_t timeSinceLastInput = nowMs - m_lastInputTime;

		// Ignore inputs that come too quickly (less than INPUT_DEBOUNCE_TIME ms apart)
		if (timeSinceLastInput < Configuration::INPUT_DEBOUNCE_TIME) {
			return;
		}

		m_lastInputTime = nowMs;
	}

	CatState nextState = getNextState(currentState, event);

	if (nextState != currentState) {
		SetCurrentState(nextState);
		// Toggle the next paw AFTER transitioning on input, so first input uses current nextPaw
		if (event == StateEvent::InputReceived) {
			updateNextPaw();
		}
	}
}

CatState CatStateMachine::getNextState(CatState currentState, StateEvent event) const {
	switch (currentState) {
	case CatState::Rest:
		switch (event) {
		case StateEvent::InputReceived:
			return m_nextPaw;
		case StateEvent::BlinkTimerExpired:
			return CatState::Blink;
		case StateEvent::SkinChanged:
			return CatState::Rest; // Stay in rest when skin changes
		default:
			return currentState;
		}

	case CatState::LeftPaw:
	case CatState::RightPaw:
		switch (event) {
		case StateEvent::TimerExpired:
			return CatState::Rest;
		case StateEvent::InputReceived:
			// On input, move to the currently scheduled next paw
			return m_nextPaw;
		case StateEvent::SkinChanged:
			return CatState::Rest; // Return to rest when skin changes
		default:
			return currentState;
		}

	case CatState::Blink:
		switch (event) {
		case StateEvent::TimerExpired:
			return CatState::Rest;
		case StateEvent::InputReceived:
			// On input from blink, go to the next paw
			return m_nextPaw;
		case StateEvent::SkinChanged:
			return CatState::Rest; // Return to rest when skin changes
		default:
			return currentState;
		}

	default:
		return currentState;
	}
}

void CatStateMachine::updateNextPaw() {
	CatState currentPaw = m_nextPaw;
	CatState nextPaw = (currentPaw == CatState::LeftPaw) ? CatState::RightPaw : CatState::LeftPaw;
	m_nextPaw = nextPaw;
}

bool CatStateMachine::IsInPawState() const {
	CatState state = m_currentState;
	return state == CatState::LeftPaw || state == CatState::RightPaw;
}

bool CatStateMachine::IsInRestState() const {
	return m_currentState == CatState::Rest;
}

bool CatStateMachine::IsInBlinkState() const {
	return m_currentState == CatState::Blink;
}
