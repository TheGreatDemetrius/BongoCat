#include <windows.h>
#include <gdiplus.h>
#include <shlwapi.h>
#include "resource.h"
#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
using namespace Gdiplus;

constexpr int
NUMBER_IMAGES = 3,
IMAGE_WIDTH = 200,
IMAGE_HEIGHT = 160,
IMAGE_RIGHT_MARGIN = 450,
IMAGE_BOTTOM_MARGIN = 115,
TOPMOST_TIMER_DELAY = 50,
IMAGE_SWITCH_DELAY = 100,
IMAGE_REST = 0,
IMAGE_LEFT_PAW = 1,
IMAGE_RIGHT_PAW = 2,
ID_IMAGE_SWITCH_TIMER = 1,
ID_TOPMOST_TIMER = 2,
ID_TRAY_CLICKS = 1000,
ID_TRAY_RESET = 1001,
ID_TRAY_STARTUP = 1002,
ID_TRAY_CLOSE = 1003,
WM_APP_INPUT_EVENT = WM_APP + 1,
WM_TRAYICON = WM_USER + 1,
PLANES_COUNT = 1,
BITS_PER_PIXEL = 32;

BYTE FULL_OPACITY = 255;
HINSTANCE hInst;
ULONG_PTR gdiplusToken = 0;
HWND hWndMain;
HHOOK keyboardHook;
HHOOK mouseHook;
HDC hdcMem = nullptr;
HBITMAP hbmp = nullptr;
HBITMAP hOldBmp = nullptr;
NOTIFYICONDATA nid = {};
Bitmap* images[NUMBER_IMAGES] = { nullptr };

int currentImageIndex = 0;
int clickCount = 0;
int nextPaw = IMAGE_LEFT_PAW;

void ShowError(LPCWSTR text) {
	MessageBox(nullptr, text, L"Error", MB_ICONERROR);
}

bool IsAutoStartEnabled() {
	return RegGetValue(HKEY_CURRENT_USER,
		L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
		L"BongoCat",
		RRF_RT_ANY,
		nullptr,
		nullptr,
		0) == ERROR_SUCCESS;
}

void SetAutoStart(bool enable) {
	LPCWSTR keyPath = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	LPCWSTR valueName = L"BongoCat";
	if (enable) {
		WCHAR path[MAX_PATH];
		DWORD pathLength = GetModuleFileName(nullptr, path, MAX_PATH);
		DWORD dataSize = (pathLength + 1) * sizeof(WCHAR);
		LSTATUS result = RegSetKeyValue(HKEY_CURRENT_USER, keyPath, valueName, REG_SZ, path, dataSize);
	}
	else {
		RegDeleteKeyValue(HKEY_CURRENT_USER, keyPath, valueName);
	}
}

void SaveClickCount() {
	RegSetKeyValue(
		HKEY_CURRENT_USER,
		L"Software\\BongoCat",
		L"ClickCount",
		REG_DWORD,
		static_cast<const BYTE*>(static_cast<void*>(&clickCount)),
		sizeof(clickCount)
	);
}

void LoadClickCount() {
	DWORD count = 0;
	DWORD dataSize = sizeof(count);
	if (RegGetValue(HKEY_CURRENT_USER,
		L"Software\\BongoCat",
		L"ClickCount",
		RRF_RT_REG_DWORD,
		nullptr,
		&count,
		&dataSize) == ERROR_SUCCESS) {
		clickCount = count;
	}
}

void AddTrayIcon(HWND hWnd) {
	nid = {};
	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uID = 1;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.uCallbackMessage = WM_TRAYICON;
	nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));
	wcscpy_s(nid.szTip, L"Bongo Cat");
	Shell_NotifyIcon(NIM_ADD, &nid);
}

void ShowContextMenu(HWND hWnd) {
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	wchar_t clicksText[MAX_PATH];
	wsprintf(clicksText, L"Clicks: %d", clickCount);
	AppendMenu(hMenu, MF_STRING | MF_DISABLED, ID_TRAY_CLICKS, clicksText);
	AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
	AppendMenu(hMenu, MF_STRING, ID_TRAY_RESET, L"Reset counter");
	AppendMenu(hMenu, MF_STRING | (IsAutoStartEnabled() ? MF_CHECKED : 0), ID_TRAY_STARTUP, L"Run at startup");
	AppendMenu(hMenu, MF_SEPARATOR, 0, nullptr);
	AppendMenu(hMenu, MF_STRING, ID_TRAY_CLOSE, L"Close");
	SetForegroundWindow(hWnd);
	TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, nullptr);
	PostMessage(hWnd, WM_NULL, 0, 0);
	DestroyMenu(hMenu);
}

