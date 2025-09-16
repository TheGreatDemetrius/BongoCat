#pragma once
#include <windows.h>
#include <memory>
#include "../utils/RAII/Hook.h"

// Forward declaration
class BongoCatApp;

// Global hook procedures
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);

class InputManager {
private:
	BongoCatApp* m_app;
	std::unique_ptr<HookWrapper> m_keyboardHook;
	std::unique_ptr<HookWrapper> m_mouseHook;

	// Helper methods
	bool InstallHooks();
	void RemoveHooks();

public:
	InputManager(BongoCatApp* app);
	~InputManager();

	// Initialization and cleanup
	bool Initialize();
	void Shutdown();

	// Event handlers (called from global hook procedures)
	void OnKeyboardEvent(WPARAM wParam, LPARAM lParam);
	void OnMouseEvent(WPARAM wParam, LPARAM lParam);
};
