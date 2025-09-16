#include "BongoCatApp.h"
#include "../states/CatStateMachine.h"
#include "../utils/Configuration.h"
#include "../utils/SettingsService.h"
#include "../utils/StateService.h"
#include "../managers/ImageManager.h"
#include "../managers/WindowManager.h"
#include "../managers/InputManager.h"
#include <windows.h>
#include "../utils/Localization.h"
#if __has_include("Resource.h")
#include "Resource.h"
#elif __has_include("../../build/Resource.h")
#include "../../build/Resource.h"
#endif

// Global window procedure required by Win32 API
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if (message == WM_CREATE) {
		CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		BongoCatApp* app = reinterpret_cast<BongoCatApp*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
		return 0;
	}

	BongoCatApp* app = reinterpret_cast<BongoCatApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (app && app->GetWindowManager()) {
		return app->GetWindowManager()->WindowProc(hWnd, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

BongoCatApp::BongoCatApp()
	: m_hInstance(nullptr)
	, m_hMainWindow(nullptr) {
	m_state = std::make_unique<ApplicationState>();
}

BongoCatApp::~BongoCatApp() {
	Shutdown();
}

bool BongoCatApp::Initialize(HINSTANCE hInstance) {
	m_hInstance = hInstance;

	// Initialize GDI+
	if (!InitializeGdiPlus()) {
		return false;
	}

	// Load state
	if (!LoadApplicationState()) {
		return false;
	}

	// Validate skin
	if (!ValidateSkinAccess()) {
		return false;
	}

	// Initialize managers
	if (!InitializeManagers()) {
		return false;
	}

	// First run: enable autostart and show a brief tray tip
	if (m_windowManager && SettingsService::IsFirstRun()) {
		SettingsService::SetRunAtStartup(true);
		std::wstring title = Localization::LoadStringResource(m_hInstance, IDS_TRAY_TIP_TITLE);
		if (title.empty()) title = L"Tray Control";
		std::wstring message = Localization::LoadStringResource(m_hInstance, IDS_FIRST_RUN_TIP);
		if (message.empty()) message = L"You can control the app from the tray menu.";
		m_windowManager->ShowTrayNotification(title.c_str(), message.c_str());
		SettingsService::MarkFirstRunCompleted();
	}

	return true;
}

bool BongoCatApp::InitializeGdiPlus() {
	m_gdiPlusWrapper = std::make_unique<GdiPlusWrapper>();
	return m_gdiPlusWrapper && *m_gdiPlusWrapper;
}

bool BongoCatApp::LoadApplicationState() {
	StateService::LoadInitialState(m_state);
	return true;
}

bool BongoCatApp::ValidateSkinAccess() {
	StateService::ValidateSkinAccess(m_state);
	return true;
}

bool BongoCatApp::InitializeManagers() {
	// Create managers in dependency order
	m_imageManager = std::make_unique<ImageManager>(m_hInstance);
	m_windowManager = std::make_unique<WindowManager>(this);
	m_inputManager = std::make_unique<InputManager>(this);

	// Initialize managers
	if (!m_imageManager->Initialize(m_state->GetCurrentSkin())) {
		return false;
	}

	// Initialize via window manager which owns the window implementation
	if (!m_windowManager->Initialize()) {
		return false;
	}

	if (!m_inputManager->Initialize()) {
		return false;
	}

	return true;
}

int BongoCatApp::Run() {
	if (!m_hMainWindow) {
		return -1;
	}

	// Message loop
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return static_cast<int>(msg.wParam);
}

void BongoCatApp::Shutdown() {
	// Timers are managed by WindowManager

	// Cleanup managers
	if (m_inputManager) {
		m_inputManager->Shutdown();
		m_inputManager.reset();
	}
	if (m_windowManager) {
		m_windowManager->Shutdown();
		m_windowManager.reset();
	}
	if (m_imageManager) {
		m_imageManager->Cleanup();
		m_imageManager.reset();
	}

	// Cleanup GDI+
	m_gdiPlusWrapper.reset();
}

void BongoCatApp::OnInputEvent() {
	if (!m_hMainWindow) return;

	// If hidden, skip redraws but still count clicks
	if (m_state && !m_state->IsVisible()) {
		m_state->IncrementClickCount();
		// Avoid starting timers while hidden
		return;
	}

	RestartBlinkTimer();

	// Increment click count
	m_state->IncrementClickCount();

	HandleStateEventAndRedraw(StateEvent::InputReceived);

	StartImageSwitchTimer(Configuration::IMAGE_SWITCH_DELAY);
}

void BongoCatApp::RedrawCurrentImage() {
	if (!m_hMainWindow || !m_windowManager || !m_imageManager) return;
	if (m_state && !m_state->IsVisible()) return; // Skip redraws while hidden
	auto image = m_imageManager->GetImage(m_state->GetCurrentImageIndex());
	if (image) {
		m_windowManager->UpdateImage(m_hMainWindow, image);
	}
}

void BongoCatApp::HandleStateEventAndRedraw(StateEvent event) {
	m_state->GetStateMachine()->HandleEvent(event);
	RedrawCurrentImage();
}

void BongoCatApp::OnWindowDestroy() {
	// Persist state for next launch
	StateService::PersistOnExit(m_state);
	// Save current window position
	if (m_hMainWindow) {
		RECT rect{};
		if (GetWindowRect(m_hMainWindow, &rect)) {
			SettingsService::WriteWindowPosition(static_cast<int>(rect.left), static_cast<int>(rect.top));
		}
	}
	m_hMainWindow = nullptr;
}

void BongoCatApp::EnsureBlinkTimerRunning() {
	if (m_windowManager) m_windowManager->EnsureBlinkTimerRunning();
}

void BongoCatApp::RestartBlinkTimer() {
	if (m_windowManager) m_windowManager->RestartBlinkTimer();
}

void BongoCatApp::StartImageSwitchTimer(UINT delayMs) {
	if (m_windowManager) m_windowManager->StartImageSwitchTimer(delayMs);
}

void BongoCatApp::StopImageSwitchTimer() {
	if (m_windowManager) m_windowManager->StopImageSwitchTimer();
}

void BongoCatApp::StopAnimationTimers() {
	if (m_windowManager) m_windowManager->StopAnimationTimers();
}
