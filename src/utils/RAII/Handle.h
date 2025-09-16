#pragma once
#include <windows.h>
#include "Base.h"

// Generic HANDLE deleter
struct HandleDeleter {
	void operator()(HANDLE handle) const {
		if (handle) CloseHandle(handle);
	}
};

// HANDLE wrapper
class HandleWrapper : public BaseRAIIWrapper<HANDLE, HandleDeleter> {
public:
	HandleWrapper() : BaseRAIIWrapper(nullptr, false) {}
	HandleWrapper(HANDLE handle, bool owned = false)
		: BaseRAIIWrapper(handle, owned) {
	}
};
