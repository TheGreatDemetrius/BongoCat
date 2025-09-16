#pragma once
#include <memory>

class BongoCatApp;

// Skin change orchestration
class SkinService {
public:
	// Validates unlock, loads images (fallback), persists, notifies, redraws
	static void ApplySkinChange(BongoCatApp* app, int newSkin);
};
