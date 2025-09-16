#include <windows.h>
#include <gdiplus.h>
#include "BongoCatApp.h"
#include "../utils/RAII/Handle.h"
#include "../utils/Configuration.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	// Single-instance via mutex
	HandleWrapper singleInstance(CreateMutexW(nullptr, TRUE, Configuration::SINGLE_INSTANCE_MUTEX_NAME), true);
	if (singleInstance.get() && GetLastError() == ERROR_ALREADY_EXISTS) {
		// Activate existing instance
		HWND hExisting = FindWindowW(Configuration::WINDOW_CLASS_NAME, nullptr);
		if (hExisting) {
			::PostMessageW(hExisting, Configuration::WM_APP_SHOW_APP, 0, 0);
			::ShowWindow(hExisting, SW_SHOW);
			::SetWindowPos(hExisting, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			::SetForegroundWindow(hExisting);
		}
		return 0;
	}

	BongoCatApp app;

	if (!app.Initialize(hInstance)) {
		return 1; // Non-zero exit code on failure
	}

	int exitCode = app.Run();
	return exitCode;
}
