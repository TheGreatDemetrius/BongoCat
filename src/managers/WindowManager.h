#pragma once
#include <windows.h>
#include <memory>
#include <utility>
#include <shellapi.h>
#include "../utils/RAII/Window.h"
#include "../utils/RAII/Gdi.h"
#include "../utils/RAII/Timer.h"
// Tray and drawing are handled here

class BongoCatApp;

class WindowManager {
private:
	BongoCatApp* m_app;
	WindowWrapper m_mainWindow;
	// Tray
	NOTIFYICONDATA m_nid{};
	IconWrapper m_trayIcon;
	// Drawing
	DeviceContextWrapper m_deviceContext;
	IconWrapper m_appIcon;
	IconWrapper m_appIconSmall;
	CursorWrapper m_appCursor;
	// Timers
	std::unique_ptr<TimerWrapper> m_blinkTimer;
	std::unique_ptr<TimerWrapper> m_imageSwitchTimer;
	std::unique_ptr<TimerWrapper> m_topmostTimer;

	// Helper methods
	ATOM RegisterWindowClass();
	bool CreateMainWindow();
	bool InitializeWindow();
	void PersistWindowPosition();
	// Drawing helpers
	bool CreateGraphicsResources(HWND hWnd);
	void CleanupGraphicsResources();
	void UpdateImageInternal(HWND hWnd, HBITMAP image);
	// Tray helpers
	bool CreateTrayIcon();
	void DestroyTrayIcon();
	HMENU CreateSkinMenu();
	void AppendSkinItem(HMENU hSkinMenu, int skinId);
	// Timer helpers
	bool InitializeTimers();

public:
	WindowManager(BongoCatApp* app);
	~WindowManager();

	// Initialization and cleanup
	bool Initialize();
	void Shutdown();

	// Window procedure (called from global WndProc)
	LRESULT WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Window management
	void SetVisible(bool show);
	bool IsWindowVisible() const;
	HWND GetMainWindow() const;
	// Drawing
	void UpdateImage(HWND windowHandle, HBITMAP imageHandle);
	// Timer controls
	void EnsureBlinkTimerRunning();
	void RestartBlinkTimer();
	void StartImageSwitchTimer(UINT delayMs);
	void StopImageSwitchTimer();
	void StopAnimationTimers();

	// Notifications
	void ShowTrayNotification(LPCWSTR title, LPCWSTR message, DWORD infoFlags = NIIF_INFO, UINT timeoutMs = 5000);

	// Event handlers
	void OnInputEvent();
	void OnTimer(UINT_PTR timerId);
	void OnTrayIcon(LPARAM lParam);
	void OnCommand(WPARAM wParam);
	void OnDestroy();
};
