#include "SkinService.h"
#include "SettingsService.h"
#include "Configuration.h"
#include "../app/BongoCatApp.h"
#include "../managers/ImageManager.h"
#include "../managers/WindowManager.h"
#include "../states/ApplicationState.h"
#include "../states/CatStateMachine.h"

static void CommitSkinChangeInternal(BongoCatApp* app, int skinId) {
	if (!app || !app->GetState()) return;
	app->GetState()->SetCurrentSkin(skinId);
	SettingsService::WriteSkin(app->GetState()->GetCurrentSkin());
	app->GetState()->GetStateMachine()->HandleEvent(StateEvent::SkinChanged);
	app->RedrawCurrentImage();
}

void SkinService::ApplySkinChange(BongoCatApp* app, int newSkin) {
	if (!app || !app->GetState()) return;

	// Validate unlock first
	if (!app->GetState()->CanUnlockSkin(newSkin)) return;

	if (app->GetState()->GetCurrentSkin() == newSkin) return;

	// Try to load new skin images; fallback to MARSHMALLOW
	if (app->GetImageManager() && app->GetImageManager()->LoadImages(newSkin)) {
		CommitSkinChangeInternal(app, newSkin);
		return;
	}

	if (app->GetImageManager() && app->GetImageManager()->LoadImages(Configuration::SKIN_MARSHMALLOW)) {
		CommitSkinChangeInternal(app, Configuration::SKIN_MARSHMALLOW);
	}
}
