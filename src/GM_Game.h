#pragma once

#include "GameMode.h"
#include "Level.h"

extern int gGameLoadLevel;
extern int gGameLoadCharacter;
extern LEVEL *gLevel; // readiness of GM_Game implied by gLevel being nullptr

class GM_Game : public GameMode
{
	public:
        bool Loop(bool *bError);
        bool Init(bool *bError);
        bool Exit(bool *bError);
};

extern GM_Game GM_Game_Inst;