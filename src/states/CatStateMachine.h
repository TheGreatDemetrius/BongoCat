#pragma once
#include <functional>
#include "../utils/Configuration.h"

// Forward declaration
class BongoCatApp;

// Cat state
enum class CatState {
	Rest = Configuration::IMAGE_REST,
	LeftPaw = Configuration::IMAGE_LEFT_PAW,
	RightPaw = Configuration::IMAGE_RIGHT_PAW,
	Blink = Configuration::IMAGE_BLINK
};

// Events
enum class StateEvent {
	InputReceived,      // User input (keyboard/mouse)
	TimerExpired,       // Timer for returning to rest
	BlinkTimerExpired,  // Blink timer expired
	SkinChanged         // Skin was changed
};

class CatStateMachine {
private:
	CatState m_currentState;
	CatState m_nextPaw;
	std::function<void(CatState)> m_onStateChanged;
	int64_t m_lastInputTime;  // Timestamp of last input event
	std::function<int64_t()> m_nowMillis;   // Time source for debounce

	// Transition logic
	CatState getNextState(CatState currentState, StateEvent event) const;
	void updateNextPaw();

public:
	CatStateMachine(std::function<void(CatState)> onStateChanged = nullptr,
		std::function<int64_t()> nowMillis = {});

	// State
	CatState GetCurrentState() const noexcept { return m_currentState; }
	void SetCurrentState(CatState state);

	// Events
	void HandleEvent(StateEvent event);

	// Paw
	CatState GetNextPaw() const noexcept { return m_nextPaw; }

	// Callback
	void SetStateChangedCallback(std::function<void(CatState)> callback) {
		m_onStateChanged = callback;
	}

	// Utility
	bool IsInPawState() const;
	bool IsInRestState() const;
	bool IsInBlinkState() const;
};
