#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Input.h"
#include "Render.h"
#include "Fade.h"
#include "Level.h"

LEVEL *gLevel;

int gGameLoadLevel = 0;
int gGameLoadCharacter = 0;

static const char *sonicOnly[] =		{"data/Sonic/Sonic", nullptr};
static const char *sonicAndTails[] =	{"data/Sonic/Sonic", "data/Tails/Tails", nullptr};
static const char *tailsOnly[] =		{"data/Knuckles/Knuckles", nullptr};
static const char *knucklesOnly[] =		{"data/Knuckles/Knuckles", nullptr};

static const char **characterSetList[] = {
	sonicOnly,
	sonicAndTails,
	tailsOnly,
	knucklesOnly,
};

void GM_Game_Exit() {
	delete gLevel;
	gLevel = nullptr;
}

bool GM_Game_Init(bool *bError)
{
	//Load level with characters given
	gLevel = new LEVEL(gGameLoadLevel, characterSetList[gGameLoadCharacter]);
	if (gLevel->fail != nullptr)
		return (*bError = true);
	
	//Fade level from black
	gLevel->SetFade(true, false);

	return false;
}

bool GM_Game_Loop(bool *bError) {
	//Handle events
	bool bExit = HandleEvents();
	
	if (gLevel == nullptr) {
		GM_Game_Init(bError);
		return false;
	}

	//Update level
	if ((*bError = gLevel->Update()) == true) {
		GM_Game_Exit();
		return bExit;
	}
	
	//Handle level fading
	bool breakThisState = false;
	
	if (gLevel->fading)
	{
		if (gLevel->isFadingIn)
		{
			gLevel->fading = !gLevel->UpdateFade();
		}
		else
		{
			//Fade out and enter next game state
			if (gLevel->UpdateFade())
			{
				gGameMode = gLevel->specialFade ? GAMEMODE_SPECIALSTAGE : (gGameMode == GAMEMODE_DEMO ? GAMEMODE_SPLASH : GAMEMODE_GAME);
				breakThisState = true;
			}
		}
	}
	
	//Draw level to the screen
	gLevel->Draw();
	
	//Render our software buffer to the screen
	// if ((*bError = 
	gSoftwareBuffer->RenderToScreen(&gLevel->background->texture->loadedPalette->colour[0]);//) == true) {
	// 	GM_Game_Exit();
	// 	return bExit;
	// }

	// if (bExit || *bError) {
	// 	GM_Game_Exit();
	// 	return bExit;
	// }
	
	// //Go to next state if set to break this state
	if (breakThisState) {
		GM_Game_Exit();
		return bExit;
	}

	return bExit;
}