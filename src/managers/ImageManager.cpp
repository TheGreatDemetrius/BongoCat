#include "ImageManager.h"
#include <shlwapi.h>
#include <gdiplus.h>
#include <cstring>
#include "../utils/Configuration.h"
#include "../utils/RAII/Gdi.h"
#include "../utils/RAII/GdiPlus.h"

ImageManager::ImageManager(HINSTANCE hInstance)
	: m_hInstance(hInstance) {
}

ImageManager::~ImageManager() {
}

bool ImageManager::Initialize(int skinId) {
	return LoadImages(skinId);
}

void ImageManager::Cleanup() {
	m_images.clear();
	// Optionally release capacity eagerly to minimize peak memory during skin swaps
	m_images.shrink_to_fit();
}

HBITMAP ImageManager::LoadPNGFromResources(int resourceID) {
	// Use RAII wrapper for resource handle
	ResourceWrapper resourceWrapper(FindResourceW(m_hInstance, MAKEINTRESOURCEW(resourceID), L"PNG"));
	if (!resourceWrapper.isValid()) return nullptr;

	DWORD resourceSize = SizeofResource(m_hInstance, resourceWrapper.get());
	if (!resourceSize) return nullptr;

	// Use RAII wrapper for global resource handle
	GlobalResourceWrapper globalResourceWrapper(LoadResource(m_hInstance, resourceWrapper.get()), true);
	if (!globalResourceWrapper.isValid()) return nullptr;

	void* pResourceData = LockResource(globalResourceWrapper.get());
	if (!pResourceData) return nullptr;

	// IStream wrapper
	StreamWrapper streamWrapper(SHCreateMemStream(static_cast<const BYTE*>(pResourceData), resourceSize), true);
	if (!streamWrapper.isValid()) return nullptr;

	auto sourceBitmap = std::make_unique<Gdiplus::Bitmap>(streamWrapper.get());
	if (!sourceBitmap || sourceBitmap->GetLastStatus() != Gdiplus::Ok) return nullptr;

	// Destination bitmap with premultiplied alpha
	const INT dstWidth = Configuration::IMAGE_WIDTH;
	const INT dstHeight = Configuration::IMAGE_HEIGHT;
	auto destBitmap = std::make_unique<Gdiplus::Bitmap>(dstWidth, dstHeight, PixelFormat32bppPARGB);
	if (!destBitmap || destBitmap->GetLastStatus() != Gdiplus::Ok) return nullptr;

	// Render into destination once during load
	Gdiplus::Graphics g(destBitmap.get());
	g.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
	g.SetCompositingQuality(Gdiplus::CompositingQualityHighSpeed);
	g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
	g.SetSmoothingMode(Gdiplus::SmoothingModeNone);
	g.DrawImage(sourceBitmap.get(), 0, 0, dstWidth, dstHeight);

	// Lock bits
	Gdiplus::Rect rect(0, 0, dstWidth, dstHeight);
	Gdiplus::BitmapData data = {};
	if (destBitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppPARGB, &data) != Gdiplus::Ok) {
		return nullptr;
	}

	// Validate lock
	if (data.Scan0 == nullptr || data.Stride == 0 || rect.Width <= 0 || rect.Height <= 0) {
		destBitmap->UnlockBits(&data);
		return nullptr;
	}

	// Create top-down 32bpp DIB and copy pixels
	ScreenDCWrapper screenDC;
	if (!screenDC.isValid()) {
		destBitmap->UnlockBits(&data);
		return nullptr;
	}

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = rect.Width;
	bmi.bmiHeader.biHeight = -rect.Height; // top-down DIB
	bmi.bmiHeader.biPlanes = Configuration::PLANES_COUNT;
	bmi.bmiHeader.biBitCount = Configuration::BITS_PER_PIXEL; // 32 bpp
	bmi.bmiHeader.biCompression = BI_RGB;

	void* dibPixels = nullptr;
	HBITMAP hDib = CreateDIBSection(screenDC.get(), &bmi, DIB_RGB_COLORS, &dibPixels, nullptr, 0);
	if (!hDib || !dibPixels) {
		destBitmap->UnlockBits(&data);
		return nullptr;
	}

	// Copy respecting strides
	const BYTE* srcBase = static_cast<const BYTE*>(data.Scan0);
	BYTE* dstBase = static_cast<BYTE*>(dibPixels);
	const INT srcStrideSigned = data.Stride;
	const UINT dstStride = static_cast<UINT>(rect.Width * Configuration::BYTES_PER_PIXEL);

	if (srcBase == nullptr || dstBase == nullptr || dstStride == 0) {
		destBitmap->UnlockBits(&data);
		DeleteObject(hDib);
		return nullptr;
	}

	const UINT absSrcStride = static_cast<UINT>(srcStrideSigned >= 0 ? srcStrideSigned : -srcStrideSigned);
	const UINT rowCopyBytes = (absSrcStride < dstStride) ? absSrcStride : dstStride;

	for (INT y = 0; y < rect.Height; ++y) {
		const BYTE* srcRow = (srcStrideSigned >= 0)
			? srcBase + static_cast<size_t>(y) * static_cast<size_t>(absSrcStride)
			: srcBase + static_cast<size_t>(rect.Height - 1 - y) * static_cast<size_t>(absSrcStride);
		BYTE* dstRow = dstBase + static_cast<size_t>(y) * static_cast<size_t>(dstStride);
		if (rowCopyBytes > 0 && srcRow != nullptr && dstRow != nullptr) {
			memcpy(dstRow, srcRow, rowCopyBytes);
		}
	}

	destBitmap->UnlockBits(&data);
	return hDib;
}

bool ImageManager::LoadImages(int skinId) {
	// Validate skin ID using utility
	if (!ValidationUtils::IsValidSkin(skinId)) {
		return false;
	}

	// Cleanup existing images first
	Cleanup();

	int baseID = Configuration::SKIN_BASE_RESOURCE_ID + (skinId * Configuration::RESOURCES_PER_SKIN);

	for (int i = 0; i < Configuration::NUMBER_IMAGES; i++) {
		HBITMAP hbmp = LoadPNGFromResources(baseID + i);
		if (!hbmp) {
			// Cleanup any partially loaded images
			Cleanup();
			return false;
		}
		// Store RAII wrapper by value
		m_images.emplace_back(hbmp, true);
	}
	return true;
}

HBITMAP ImageManager::GetImage(int index) const {
	if (!ValidationUtils::IsValidImageIndex(index, static_cast<int>(m_images.size()))) {
		return nullptr;
	}
	return m_images[index].get();
}
