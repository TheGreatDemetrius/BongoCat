#pragma once
#include <windows.h>
#include "Base.h"

// GDI and UI deleters
struct DeviceContextDeleter {
	void operator()(HDC hdc) const {
		if (hdc) DeleteDC(hdc);
	}
};

struct BitmapDeleter {
	void operator()(HBITMAP hbmp) const {
		if (hbmp) DeleteObject(hbmp);
	}
};

struct IconDeleter {
	void operator()(HICON hIcon) const {
		if (hIcon) DestroyIcon(hIcon);
	}
};

struct CursorDeleter {
	void operator()(HCURSOR hCursor) const {
		// System cursors don't need destruction
	}
};

// GDI and UI wrappers
class DeviceContextWrapper : public BaseRAIIWrapper<HDC, DeviceContextDeleter> {
public:
	DeviceContextWrapper() : BaseRAIIWrapper(nullptr, false) {}
	DeviceContextWrapper(HDC hdc, bool owned = false)
		: BaseRAIIWrapper(hdc, owned) {
	}
};

class BitmapWrapper : public BaseRAIIWrapper<HBITMAP, BitmapDeleter> {
public:
	BitmapWrapper() : BaseRAIIWrapper(nullptr, false) {}
	BitmapWrapper(HBITMAP hbmp, bool owned = false)
		: BaseRAIIWrapper(hbmp, owned) {
	}
};

class IconWrapper : public BaseRAIIWrapper<HICON, IconDeleter> {
public:
	IconWrapper() : BaseRAIIWrapper(nullptr, false) {}
	IconWrapper(HICON hIcon, bool owned = false)
		: BaseRAIIWrapper(hIcon, owned) {
	}

	// Load from resources
	static IconWrapper LoadFromResource(HINSTANCE hInstance, LPCWSTR resourceName) {
		return IconWrapper(LoadIconW(hInstance, resourceName), false);
	}

	static IconWrapper LoadFromResource(HINSTANCE hInstance, int resourceId) {
		return IconWrapper(LoadIconW(hInstance, MAKEINTRESOURCEW(resourceId)), false);
	}
};

class CursorWrapper : public BaseRAIIWrapper<HCURSOR, CursorDeleter> {
public:
	CursorWrapper() : BaseRAIIWrapper(nullptr, false) {}
	CursorWrapper(HCURSOR hCursor, bool owned = false)
		: BaseRAIIWrapper(hCursor, owned) {
	}

	// Load system cursor
	static CursorWrapper LoadSystemCursor(LPCTSTR cursorName) {
		return CursorWrapper(LoadCursor(nullptr, cursorName), false);
	}
};

// Device context from GetDC/ReleaseDC
class ScreenDCWrapper {
private:
	HDC hdc_;
	HWND hwnd_;
	bool isScreenDC_;

public:
	ScreenDCWrapper(HWND hwnd = nullptr)
		: hdc_(nullptr), hwnd_(hwnd), isScreenDC_(hwnd == nullptr) {
		hdc_ = GetDC(hwnd);
	}

	~ScreenDCWrapper() {
		if (hdc_) {
			ReleaseDC(hwnd_, hdc_);
		}
	}

	// Non-copyable
	ScreenDCWrapper(const ScreenDCWrapper&) = delete;
	ScreenDCWrapper& operator=(const ScreenDCWrapper&) = delete;

	// Movable
	ScreenDCWrapper(ScreenDCWrapper&& other) noexcept
		: hdc_(other.hdc_), hwnd_(other.hwnd_), isScreenDC_(other.isScreenDC_) {
		other.hdc_ = nullptr;
		other.hwnd_ = nullptr;
		other.isScreenDC_ = false;
	}

	ScreenDCWrapper& operator=(ScreenDCWrapper&& other) noexcept {
		if (this != &other) {
			if (hdc_) {
				ReleaseDC(hwnd_, hdc_);
			}
			hdc_ = other.hdc_;
			hwnd_ = other.hwnd_;
			isScreenDC_ = other.isScreenDC_;
			other.hdc_ = nullptr;
			other.hwnd_ = nullptr;
			other.isScreenDC_ = false;
		}
		return *this;
	}

	// Accessors
	HDC get() const { return hdc_; }
	bool isValid() const { return hdc_ != nullptr; }
	operator HDC() const { return hdc_; }
	HWND getWindow() const { return hwnd_; }
	bool isScreenDC() const { return isScreenDC_; }
};

// Selected GDI object
class SelectedObjectWrapper {
private:
	HDC hdc_;
	HGDIOBJ hOldObject_;
	bool isSelected_;

public:
	SelectedObjectWrapper() : hdc_(nullptr), hOldObject_(nullptr), isSelected_(false) {}
	SelectedObjectWrapper(HDC hdc, HGDIOBJ hNewObject)
		: hdc_(hdc), hOldObject_(nullptr), isSelected_(false) {
		if (hdc && hNewObject) {
			HGDIOBJ previousObject = SelectObject(hdc, hNewObject);
			if (previousObject && previousObject != HGDI_ERROR) {
				hOldObject_ = previousObject;
				isSelected_ = true;
			}
			else {
				// Selection failed; leave as not selected so Restore() is a no-op
				isSelected_ = false;
				hOldObject_ = nullptr;
			}
		}
	}

	~SelectedObjectWrapper() {
		Restore();
	}

	// Delete copy constructor and assignment
	SelectedObjectWrapper(const SelectedObjectWrapper&) = delete;
	SelectedObjectWrapper& operator=(const SelectedObjectWrapper&) = delete;

	// Allow move semantics
	SelectedObjectWrapper(SelectedObjectWrapper&& other) noexcept
		: hdc_(other.hdc_), hOldObject_(other.hOldObject_), isSelected_(other.isSelected_) {
		other.hdc_ = nullptr;
		other.hOldObject_ = nullptr;
		other.isSelected_ = false;
	}

	SelectedObjectWrapper& operator=(SelectedObjectWrapper&& other) noexcept {
		if (this != &other) {
			Restore();
			hdc_ = other.hdc_;
			hOldObject_ = other.hOldObject_;
			isSelected_ = other.isSelected_;
			other.hdc_ = nullptr;
			other.hOldObject_ = nullptr;
			other.isSelected_ = false;
		}
		return *this;
	}

	void Restore() {
		if (isSelected_ && hdc_ && hOldObject_ && hOldObject_ != HGDI_ERROR) {
			SelectObject(hdc_, hOldObject_);
			isSelected_ = false;
		}
	}

	bool IsSelected() const { return isSelected_; }
	HGDIOBJ GetOldObject() const { return hOldObject_; }
};
