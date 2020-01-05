#include "Game.h"
#include "Log.h"
#include "Error.h"
#include "GM.h"

//Debug bool
bool gDebugEnabled = false;

//Score, time, rings, and lives
#define SCORE_REWARD 50000
#define RINGS_REWARD 100

#define RINGS_CAP 999
#define LIVES_CAP 99

#define INITIAL_LIVES 3

unsigned int gScore = 0;
unsigned int gNextScoreReward = SCORE_REWARD;

unsigned int gTime = 0;

unsigned int gRings = 0;
unsigned int gNextRingReward = RINGS_REWARD;

unsigned int gLives = INITIAL_LIVES;

//Generic game functions
void AddToScore(unsigned int score)
{
	//Increase score
	gScore += score;
	
	//Check for extra life rewards
	if (gScore >= gNextScoreReward)
	{
		//Increase lives, update our reward, and play jingle
		while (gScore >= gNextScoreReward)
		{
			gNextScoreReward += SCORE_REWARD;
			AddToLives(1);
		}
	}
}

void AddToRings(unsigned int rings)
{
	//Update ring reward (if we've lost a bunch of rings then lower it)
	if (gRings == 0)
		gNextRingReward = RINGS_REWARD;
	while (gRings + (RINGS_REWARD * 2) < gNextRingReward)
		gNextRingReward -= RINGS_REWARD;
	
	//Increase ring count and cap
	if (gRings >= RINGS_CAP - rings)
		gRings = RINGS_CAP;
	else
		gRings += rings;
	
	//Check for extra life rewards
	if (gRings >= gNextRingReward)
	{
		//Increase lives, update our reward, and play jingle
		while (gRings >= gNextRingReward)
		{
			AddToLives(1);
			gNextRingReward += RINGS_REWARD;
		}
	}
	else
	{
		//Play ring sound
		PlaySound(SOUNDID_RING);
	}
}

void AddToLives(unsigned int lives)
{
	//Increase lives and cap
	if (gLives >= LIVES_CAP - lives)
		gLives = LIVES_CAP;
	else
		gLives += lives;
}

void InitializeScores()
{
	gTime = 0;
	gRings = 0;
	gNextRingReward = RINGS_REWARD;
}

//Game loop
GAMEMODE gGameMode;

#include <emscripten.h>

GAMEMODE gGameModePrev;

bool IterateGameLoop(bool *bError) {
	bool bExit = false;

	if (gGameMode != gGameModePrev) {
		switch (gGameMode)
		{
			case GAMEMODE_SPLASH:
				bExit = GM_Splash_Init(bError);
				break;
			case GAMEMODE_TITLE:
				bExit = GM_Title_Init(bError);
				break;
			// case GAMEMODE_DEMO:
			// case GAMEMODE_GAME:
			// 	bExit = GM_Game_Init(bError);
			// 	break;
			// case GAMEMODE_SPECIALSTAGE:
			// 	bExit = GM_SpecialStage(&bError);
			// 	break;
		}
		gGameModePrev = gGameMode;
		if (bExit) return true;
	}

	switch (gGameMode)
	{
		case GAMEMODE_SPLASH:
			bExit = GM_Splash_Loop(bError);
			break;
		case GAMEMODE_TITLE:
			bExit = GM_Title_Loop(bError);
			break;
		case GAMEMODE_DEMO:
		case GAMEMODE_GAME:
			bExit = GM_Game_Loop(bError);
			break;
		// case GAMEMODE_SPECIALSTAGE:
		// 	GM_SpecialStage_Loop();
		// 	break;
	}

	return bExit;
}

#ifdef EMSCRIPTEN
bool bErrorEm = false;
void EmscriptenLoop() {
	IterateGameLoop(&bErrorEm);
}
#endif

bool EnterGameLoop()
{
	//Initialize game memory
	gGameMode = GAMEMODE_SPLASH; //Start at splash screen
	
	gScore = 0;
	gNextScoreReward = SCORE_REWARD;
	gTime = 0;
	gRings = 0;
	gNextRingReward = RINGS_REWARD;
	gLives = INITIAL_LIVES;
	
	emscripten_set_main_loop(EmscriptenLoop, 0, 1);

	// //Run game code
	bool bExit = false;
	bool bError = false;
	
	#ifndef EMSCRIPTEN
	while (!(bExit || bError))
		bExit = IterateGameLoop(&bError);
	}
	#endif
	
	//End game loop
	return bError;
}

