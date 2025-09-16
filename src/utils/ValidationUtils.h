#pragma once
#include <cstdint>

// Centralized validation
class ValidationUtils {
public:
	// Skin validation
	static inline bool IsValidSkin(int skin);
	static bool CanUnlockSkin(int skin, int clickCount);
	static int GetUnlockThreshold(int skin);

	// Click count validation
	static bool IsValidClickCount(int count);

	// Registry validation
	static bool IsValidDWordRange(uint32_t value, uint32_t min, uint32_t max);

	// Resource validation
	static bool IsValidImageIndex(int index, int maxImages);
};

// Inline implementations
#include "Configuration.h"

inline bool ValidationUtils::IsValidSkin(int skin) {
	return skin >= 0 && skin < Configuration::SKIN_COUNT;
}
