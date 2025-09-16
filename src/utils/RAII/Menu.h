#pragma once
#include <windows.h>
#include "Base.h"

// Menu deleter
struct MenuDeleter {
	void operator()(HMENU hMenu) const {
		if (hMenu) DestroyMenu(hMenu);
	}
};

// HMENU wrapper
class MenuWrapper : public BaseRAIIWrapper<HMENU, MenuDeleter> {
public:
	MenuWrapper() : BaseRAIIWrapper(nullptr, false) {}
	MenuWrapper(HMENU hMenu, bool owned = false)
		: BaseRAIIWrapper(hMenu, owned) {
	}
};
