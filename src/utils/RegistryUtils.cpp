#include "RegistryUtils.h"
#include <cwchar>

bool RegistryUtils::EnsureKey(HKEY hKey, LPCWSTR subKey, REGSAM samDesired) {
	HKEY hCreated = nullptr;
	DWORD disposition = 0;
	LONG res = RegCreateKeyExW(
		hKey,
		subKey,
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		samDesired,
		nullptr,
		&hCreated,
		&disposition);
	if (hCreated) RegCloseKey(hCreated);
	return res == ERROR_SUCCESS;
}

bool RegistryUtils::SetDWordValue(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName, DWORD data) {
	if (!EnsureKey(hKey, subKey)) return false;
	return RegSetKeyValueW(hKey, subKey, valueName, REG_DWORD,
		reinterpret_cast<const BYTE*>(&data), sizeof(data)) == ERROR_SUCCESS;
}

bool RegistryUtils::GetDWordValue(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName, DWORD& data, DWORD defaultValue) {
	DWORD dataSize = sizeof(data);
	if (RegGetValueW(hKey, subKey, valueName, RRF_RT_REG_DWORD, nullptr, &data, &dataSize) == ERROR_SUCCESS) {
		return true;
	}
	data = defaultValue;
	return false;
}

bool RegistryUtils::SetStringValue(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName, LPCWSTR data) {
	if (!EnsureKey(hKey, subKey)) return false;
	return RegSetKeyValueW(hKey, subKey, valueName, REG_SZ,
		data, static_cast<DWORD>((wcslen(data) + 1) * sizeof(WCHAR))) == ERROR_SUCCESS;
}

bool RegistryUtils::DeleteValue(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName) {
	return RegDeleteKeyValueW(hKey, subKey, valueName) == ERROR_SUCCESS;
}

bool RegistryUtils::ValueExists(HKEY hKey, LPCWSTR subKey, LPCWSTR valueName) {
	return RegGetValueW(hKey, subKey, valueName, RRF_RT_ANY, nullptr, nullptr, nullptr) == ERROR_SUCCESS;
}
