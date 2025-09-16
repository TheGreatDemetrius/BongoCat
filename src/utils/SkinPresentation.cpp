#include "SkinPresentation.h"
#include "Configuration.h"
#include "Localization.h"
#if __has_include("Resource.h")
#include "Resource.h"
#elif __has_include("../../build/Resource.h")
#include "../../build/Resource.h"
#endif

namespace SkinPresentation {
	const wchar_t* GetSkinName(int skin) {
		switch (skin) {
		case Configuration::SKIN_MARSHMALLOW:
			return L"Marshmallow";
		case Configuration::SKIN_MOCHI:
			return L"Mochi";
		case Configuration::SKIN_TOFFEE:
			return L"Toffee";
		case Configuration::SKIN_HONEY:
			return L"Honey";
		case Configuration::SKIN_LATTE:
			return L"Latte";
		case Configuration::SKIN_TREACLE:
			return L"Treacle";
		default:
			return L"Unknown";
		}
	}
}