Bitmap* LoadPNGFromResources(int resourceID) {
	HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(resourceID), L"PNG");
	if (!hResource)
		return nullptr;

	DWORD resourceSize = SizeofResource(hInst, hResource);
	if (!resourceSize)
		return nullptr;

	HGLOBAL hGlobalRes = LoadResource(hInst, hResource);
	if (!hGlobalRes)
		return nullptr;

	void* pResourceData = LockResource(hGlobalRes);
	if (!pResourceData)
		return nullptr;

	IStream* pStream = SHCreateMemStream(static_cast<const BYTE*>(pResourceData), resourceSize);
	if (!pStream)
		return nullptr;

	Bitmap* bitmap = new Bitmap(pStream);
	Status status = bitmap->GetLastStatus();
	if (status != Ok) {
		delete bitmap;
		bitmap = nullptr;
	}
	pStream->Release();

	return bitmap;
}

void CleanTrayIcon() {
	if (nid.cbSize) {
		Shell_NotifyIcon(NIM_DELETE, &nid);
		if (nid.hIcon) {
			DestroyIcon(nid.hIcon);
			nid.hIcon = nullptr;
		}
		ZeroMemory(&nid, sizeof(nid));
	}
}

void CleanDrawingResources() {
	if (hdcMem) {
		if (hOldBmp) {
			SelectObject(hdcMem, hOldBmp);
			hOldBmp = nullptr;
		}
		DeleteDC(hdcMem);
		hdcMem = nullptr;
	}
	if (hbmp) {
		DeleteObject(hbmp);
		hbmp = nullptr;
	}
}

void CleanImages() {
	for (auto& image : images) {
		delete image;
		image = nullptr;
	}
}

void CleanInputHooks() {
	if (keyboardHook) {
		UnhookWindowsHookEx(keyboardHook);
		keyboardHook = nullptr;
	}
	if (mouseHook) {
		UnhookWindowsHookEx(mouseHook);
		mouseHook = nullptr;
	}
}

void CleanTimers() {
	if (hWndMain) {
		KillTimer(hWndMain, ID_TOPMOST_TIMER);
		KillTimer(hWndMain, ID_IMAGE_SWITCH_TIMER);
	}
}

void CleanGdiPlus() {
	if (gdiplusToken) {
		GdiplusShutdown(gdiplusToken);
		gdiplusToken = 0;
	}
}

bool LoadImages() {
	images[0] = LoadPNGFromResources(IDB_PNG1);
	images[1] = LoadPNGFromResources(IDB_PNG2);
	images[2] = LoadPNGFromResources(IDB_PNG3);
	if (!images[0] || !images[1] || !images[2]) {
		ShowError(L"Failed to load image resources");
		return false;
	}
	return true;
}

void UpdateWindowWithImage() {
	if (!images[currentImageIndex] || !hdcMem || !hbmp)
		return;

	HDC hdcScreen = GetDC(nullptr);
	if (!hdcScreen)
		return;
	Graphics graphics(hdcMem);
	graphics.Clear(Color(0, 0, 0, 0));
	graphics.DrawImage(images[currentImageIndex], 0, 0, IMAGE_WIDTH, IMAGE_HEIGHT);
	RECT rect;
	GetWindowRect(hWndMain, &rect);
	POINT ptPos = { rect.left, rect.top };
	SIZE sizeWnd = { IMAGE_WIDTH, IMAGE_HEIGHT };
	POINT ptSrc = { 0, 0 };
	BLENDFUNCTION blend = {
		AC_SRC_OVER, 0, 255, AC_SRC_ALPHA
	};
	UpdateLayeredWindow(
		hWndMain,
		hdcScreen,
		&ptPos,
		&sizeWnd,
		hdcMem,
		&ptSrc,
		0,
		&blend,
		ULW_ALPHA
	);

	ReleaseDC(nullptr, hdcScreen);
}

