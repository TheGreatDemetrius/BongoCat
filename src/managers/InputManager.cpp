#include "InputManager.h"
#include "../utils/Configuration.h"
#include <atomic>
#include "../states/ApplicationState.h"
#include "../app/BongoCatApp.h"

// File-scope hook target
namespace {
	std::atomic<BongoCatApp*> g_hooksApp{ nullptr };

	inline void SetHooksApp(BongoCatApp* app) {
		g_hooksApp.store(app, std::memory_order_release);
	}

	inline BongoCatApp* GetHooksApp() {
		return g_hooksApp.load(std::memory_order_acquire);
	}
}

// Global hook procedures
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= HC_ACTION) {
		BongoCatApp* app = GetHooksApp();
		if (app && app->GetInputManager()) {
			app->GetInputManager()->OnKeyboardEvent(wParam, lParam);
		}
	}
	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= HC_ACTION) {
		BongoCatApp* app = GetHooksApp();
		if (app && app->GetInputManager()) {
			app->GetInputManager()->OnMouseEvent(wParam, lParam);
		}
	}
	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

InputManager::InputManager(BongoCatApp* app)
	: m_app(app) {
	// Expose app to global hook procedures
	SetHooksApp(app);
}

InputManager::~InputManager() {
	// Clear global hook app if we are the owner
	if (GetHooksApp() == m_app) {
		SetHooksApp(nullptr);
	}
}

bool InputManager::Initialize() {
	return InstallHooks();
}

void InputManager::Shutdown() {
	RemoveHooks();
}

bool InputManager::InstallHooks() {
	// Install keyboard hook
	if (!m_app) return false;
	m_keyboardHook = std::make_unique<HookWrapper>(WH_KEYBOARD_LL,
		::LowLevelKeyboardProc, m_app->GetInstance());
	if (!m_keyboardHook || !m_keyboardHook->isValid()) {
		m_keyboardHook.reset();
		return false;
	}

	// Install mouse hook
	m_mouseHook = std::make_unique<HookWrapper>(WH_MOUSE_LL,
		::LowLevelMouseProc, m_app->GetInstance());
	if (!m_mouseHook || !m_mouseHook->isValid()) {
		m_keyboardHook.reset();
		m_mouseHook.reset();
		return false;
	}

	return true;
}

void InputManager::RemoveHooks() {
	// Remove hooks in reverse order
	m_mouseHook.reset();
	m_keyboardHook.reset();
}

void InputManager::OnKeyboardEvent(WPARAM wParam, LPARAM lParam) {
	if (!m_app || !m_app->GetState()) return;

	bool keydown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
	bool keyup = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

	if (keydown && !m_app->GetState()->IsKeyPressed()) {
		m_app->GetState()->SetKeyPressed(true);
		HWND mainWindow = m_app->GetMainWindow();
		if (mainWindow) {
			// Use SendNotifyMessage for better performance - non-blocking
			SendNotifyMessage(mainWindow, Configuration::WM_APP_INPUT_EVENT, 0, 0);
		}
	}
	else if (keyup) {
		m_app->GetState()->SetKeyPressed(false);
	}
}

void InputManager::OnMouseEvent(WPARAM wParam, LPARAM lParam) {
	if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN) {
		if (m_app && m_app->GetMainWindow()) {
			// Use SendNotifyMessage for better performance - non-blocking
			SendNotifyMessage(m_app->GetMainWindow(), Configuration::WM_APP_INPUT_EVENT, 0, 0);
		}
	}
}
