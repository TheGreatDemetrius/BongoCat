#pragma once
#include <windows.h>
#include <memory>
#include <atomic>
#include <vector>
#include "../utils/Configuration.h"
#include "../utils/RAII/GdiPlus.h"
#include "../states/ApplicationState.h"
// Uses concrete managers
class ImageManager;
class InputManager;
class WindowManager;

// Global window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

class BongoCatApp {
private:
	// Core application data
	HINSTANCE m_hInstance;
	std::unique_ptr<GdiPlusWrapper> m_gdiPlusWrapper;

	// State
	std::unique_ptr<ApplicationState> m_state;

	// Managers
	std::unique_ptr<ImageManager> m_imageManager;
	std::unique_ptr<InputManager> m_inputManager;
	// Window implementation
	std::unique_ptr<WindowManager> m_windowManager;

	// Window handle
	HWND m_hMainWindow;

	// Initialization
	bool InitializeGdiPlus();
	bool LoadApplicationState();
	bool ValidateSkinAccess();
	bool InitializeManagers();

public:
	BongoCatApp();
	~BongoCatApp();

	// Main
	bool Initialize(HINSTANCE hInstance);
	int Run();
	void Shutdown();

	// State
	ApplicationState* GetState() const noexcept { return m_state.get(); }

	// Accessors
	HINSTANCE GetInstance() const noexcept { return m_hInstance; }
	HWND GetMainWindow() const noexcept { return m_hMainWindow; }
	void SetMainWindow(HWND hWnd) noexcept { m_hMainWindow = hWnd; }

	// Manager accessors
	ImageManager* GetImageManager() const noexcept { return m_imageManager.get(); }
	InputManager* GetInputManager() const noexcept { return m_inputManager.get(); }
	WindowManager* GetWindowManager() const noexcept { return m_windowManager.get(); }

	// Events
	void OnInputEvent();

	void OnWindowDestroy();

	// Utility
	void RedrawCurrentImage();
	void HandleStateEventAndRedraw(StateEvent event);

	// Timer controls
	void EnsureBlinkTimerRunning();
	void RestartBlinkTimer();
	void StartImageSwitchTimer(UINT delayMs);
	void StopImageSwitchTimer();
	void StopAnimationTimers();
};