void SwitchImage(bool fromTimer = false) {
	if (fromTimer) {
		currentImageIndex = IMAGE_REST;
		UpdateWindowWithImage();
		return;
	}
	clickCount++;
	currentImageIndex = nextPaw;
	nextPaw = (nextPaw == IMAGE_LEFT_PAW) ? IMAGE_RIGHT_PAW : IMAGE_LEFT_PAW;

	UpdateWindowWithImage();
	KillTimer(hWndMain, ID_IMAGE_SWITCH_TIMER);
	SetTimer(hWndMain, ID_IMAGE_SWITCH_TIMER, IMAGE_SWITCH_DELAY, nullptr);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_APP_INPUT_EVENT:
		SwitchImage();
		break;

	case WM_TRAYICON:
		if (lParam == WM_LBUTTONUP) {
			ShowWindow(hWnd, IsWindowVisible(hWnd) ? SW_HIDE : SW_SHOW);
		}
		else if (lParam == WM_RBUTTONUP) {
			ShowContextMenu(hWnd);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_TRAY_CLOSE:
			DestroyWindow(hWnd);
			break;
		case ID_TRAY_STARTUP:
			SetAutoStart(!IsAutoStartEnabled());
			break;
		case ID_TRAY_RESET:
			clickCount = 0;
			SaveClickCount();
			break;
		}
		break;

	case WM_NCHITTEST:
		return HTCAPTION;

	case WM_TIMER:
		switch (wParam) {
		case ID_TOPMOST_TIMER:
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			break;
		case ID_IMAGE_SWITCH_TIMER:
			KillTimer(hWnd, ID_IMAGE_SWITCH_TIMER);
			SwitchImage(true);
			break;
		}
		break;

	case WM_DESTROY:
		SaveClickCount();
		CleanInputHooks();
		CleanTimers();
		CleanTrayIcon();
		CleanDrawingResources();
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

ATOM RegisterBongoClass(HINSTANCE hInstance) {
	WNDCLASSEXW wcex = { sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszClassName = L"BongoCatClass";
	wcex.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
		PostMessage(hWndMain, WM_APP_INPUT_EVENT, 0, 0);
	}
	return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= HC_ACTION && (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN || wParam == WM_MBUTTONDOWN)) {
		PostMessage(hWndMain, WM_APP_INPUT_EVENT, 0, 0);
	}
	return CallNextHookEx(mouseHook, nCode, wParam, lParam);
}

bool CreateBongoDrawingResources(HDC* phdcMem, HBITMAP* phbmp, HBITMAP* phOldBmp) {
	HDC hdcScreen = GetDC(nullptr);
	if (!hdcScreen) {
		ShowError(L"Failed to get screen DC");
		return false;
	}

	*phdcMem = CreateCompatibleDC(hdcScreen);
	if (!*phdcMem) {
		ShowError(L"Failed to create memory DC");
		ReleaseDC(nullptr, hdcScreen);
		return false;
	}

	BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER), IMAGE_WIDTH, -IMAGE_HEIGHT, PLANES_COUNT, BITS_PER_PIXEL };
	void* pBits = nullptr;
	*phbmp = CreateDIBSection(hdcScreen, &bmi, DIB_RGB_COLORS, &pBits, nullptr, 0);
	if (!*phbmp) {
		ShowError(L"Failed to create DIB section");
		ReleaseDC(nullptr, hdcScreen);
		DeleteDC(*phdcMem);
		*phdcMem = nullptr;
		return false;
	}

	*phOldBmp = (HBITMAP)SelectObject(*phdcMem, *phbmp);
	if (!*phOldBmp) {
		ShowError(L"Failed to select bitmap into memory DC");
		ReleaseDC(nullptr, hdcScreen);
		DeleteObject(*phbmp);
		*phbmp = nullptr;
		DeleteDC(*phdcMem);
		*phdcMem = nullptr;
		return false;
	}

	ReleaseDC(nullptr, hdcScreen);
	return true;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance;

	RECT workArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
	int x = workArea.right - IMAGE_RIGHT_MARGIN;
	int y = workArea.bottom - IMAGE_BOTTOM_MARGIN;

	hWndMain = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
		L"BongoCatClass", L"Bongo Cat", WS_POPUP,
		x, y, IMAGE_WIDTH, IMAGE_HEIGHT,
		nullptr, nullptr, hInstance, nullptr
	);

	if (!hWndMain) {
		ShowError(L"Failed to create main window");
		return false;
	}

	if (!CreateBongoDrawingResources(&hdcMem, &hbmp, &hOldBmp)) {
		CleanTrayIcon();
		CleanDrawingResources();
		DestroyWindow(hWndMain);
		return false;
	}
	AddTrayIcon(hWndMain);
	SetTimer(hWndMain, ID_TOPMOST_TIMER, TOPMOST_TIMER_DELAY, nullptr);
	ShowWindow(hWndMain, nCmdShow);
	UpdateWindowWithImage();
	return true;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	GdiplusStartupInput gdiplusStartupInput;

	if (!RegisterBongoClass(hInstance) ||
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr) != Ok ||
		!LoadImages() ||
		!InitInstance(hInstance, nCmdShow)) {
		CleanImages();
		CleanGdiPlus();
		return false;
	}

	keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
	mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hInstance, 0);
	if (!keyboardHook || !mouseHook) {
		ShowError(L"Failed to install input hooks");
		DestroyWindow(hWndMain);
		return false;
	}

	LoadClickCount();

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	CleanImages();
	CleanGdiPlus();
	return (int)msg.wParam;
}
