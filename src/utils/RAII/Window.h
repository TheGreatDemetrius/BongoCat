#pragma once
#include <windows.h>
#include "Base.h"

// Window handle deleter
struct WindowDeleter {
	void operator()(HWND hwnd) const {
		if (hwnd) DestroyWindow(hwnd);
	}
};

// Window handle wrapper
class WindowWrapper : public BaseRAIIWrapper<HWND, WindowDeleter> {
public:
	WindowWrapper() : BaseRAIIWrapper(nullptr, false) {}
	WindowWrapper(HWND hwnd, bool owned = false)
		: BaseRAIIWrapper(hwnd, owned) {
	}
};
