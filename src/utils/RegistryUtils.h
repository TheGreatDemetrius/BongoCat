#pragma once
#include <windows.h>

// Registry operations
class RegistryUtils {
public:
	// Generic registry value operations
	static bool SetDWordValue(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName, DWORD data);
	static bool GetDWordValue(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName, DWORD& data, DWORD defaultValue = 0);
	static bool SetStringValue(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName, LPCWSTR data);
	static bool DeleteValue(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName);
	static bool ValueExists(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName);

	// Key creation/open helper
	static bool EnsureKey(HKEY hKey, LPCWSTR subKey, REGSAM samDesired = KEY_SET_VALUE | KEY_QUERY_VALUE);
};
