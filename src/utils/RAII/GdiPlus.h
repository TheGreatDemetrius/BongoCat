#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <objidl.h>
#include "Base.h"

// GDI+ and related deleters
struct GdiPlusBitmapDeleter {
	void operator()(Gdiplus::Bitmap* bitmap) const {
		if (bitmap) delete bitmap;
	}
};

struct ResourceDeleter {
	void operator()(HRSRC /*hResource*/) const {
		// No explicit cleanup
	}
};

struct GlobalResourceDeleter {
	void operator()(HGLOBAL /*hGlobal*/) const {
		// Do not free HGLOBAL from LoadResource
	}
};

struct StreamDeleter {
	void operator()(IStream* pStream) const {
		if (pStream) pStream->Release();
	}
};

// GDI+ startup/shutdown
class GdiPlusWrapper {
private:
	ULONG_PTR token_;
	bool initialized_;

public:
	GdiPlusWrapper() : token_(0), initialized_(false) {
		Gdiplus::GdiplusStartupInput input;
		if (Gdiplus::GdiplusStartup(&token_, &input, nullptr) == Gdiplus::Ok) {
			initialized_ = true;
		}
	}

	~GdiPlusWrapper() {
		if (initialized_) {
			Gdiplus::GdiplusShutdown(token_);
		}
	}

	// Non-copyable
	GdiPlusWrapper(const GdiPlusWrapper&) = delete;
	GdiPlusWrapper& operator=(const GdiPlusWrapper&) = delete;

	// Movable
	GdiPlusWrapper(GdiPlusWrapper&& other) noexcept
		: token_(other.token_), initialized_(other.initialized_) {
		other.token_ = 0;
		other.initialized_ = false;
	}

	GdiPlusWrapper& operator=(GdiPlusWrapper&& other) noexcept {
		if (this != &other) {
			if (initialized_) {
				Gdiplus::GdiplusShutdown(token_);
			}
			token_ = other.token_;
			initialized_ = other.initialized_;
			other.token_ = 0;
			other.initialized_ = false;
		}
		return *this;
	}

	// Accessors
	bool isInitialized() const { return initialized_; }
	ULONG_PTR getToken() const { return token_; }
	operator bool() const { return initialized_; }
};

// Resource and stream wrappers
class ResourceWrapper : public BaseRAIIWrapper<HRSRC, ResourceDeleter> {
public:
	ResourceWrapper(HRSRC hResource)
		: BaseRAIIWrapper(hResource, false) {
	}
};

class GlobalResourceWrapper : public BaseRAIIWrapper<HGLOBAL, GlobalResourceDeleter> {
public:
	GlobalResourceWrapper(HGLOBAL hGlobal, bool owned = false)
		: BaseRAIIWrapper(hGlobal, owned) {
	}
};

class StreamWrapper : public BaseRAIIWrapper<IStream*, StreamDeleter> {
public:
	StreamWrapper(IStream* pStream, bool owned = false)
		: BaseRAIIWrapper(pStream, owned) {
	}
};
