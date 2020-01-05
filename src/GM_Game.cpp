#include "GM_Game.h"

#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Input.h"
#include "Render.h"
#include "Fade.h"

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

int gGameLoadLevel = 0;
int gGameLoadCharacter = 0;
LEVEL *gLevel; // readiness of GM_Game implied by gLevel being nullptr

bool GM_Game::Init(bool *bError)
{
	//Load level with characters given
	gLevel = new LEVEL(gGameLoadLevel, characterSetList[gGameLoadCharacter]);
	if (gLevel->fail != nullptr)
		return (*bError = true);
	
	//Fade level from black
	gLevel->SetFade(true, false);

	return 0;
}

bool GM_Game::Loop(bool *bError)
{
	//Handle events
	bool bExit = HandleEvents();
	
	if (gLevel == nullptr) {
		Init(bError);
		return false;
	}

	//Update level
	if ((*bError = gLevel->Update()) == true) {
		Exit(bError);
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
	if ((*bError = gSoftwareBuffer->RenderToScreen(&gLevel->background->texture->loadedPalette->colour[0])) == true) {
		Exit(bError);
		return bExit;
	}

	if (bExit || *bError) {
		Exit(bError);
		return bExit;
	}
	
	// //Go to next state if set to break this state
	if (breakThisState) {
		Exit(bError);
		return bExit;
	}

	return bExit;
}

bool GM_Game::Exit(bool *bError)
{
	delete gLevel;
	gLevel = nullptr;

	return 0;
}

GM_Game GM_Game_Inst;