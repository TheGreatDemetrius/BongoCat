#pragma once
#include <windows.h>
#include <memory>
#include <vector>
#include "../utils/Configuration.h"
#include "../utils/RAII/Gdi.h"
#include "../utils/ValidationUtils.h"
// Concrete class; no interface indirection

class ImageManager {
private:
	HINSTANCE m_hInstance;
	std::vector<BitmapWrapper> m_images;

	// Helper methods
	HBITMAP LoadPNGFromResources(int resourceID);

public:
	ImageManager(HINSTANCE hInstance);
	~ImageManager();

	// Initialization and cleanup
	bool Initialize(int skinId);
	void Cleanup();

	// Image loading
	bool LoadImages(int skinId);

	// Image access
	HBITMAP GetImage(int index) const;
};
