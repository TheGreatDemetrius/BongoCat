#include "SettingsService.h"
#include "Configuration.h"
#include "RegistryUtils.h"
#include "ValidationUtils.h"

int SettingsService::ReadClickCount() {
	DWORD clicks = 0;
	RegistryUtils::GetDWordValue(
		HKEY_CURRENT_USER,
		Configuration::REGISTRY_KEY,
		L"ClickCount",
		clicks,
		0);
	// Clamp to valid range
	if (!ValidationUtils::IsValidClickCount(static_cast<int>(clicks))) {
		clicks = 0;
	}
	return static_cast<int>(clicks);
}

void SettingsService::WriteClickCount(int count) {
	DWORD value = static_cast<DWORD>(count < 0 ? 0 : count);
	RegistryUtils::SetDWordValue(
		HKEY_CURRENT_USER,
		Configuration::REGISTRY_KEY,
		L"ClickCount",
		value);
}

int SettingsService::ReadSkin() {
	DWORD skin = 0;
	RegistryUtils::GetDWordValue(
		HKEY_CURRENT_USER,
		Configuration::REGISTRY_KEY,
		L"Skin",
		skin,
		0);
	return static_cast<int>(skin);
}

void SettingsService::WriteSkin(int skin) {
	RegistryUtils::SetDWordValue(
		HKEY_CURRENT_USER,
		Configuration::REGISTRY_KEY,
		L"Skin",
		static_cast<DWORD>(skin));
}

bool SettingsService::ReadWindowPosition(int& x, int& y) {
	DWORD dx = 0, dy = 0;
	bool okX = RegistryUtils::GetDWordValue(HKEY_CURRENT_USER, Configuration::REGISTRY_KEY, L"WindowPosX", dx, 0);
	bool okY = RegistryUtils::GetDWordValue(HKEY_CURRENT_USER, Configuration::REGISTRY_KEY, L"WindowPosY", dy, 0);
	if (okX && okY) {
		x = static_cast<int>(dx);
		y = static_cast<int>(dy);
		return true;
	}
	return false;
}

void SettingsService::WriteWindowPosition(int x, int y) {
	RegistryUtils::SetDWordValue(HKEY_CURRENT_USER, Configuration::REGISTRY_KEY, L"WindowPosX", static_cast<DWORD>(x));
	RegistryUtils::SetDWordValue(HKEY_CURRENT_USER, Configuration::REGISTRY_KEY, L"WindowPosY", static_cast<DWORD>(y));
}

bool SettingsService::IsRunAtStartupEnabled() {
	return RegistryUtils::ValueExists(HKEY_CURRENT_USER, Configuration::AUTOSTART_KEY, Configuration::AUTOSTART_VALUE);
}

bool SettingsService::SetRunAtStartup(bool enable) {
	if (enable) {
		WCHAR path[MAX_PATH];
		if (!GetModuleFileNameW(nullptr, path, MAX_PATH)) return false;
		WCHAR quoted[MAX_PATH * 2] = { 0 };
		wcscpy_s(quoted, L"\"");
		wcscat_s(quoted, path);
		wcscat_s(quoted, L"\"");
		return RegistryUtils::SetStringValue(HKEY_CURRENT_USER, Configuration::AUTOSTART_KEY, Configuration::AUTOSTART_VALUE, quoted);
	}
	return RegistryUtils::DeleteValue(HKEY_CURRENT_USER, Configuration::AUTOSTART_KEY, Configuration::AUTOSTART_VALUE);
}

bool SettingsService::IsFirstRun() {
	DWORD value = 0;
	// If the value is missing, default 0 indicates first run
	RegistryUtils::GetDWordValue(HKEY_CURRENT_USER, Configuration::REGISTRY_KEY, L"FirstRunDone", value, 0);
	return value == 0;
}

void SettingsService::MarkFirstRunCompleted() {
	RegistryUtils::SetDWordValue(HKEY_CURRENT_USER, Configuration::REGISTRY_KEY, L"FirstRunDone", 1);
}
