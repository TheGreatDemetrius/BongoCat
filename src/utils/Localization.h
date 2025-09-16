#pragma once
#include <windows.h>
#include <string>
#include <cstdarg>
#include <cwchar>
#include <iterator>

namespace Localization {

	inline std::wstring LoadStringResource(HINSTANCE instanceHandle, UINT stringId) {
		// Try first with a reasonably sized automatic buffer
		wchar_t stackBuffer[256] = { 0 };
		int length = ::LoadStringW(instanceHandle, stringId, stackBuffer, static_cast<int>(std::size(stackBuffer)));
		if (length > 0 && length < static_cast<int>(std::size(stackBuffer))) {
			return std::wstring(stackBuffer, static_cast<size_t>(length));
		}
		// Fallback: query length by progressively growing the buffer
		int capacity = 512;
		std::wstring result;
		for (int attempts = 0; attempts < 4; ++attempts) {
			result.assign(static_cast<size_t>(capacity), L'\0');
			int written = ::LoadStringW(instanceHandle, stringId, &result[0], capacity);
			if (written > 0 && written < capacity) {
				result.resize(static_cast<size_t>(written));
				return result;
			}
			capacity *= 2;
		}
		return L"";
	}

	inline std::wstring FormatWide(const wchar_t* formatString, ...) {
		if (!formatString) return L"";
		va_list args;
		va_start(args, formatString);
		wchar_t buffer[512] = { 0 };
		int written = _vsnwprintf_s(buffer, std::size(buffer), _TRUNCATE, formatString, args);
		va_end(args);
		if (written < 0) {
			return L"";
		}
		return std::wstring(buffer);
	}
}
