#include "WindowManager.h"
#include "../app/BongoCatApp.h"
#include "../utils/RAII/Window.h"
#include "../utils/RAII/Gdi.h"
#include "../utils/RAII/Menu.h"
#include "../states/CatStateMachine.h"
#include "../utils/RegistryUtils.h"
#include "../utils/SettingsService.h"
#include "../utils/ValidationUtils.h"
#include "../utils/SkinPresentation.h"
#include "../utils/SkinService.h"
#include "../utils/Configuration.h"
#include "../utils/Localization.h"
// Resource.h is supplied by the build system include paths
#if __has_include("Resource.h")
#include "Resource.h"
#elif __has_include("../../build/Resource.h")
#include "../../build/Resource.h"
#else
#error "Resource.h not found. Configure include paths to provide it."
#endif

// Global WndProc
extern LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

WindowManager::WindowManager(BongoCatApp* app)
	: m_app(app) {
}

WindowManager::~WindowManager() {}

ATOM WindowManager::RegisterWindowClass() {
	// Load icons and cursor
	m_appIcon = IconWrapper::LoadFromResource(m_app->GetInstance(), IDI_ICON);
	m_appIconSmall = IconWrapper::LoadFromResource(m_app->GetInstance(), IDI_ICON);
	m_appCursor = CursorWrapper::LoadSystemCursor(IDC_ARROW);

	WNDCLASSEXW wcex = { sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc; // Use global WndProc
	wcex.hInstance = m_app->GetInstance();
	wcex.hIcon = m_appIcon.get();
	wcex.hCursor = m_appCursor.get();
	wcex.hbrBackground = nullptr; // Layered windows don't use background brush
	wcex.lpszClassName = Configuration::WINDOW_CLASS_NAME;
	wcex.hIconSm = m_appIconSmall.get();
	return RegisterClassExW(&wcex);
}

bool WindowManager::CreateMainWindow() {
	// Determine initial window position: try registry, else default to bottom-right offset
	int x = 0;
	int y = 0;
	bool hasSavedPosition = SettingsService::ReadWindowPosition(x, y);

	if (!hasSavedPosition) {
		RECT workArea;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
		// Position using true margins from the bottom-right corner
		x = workArea.right - Configuration::IMAGE_RIGHT_MARGIN;
		y = workArea.bottom - Configuration::IMAGE_BOTTOM_MARGIN;
	}

	if (!m_app) return false;

	// Window title (app name is not localized)
	std::wstring windowTitle = Configuration::WINDOW_TITLE;

	HWND hWndMain = CreateWindowExW(
		WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
		Configuration::WINDOW_CLASS_NAME, windowTitle.c_str(), WS_POPUP,
		x, y, Configuration::IMAGE_WIDTH, Configuration::IMAGE_HEIGHT,
		nullptr, nullptr, m_app->GetInstance(), m_app
	);

	if (!hWndMain) return false;

	m_mainWindow = WindowWrapper(hWndMain, false);
	m_app->SetMainWindow(hWndMain);

	return m_mainWindow.get() != nullptr;
}

bool WindowManager::InitializeWindow() {
	if (!m_app || !m_app->GetMainWindow()) {
		return false;
	}
	if (!CreateGraphicsResources(m_app->GetMainWindow())) return false;
	if (!CreateTrayIcon()) return false;

	// Initialize timers
	if (!InitializeTimers()) return false;

	::ShowWindow(m_app->GetMainWindow(), SW_SHOW);
	m_app->RedrawCurrentImage();

	return true;
}

bool WindowManager::Initialize() {
	if (!RegisterWindowClass()) {
		return false;
	}

	if (!CreateMainWindow()) {
		return false;
	}

	if (!InitializeWindow()) {
		return false;
	}

	return true;
}

void WindowManager::Shutdown() {
	// Ensure timers are stopped before destroying the window
	StopAnimationTimers();
	if (m_mainWindow.get()) {
		DestroyWindow(m_mainWindow.get());
		m_mainWindow = WindowWrapper();
	}
	// Unregister window class for tidy shutdown
	if (m_app && m_app->GetInstance()) {
		UnregisterClassW(Configuration::WINDOW_CLASS_NAME, m_app->GetInstance());
	}
	// Cleanup merged resources
	DestroyTrayIcon();
	ZeroMemory(&m_nid, sizeof(m_nid));
	m_deviceContext = DeviceContextWrapper();
}

LRESULT WindowManager::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case Configuration::WM_APP_INPUT_EVENT:
		OnInputEvent();
		break;

	case Configuration::WM_TRAYICON:
		OnTrayIcon(lParam);
		break;

	case Configuration::WM_APP_SHOW_APP:
		// Use our app-visible method so timers/state are restored properly
		SetVisible(true);
		break;

	case WM_COMMAND:
		OnCommand(wParam);
		break;

	case WM_NCHITTEST:
		return HTCAPTION;

	case WM_TIMER:
		OnTimer(wParam);
		break;

	case WM_EXITSIZEMOVE: {
		// Persist position after move
		PersistWindowPosition();
		break;
	}

	case WM_QUERYENDSESSION:
		// Persist on shutdown/logoff
		PersistWindowPosition();
		if (m_app && m_app->GetState()) {
			SettingsService::WriteClickCount(m_app->GetState()->GetClickCount());
		}
		return TRUE;

	case WM_ENDSESSION:
		if (wParam) { // session is ending
			PersistWindowPosition();
			if (m_app && m_app->GetState()) {
				SettingsService::WriteClickCount(m_app->GetState()->GetClickCount());
			}
		}
		break;

	case WM_DESTROY:
		OnDestroy();
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void WindowManager::SetVisible(bool show) {
	if (!m_app || !m_app->GetMainWindow()) return;

	// OS window visibility
	::ShowWindow(m_app->GetMainWindow(), show ? SW_SHOW : SW_HIDE);

	// App visibility state
	if (m_app->GetState()) {
		m_app->GetState()->SetVisible(show);
	}

	// Timers by visibility
	if (show) {
		EnsureBlinkTimerRunning();
		StopImageSwitchTimer();
	}
	else {
		StopAnimationTimers();
	}

	// Reassert topmost when showing
	if (show) {
		::SetWindowPos(m_app->GetMainWindow(), HWND_TOPMOST, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		// Reset to Rest and redraw
		if (m_app) {
			m_app->HandleStateEventAndRedraw(StateEvent::TimerExpired);
		}
	}
}

bool WindowManager::IsWindowVisible() const {
	return m_app && m_app->GetMainWindow() && ::IsWindowVisible(m_app->GetMainWindow());
}

HWND WindowManager::GetMainWindow() const {
	return m_app ? m_app->GetMainWindow() : nullptr;
}

void WindowManager::OnInputEvent() {
	if (m_app) {
		m_app->OnInputEvent();
	}
}

void WindowManager::OnTimer(UINT_PTR timerId) {
	if (!m_app) return;
	if (timerId == Configuration::ID_BLINK_TIMER) {
		m_app->HandleStateEventAndRedraw(StateEvent::BlinkTimerExpired);
		StartImageSwitchTimer(Configuration::BLINK_DELAY);
	}
	else if (timerId == Configuration::ID_IMAGE_SWITCH_TIMER) {
		m_app->HandleStateEventAndRedraw(StateEvent::TimerExpired);
	}
	else if (timerId == Configuration::ID_TOPMOST_TIMER) {
		if (m_app->GetMainWindow()) {
			SetWindowPos(m_app->GetMainWindow(), HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}
}

void WindowManager::OnTrayIcon(LPARAM lParam) {
	if (!m_app) return;
	if (lParam == WM_LBUTTONUP) {
		SetVisible(!IsWindowVisible());
	}
	else if (lParam == WM_RBUTTONUP) {
		POINT pt; GetCursorPos(&pt);
		MenuWrapper menu(CreatePopupMenu(), true);
		std::wstring clicksFormat = Localization::LoadStringResource(m_app->GetInstance(), IDS_TRAY_CLICKS_FORMAT);
		if (clicksFormat.empty()) clicksFormat = L"Clicks: %d";
		std::wstring clicksText = Localization::FormatWide(clicksFormat.c_str(), m_app->GetState()->GetClickCount());
		AppendMenuW(menu.get(), MF_STRING | MF_GRAYED, Configuration::ID_TRAY_CLICKS, clicksText.c_str());
		AppendMenuW(menu.get(), MF_SEPARATOR, 0, nullptr);
		bool isVisible = IsWindowVisible();
		std::wstring showText = Localization::LoadStringResource(m_app->GetInstance(), IDS_TRAY_SHOW);
		if (showText.empty()) showText = L"Show";
		std::wstring hideText = Localization::LoadStringResource(m_app->GetInstance(), IDS_TRAY_HIDE);
		if (hideText.empty()) hideText = L"Hide";
		AppendMenuW(menu.get(), MF_STRING, Configuration::ID_TRAY_HIDE, isVisible ? hideText.c_str() : showText.c_str());
		std::wstring resetPosText = Localization::LoadStringResource(m_app->GetInstance(), IDS_TRAY_RESET_POSITION);
		if (resetPosText.empty()) resetPosText = L"Reset position";
		AppendMenuW(menu.get(), MF_STRING, Configuration::ID_TRAY_RESET_POSITION, resetPosText.c_str());
		MenuWrapper skinMenu(CreateSkinMenu(), false);
		std::wstring skinsText = Localization::LoadStringResource(m_app->GetInstance(), IDS_TRAY_SKINS);
		if (skinsText.empty()) skinsText = L"Skins";
		AppendMenuW(menu.get(), MF_POPUP, (UINT_PTR)skinMenu.get(), skinsText.c_str());
		bool startup = RegistryUtils::ValueExists(HKEY_CURRENT_USER, Configuration::AUTOSTART_KEY, Configuration::AUTOSTART_VALUE);
		std::wstring startupText = Localization::LoadStringResource(m_app->GetInstance(), IDS_TRAY_STARTUP);
		if (startupText.empty()) startupText = L"Run at startup";
		AppendMenuW(menu.get(), MF_STRING | (startup ? MF_CHECKED : 0), Configuration::ID_TRAY_STARTUP, startupText.c_str());
		AppendMenuW(menu.get(), MF_SEPARATOR, 0, nullptr);
		std::wstring closeText = Localization::LoadStringResource(m_app->GetInstance(), IDS_TRAY_CLOSE);
		if (closeText.empty()) closeText = L"Close";
		AppendMenuW(menu.get(), MF_STRING, Configuration::ID_TRAY_CLOSE, closeText.c_str());
		::SetForegroundWindow(m_app->GetMainWindow());
		::TrackPopupMenu(menu.get(), TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, m_app->GetMainWindow(), nullptr);
		::PostMessage(m_app->GetMainWindow(), WM_NULL, 0, 0);
	}
}

void WindowManager::OnCommand(WPARAM wParam) {
	if (!m_app) return;
	switch (LOWORD(wParam)) {
	case Configuration::ID_TRAY_CLOSE:
		DestroyWindow(m_app->GetMainWindow());
		break;
	case Configuration::ID_TRAY_STARTUP: {
		{
			bool enabled = SettingsService::IsRunAtStartupEnabled();
			SettingsService::SetRunAtStartup(!enabled);
		}
		break;
	}
	case Configuration::ID_TRAY_HIDE:
		SetVisible(!IsWindowVisible());
		break;
	case Configuration::ID_TRAY_RESET_POSITION: {
		// Move window to default bottom-right position
		RECT workArea{};
		SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
		int x = workArea.right - Configuration::IMAGE_RIGHT_MARGIN;
		int y = workArea.bottom - Configuration::IMAGE_BOTTOM_MARGIN;
		SetWindowPos(m_app->GetMainWindow(), nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		// Persist new position
		SettingsService::WriteWindowPosition(x, y);
		break;
	}
	case Configuration::ID_TRAY_SKIN_MARSHMALLOW:
	case Configuration::ID_TRAY_SKIN_MOCHI:
	case Configuration::ID_TRAY_SKIN_TOFFEE:
	case Configuration::ID_TRAY_SKIN_HONEY:
	case Configuration::ID_TRAY_SKIN_LATTE:
	case Configuration::ID_TRAY_SKIN_TREACLE: {
		int newSkin = LOWORD(wParam) - Configuration::ID_TRAY_SKIN_MARSHMALLOW;
		SkinService::ApplySkinChange(m_app, newSkin);
		break;
	}
	}
}

void WindowManager::OnDestroy() {
	// Stop timers tied to this window handle to avoid stray WM_TIMER
	StopAnimationTimers();
	if (m_app) {
		m_app->OnWindowDestroy();
	}
	// Remove tray icon before the window is fully destroyed to avoid ghost icons
	DestroyTrayIcon();
	// Ensure wrapper does not attempt to destroy an already destroyed window later
	m_mainWindow = WindowWrapper();
	PostQuitMessage(0);
}

void WindowManager::PersistWindowPosition() {
	if (!m_app || !m_app->GetMainWindow()) return;
	RECT rect{};
	if (GetWindowRect(m_app->GetMainWindow(), &rect)) {
		SettingsService::WriteWindowPosition(static_cast<int>(rect.left), static_cast<int>(rect.top));
	}
}

// ---- Drawing ----
bool WindowManager::CreateGraphicsResources(HWND hWnd) {
	ScreenDCWrapper screenDC;
	if (!screenDC.isValid()) return false;

	HDC hdcMem = CreateCompatibleDC(screenDC.get());
	if (!hdcMem) {
		CleanupGraphicsResources();
		return false;
	}

	m_deviceContext = DeviceContextWrapper(hdcMem, true);

	// No persistent DIB; select at draw time
	return true;
}

void WindowManager::CleanupGraphicsResources() {
	m_deviceContext = DeviceContextWrapper();
}

void WindowManager::UpdateImageInternal(HWND hWnd, HBITMAP image) {
	if (!image || !m_deviceContext.get()) return;

	ScreenDCWrapper screenDC;
	if (!screenDC.isValid()) return;

	SelectedObjectWrapper selectImage(m_deviceContext.get(), image);
	if (!selectImage.IsSelected()) return;

	RECT rect;
	GetWindowRect(hWnd, &rect);
	POINT ptPos = { rect.left, rect.top };
	SIZE sizeWnd = { Configuration::IMAGE_WIDTH, Configuration::IMAGE_HEIGHT };
	POINT ptSrc = { 0, 0 };
	BLENDFUNCTION blend = { AC_SRC_OVER, 0, Configuration::FULL_OPACITY, AC_SRC_ALPHA };

	UpdateLayeredWindow(hWnd, screenDC.get(), &ptPos, &sizeWnd,
		m_deviceContext.get(), &ptSrc, 0, &blend, ULW_ALPHA);
}

void WindowManager::UpdateImage(HWND windowHandle, HBITMAP imageHandle) {
	UpdateImageInternal(windowHandle, imageHandle);
}

// ---- Tray ----
bool WindowManager::CreateTrayIcon() {
	if (!m_app) return false;
	m_trayIcon = IconWrapper::LoadFromResource(m_app->GetInstance(), IDI_ICON);

	m_nid.cbSize = sizeof(m_nid);
	m_nid.hWnd = m_app->GetMainWindow();
	m_nid.uID = Configuration::TRAY_ICON_ID;
	m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	m_nid.uCallbackMessage = Configuration::WM_TRAYICON;
	m_nid.hIcon = m_trayIcon.get();

	{
		// Tooltip (app name is not localized)
		std::wstring tipString = Configuration::TRAY_TIP;
		size_t cap = sizeof(m_nid.szTip) / sizeof(m_nid.szTip[0]);
		size_t i = 0;
		for (; i + 1 < cap && i < tipString.size(); ++i) {
			m_nid.szTip[i] = tipString[i];
		}
		m_nid.szTip[i] = L'\0';
	}

	return Shell_NotifyIcon(NIM_ADD, &m_nid) == TRUE;
}

void WindowManager::DestroyTrayIcon() {
	if (m_nid.cbSize) {
		Shell_NotifyIcon(NIM_DELETE, &m_nid);
		ZeroMemory(&m_nid, sizeof(m_nid));
	}
}

void WindowManager::ShowTrayNotification(LPCWSTR title, LPCWSTR message, DWORD infoFlags, UINT timeoutMs) {
	if (!m_nid.cbSize) return;
	NOTIFYICONDATA nid = m_nid;
	nid.uFlags |= NIF_INFO;
	// Copy title
	{
		size_t cap = sizeof(nid.szInfoTitle) / sizeof(nid.szInfoTitle[0]);
		size_t i = 0;
		for (; i + 1 < cap && title && title[i] != L'\0'; ++i) {
			nid.szInfoTitle[i] = title[i];
		}
		nid.szInfoTitle[i] = L'\0';
	}
	// Copy message
	{
		size_t cap = sizeof(nid.szInfo) / sizeof(nid.szInfo[0]);
		size_t i = 0;
		for (; i + 1 < cap && message && message[i] != L'\0'; ++i) {
			nid.szInfo[i] = message[i];
		}
		nid.szInfo[i] = L'\0';
	}
	nid.dwInfoFlags = infoFlags;
	nid.uTimeout = timeoutMs; // supported by older shells; newer ignore and use default
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void WindowManager::AppendSkinItem(HMENU hSkinMenu, int skinId) {
	const wchar_t* skinName = SkinPresentation::GetSkinName(skinId);
	const int unlockThreshold = ValidationUtils::GetUnlockThreshold(skinId);

	wchar_t skinText[MAX_PATH] = { 0 };
	const bool unlocked = m_app->GetState()->GetClickCount() >= unlockThreshold;
	if (unlocked) {
		wcsncpy_s(skinText, skinName, _TRUNCATE);
		AppendMenuW(hSkinMenu, MF_STRING | (m_app->GetState()->GetCurrentSkin() == skinId ? MF_CHECKED : 0),
			Configuration::ID_TRAY_SKIN_MARSHMALLOW + skinId, skinText);
	}
	else {
		std::wstring lockedFormat = Localization::LoadStringResource(m_app->GetInstance(), IDS_SKIN_LOCKED_FORMAT);
		if (lockedFormat.empty()) lockedFormat = L"%s (%d clicks)";
		std::wstring lockedText = Localization::FormatWide(lockedFormat.c_str(), skinName, unlockThreshold);
		wcsncpy_s(skinText, lockedText.c_str(), _TRUNCATE);
		AppendMenuW(hSkinMenu, MF_STRING | MF_GRAYED, Configuration::ID_TRAY_SKIN_MARSHMALLOW + skinId, skinText);
	}
}

HMENU WindowManager::CreateSkinMenu() {
	HMENU hSkinMenu = CreatePopupMenu();
	for (int skinId = 0; skinId < Configuration::SKIN_COUNT; ++skinId) {
		AppendSkinItem(hSkinMenu, skinId);
	}
	return hSkinMenu;
}

// ---- Timers ----
bool WindowManager::InitializeTimers() {
	if (!m_app || !m_app->GetMainWindow()) return false;
	m_blinkTimer = std::make_unique<TimerWrapper>(m_app->GetMainWindow(), Configuration::ID_BLINK_TIMER);
	m_imageSwitchTimer = std::make_unique<TimerWrapper>(m_app->GetMainWindow(), Configuration::ID_IMAGE_SWITCH_TIMER);
	m_topmostTimer = std::make_unique<TimerWrapper>(m_app->GetMainWindow(), Configuration::ID_TOPMOST_TIMER);

	if (!m_blinkTimer->Set(Configuration::BLINK_INTERVAL)) return false;
	if (!m_topmostTimer->Set(Configuration::TOPMOST_TIMER_DELAY)) return false;
	return true;
}

void WindowManager::EnsureBlinkTimerRunning() {
	if (m_blinkTimer && !m_blinkTimer->IsActive()) {
		m_blinkTimer->Set(Configuration::BLINK_INTERVAL);
	}
}

void WindowManager::RestartBlinkTimer() {
	if (m_blinkTimer) {
		m_blinkTimer->Restart(Configuration::BLINK_INTERVAL);
	}
}

void WindowManager::StartImageSwitchTimer(UINT delayMs) {
	if (m_imageSwitchTimer) {
		m_imageSwitchTimer->Set(delayMs);
	}
}

void WindowManager::StopImageSwitchTimer() {
	if (m_imageSwitchTimer) {
		m_imageSwitchTimer->Kill();
	}
}

void WindowManager::StopAnimationTimers() {
	if (m_blinkTimer) m_blinkTimer->Kill();
	if (m_imageSwitchTimer) m_imageSwitchTimer->Kill();
	if (m_topmostTimer) m_topmostTimer->Kill();
}
