#pragma once
#include <memory>
#include "../utils/Configuration.h"
#include "CatStateMachine.h"

class ApplicationState {
private:
	int m_clickCount;
	int m_currentSkin;
	bool m_isKeyPressed;
	bool m_isVisible;
	std::unique_ptr<CatStateMachine> m_stateMachine;

public:
	ApplicationState();

	// State machine access
	CatStateMachine* GetStateMachine() const noexcept { return m_stateMachine.get(); }

	// Click state
	int GetClickCount() const noexcept { return m_clickCount; }
	void IncrementClickCount() noexcept { ++m_clickCount; }
	void SetClickCount(int count) noexcept { m_clickCount = count; }

	// Skin state
	int GetCurrentSkin() const noexcept { return m_currentSkin; }
	void SetCurrentSkin(int skin) noexcept { m_currentSkin = skin; }

	// Input state
	bool IsKeyPressed() const noexcept { return m_isKeyPressed; }
	void SetKeyPressed(bool pressed) noexcept { m_isKeyPressed = pressed; }

	// Visibility state
	bool IsVisible() const noexcept { return m_isVisible; }
	void SetVisible(bool visible) noexcept { m_isVisible = visible; }

	// Business logic
	bool CanUnlockSkin(int skinId) const;

	// Convenience methods
	int GetCurrentImageIndex() const;
};
