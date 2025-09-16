#include "ValidationUtils.h"
#include "Configuration.h"
#include <climits>

bool ValidationUtils::CanUnlockSkin(int skin, int clickCount) {
	if (!IsValidSkin(skin)) {
		return false;
	}

	int threshold = GetUnlockThreshold(skin);
	return clickCount >= threshold;
}

int ValidationUtils::GetUnlockThreshold(int skin) {
	return (skin >= 0 && skin < Configuration::SKIN_COUNT) 
		? Configuration::UNLOCK_THRESHOLDS[skin] 
		: 0;
}

bool ValidationUtils::IsValidClickCount(int count) {
	return count >= 0 && count <= INT_MAX;
}

bool ValidationUtils::IsValidDWordRange(uint32_t value, uint32_t min, uint32_t max) {
	return value >= min && value <= max;
}

bool ValidationUtils::IsValidImageIndex(int index, int maxImages) {
	return index >= 0 && index < maxImages;
}
