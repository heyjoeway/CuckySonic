#pragma once

#include "GameMode.h"
#include "Render.h"

#define SPLASH_TIME 120

class GM_Splash : public GameMode {
    private:
        TEXTURE *gSplashTexture;
        bool gSplashdidPlayJingle;
        unsigned int gSplashFrame, gSplashAnimFrame;

    public:
        bool ready = false;
        bool Loop(bool *bError);
        bool Init(bool *bError);
        bool Exit(bool *bError);
};

extern GM_Splash GM_Splash_Inst;