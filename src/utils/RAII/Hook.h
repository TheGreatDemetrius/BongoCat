#pragma once
#include <windows.h>
#include "Base.h"

// Windows hook deleter
struct HookDeleter {
	void operator()(HHOOK hook) const {
		if (hook) UnhookWindowsHookEx(hook);
	}
};

// Windows hook wrapper
class HookWrapper : public BaseRAIIWrapper<HHOOK, HookDeleter> {
private:
	HINSTANCE hInstance_;
	int hookType_;
	HOOKPROC proc_;

public:
	HookWrapper(int hookType, HOOKPROC proc, HINSTANCE hInstance)
		: BaseRAIIWrapper(SetWindowsHookEx(hookType, proc, hInstance, 0), true)
		, hInstance_(hInstance), hookType_(hookType), proc_(proc) {
	}
};
