#pragma once
#include <windows.h>

namespace Configuration {
	// ============================================================================
	// WINDOW CONFIGURATION
	// ============================================================================
	constexpr LPCWSTR WINDOW_CLASS_NAME = L"BongoCatClass";
	constexpr LPCWSTR WINDOW_TITLE = L"Bongo Cat";
	constexpr LPCWSTR TRAY_TIP = L"Bongo Cat";

	// ============================================================================
	// GRAPHICS CONFIGURATION
	// ============================================================================
	constexpr int IMAGE_WIDTH = 180;
	constexpr int IMAGE_HEIGHT = 116;
	constexpr int IMAGE_RIGHT_MARGIN = 450;
	constexpr int IMAGE_BOTTOM_MARGIN = 80;
	constexpr int PLANES_COUNT = 1;
	constexpr int BITS_PER_PIXEL = 32;
	constexpr int BYTES_PER_PIXEL = BITS_PER_PIXEL / 8;
	constexpr BYTE FULL_OPACITY = 255;
	constexpr int TRANSPARENT_RED = 0;
	constexpr int TRANSPARENT_GREEN = 0;
	constexpr int TRANSPARENT_BLUE = 0;
	constexpr int TRANSPARENT_ALPHA = 0;

	// ============================================================================
	// TIMER CONFIGURATION
	// ============================================================================
	constexpr int TOPMOST_TIMER_DELAY = 60;
	constexpr int IMAGE_SWITCH_DELAY = 150;
	constexpr int BLINK_INTERVAL = 8000;
	constexpr int BLINK_DELAY = 200;

	// Timer IDs
	constexpr int ID_IMAGE_SWITCH_TIMER = 1;
	constexpr int ID_TOPMOST_TIMER = 2;
	constexpr int ID_BLINK_TIMER = 3;

	// ============================================================================
	// MENU CONFIGURATION
	// ============================================================================
	// Tray menu IDs
	constexpr int ID_TRAY_CLICKS = 1000;
	constexpr int ID_TRAY_STARTUP = 1001;
	constexpr int ID_TRAY_CLOSE = 1002;
	constexpr int ID_TRAY_HIDE = 1003;
	constexpr int ID_TRAY_RESET_POSITION = 1004;

	// Tray skin menu IDs
	constexpr int ID_TRAY_SKIN_MARSHMALLOW = 2000;
	constexpr int ID_TRAY_SKIN_MOCHI = 2001;
	constexpr int ID_TRAY_SKIN_TOFFEE = 2002;
	constexpr int ID_TRAY_SKIN_HONEY = 2003;
	constexpr int ID_TRAY_SKIN_LATTE = 2004;
	constexpr int ID_TRAY_SKIN_TREACLE = 2005;

	// ============================================================================
	// DOMAIN CONSTANTS (merged from DomainConstants.h)
	// ============================================================================
	// Image counts and indices (domain-view of states/images)
	constexpr int NUMBER_IMAGES = 4;
	constexpr int IMAGE_REST = 0;
	constexpr int IMAGE_LEFT_PAW = 1;
	constexpr int IMAGE_RIGHT_PAW = 2;
	constexpr int IMAGE_BLINK = 3;

	// Skins (domain identifiers)
	constexpr int SKIN_COUNT = 6;
	constexpr int SKIN_MARSHMALLOW = 0;
	constexpr int SKIN_MOCHI = 1;
	constexpr int SKIN_TOFFEE = 2;
	constexpr int SKIN_HONEY = 3;
	constexpr int SKIN_LATTE = 4;
	constexpr int SKIN_TREACLE = 5;

	// Unlock thresholds
	constexpr int UNLOCK_MARSHMALLOW = 0;
	constexpr int UNLOCK_MOCHI = 100;
	constexpr int UNLOCK_TOFFEE = 1000;
	constexpr int UNLOCK_HONEY = 10000;
	constexpr int UNLOCK_LATTE = 100000;
	constexpr int UNLOCK_TREACLE = 1000000;

	// Unlock thresholds array for efficient lookup
	constexpr int UNLOCK_THRESHOLDS[SKIN_COUNT] = {
		UNLOCK_MARSHMALLOW, UNLOCK_MOCHI, UNLOCK_TOFFEE,
		UNLOCK_HONEY, UNLOCK_LATTE, UNLOCK_TREACLE
	};

	// ============================================================================
	// PLATFORM RESOURCE CONFIGURATION
	// ============================================================================
	// Skin resource base (platform resources)
	constexpr int SKIN_BASE_RESOURCE_ID = 101;
	constexpr int RESOURCES_PER_SKIN = NUMBER_IMAGES;

	// ============================================================================
	// INPUT CONFIGURATION
	// ============================================================================
	constexpr int INPUT_DEBOUNCE_TIME = 60;

	// ============================================================================
	// WINDOW MESSAGES
	// ============================================================================
	constexpr UINT WM_APP_INPUT_EVENT = WM_APP + 1;
	constexpr UINT WM_APP_SHOW_APP = WM_APP + 2;
	constexpr UINT WM_TRAYICON = WM_USER + 1;

	// ============================================================================
	// SYSTEM CONFIGURATION
	// ============================================================================
	constexpr int TRAY_ICON_ID = 1;
	constexpr LPCWSTR SINGLE_INSTANCE_MUTEX_NAME = L"Local\\BongoCat_SingleInstance";

	// Registry keys (platform)
	constexpr LPCWSTR REGISTRY_KEY = L"Software\\BongoCat";
	constexpr LPCWSTR AUTOSTART_KEY = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	constexpr LPCWSTR AUTOSTART_VALUE = L"BongoCat";
}
