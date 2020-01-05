#include "Game.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Render.h"
#include "Fade.h"
#include "MathUtil.h"
#include "Input.h"
#include "Audio.h"

#define SPLASH_TIME 120

TEXTURE gSplashTexture("data/Splash.bmp");
bool gSplashdidPlayJingle = false;
unsigned int gSplashFrame = 0, gSplashAnimFrame = 0;

bool GM_Splash_Loop(bool *bError) {
	//Handle events
	HandleEvents();
	
	//Handle fading
	if (gController[0].press.a || gController[0].press.b || gController[0].press.c || gController[0].press.start)
		gSplashFrame = mmax(gSplashFrame, SPLASH_TIME);
	
	bool bBreak = false;
	if (gSplashFrame < SPLASH_TIME)
	{
		//Fade in and play splash jingle once done
		if (PaletteFadeInFromWhite(gSplashTexture.loadedPalette))
		{
			if (!gSplashdidPlayJingle)
				PlaySound(SOUNDID_SPLASHJINGLE);
			gSplashdidPlayJingle = true;
		}
	}
	else if (PaletteFadeOutToBlack(gSplashTexture.loadedPalette))
		bBreak = true;
	
	//Draw splash
	RECT strip = {0, 0, gSplashTexture.width, 1};
	
	for (int y = 0; y < gRenderSpec.height; y++)
	{
		//Get our distortion
		int xOff;
		int inY = y - (gRenderSpec.height - gSplashTexture.height) / 2;
		
		xOff = GetSin((y) + (gSplashAnimFrame * 2)) * 15 / 0x100;
		inY += GetCos((y) + (gSplashAnimFrame * 4)) * 4 / 0x100;
		
		//Draw strip
		if (inY >= 0 && inY < gSplashTexture.height)
		{
			strip.y = inY;
			gSoftwareBuffer->DrawTexture(&gSplashTexture, gSplashTexture.loadedPalette, &strip, 0, (gRenderSpec.width - gSplashTexture.width) / 2 + xOff, y, false, false);
		}
	}
	
	//Render our software buffer to the screen (using the first colour of our splash texture, should be white)
	if ((*bError = gSoftwareBuffer->RenderToScreen(&gSplashTexture.loadedPalette->colour[0])) == true)
		bBreak = true;
	
	//Exit if faded out
	if (bBreak)
		gGameMode = GAMEMODE_TITLE;
	
	//Increment frame counter
	if (!(gController[0].held.left && gController[0].held.right))
		gSplashFrame++;
	gSplashAnimFrame++;

	return 0;
}

bool GM_Splash_Init(bool *bError)
{
	//Load our textures
	// gSplashTexture = TEXTURE("data/Splash.bmp");
	if (gSplashTexture.fail != nullptr)
		return (*bError = Error(gSplashTexture.fail));
	
	FillPaletteWhite(gSplashTexture.loadedPalette);
	
	gSplashdidPlayJingle = false;
	gSplashFrame = 0;
	gSplashAnimFrame = 0;
	
	return 0;
}
