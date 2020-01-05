#pragma once

#include "GameMode.h"
#include "Background.h"
#include "Render.h"

class GM_Title : public GameMode
{
	private:
		TEXTURE *gTitleTexture;
		BACKGROUND *gTitleBackground;

		//Emblem and banner positions
		int gTitleEmblemX;
		int gTitleEmblemY;

		int gTitleBannerX;
		int gTitleBannerY;

		//Title state
		int gTitleYShift;
		int gTitleYSpeed;
		int gTitleYGoal;
		int gTitleFrame;

		int gTitleBackgroundScroll, gTitleBackgroundScrollSpeed;

		//Sonic's animation and position
		int gTitleSonicTime;

		int gTitleSonicX;
		int gTitleSonicY;

		int gTitleSonicXsp;
		int gTitleSonicYsp;

		int gTitleSonicFrame;
		int gTitleSonicHandFrame;
		int gTitleSonicAnimTimer;

		//Selection state
		bool gTitleSelected;

		bool ready = false;

	public:
        bool Loop(bool *bError);
        bool Init(bool *bError);
        bool Exit(bool *bError);
};

extern GM_Title GM_Title_Inst;