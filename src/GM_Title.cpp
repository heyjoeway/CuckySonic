#include "GM_Title.h"

#include <math.h>

#include "Game.h"
#include "GameConstants.h"
#include "Error.h"
#include "Log.h"
#include "Event.h"
#include "Fade.h"
#include "MathUtil.h"
#include "Input.h"
#include "Audio.h"

//Title constants
enum TITLE_LAYERS
{
	TITLELAYER_MENU,
	TITLELAYER_BANNER,
	TITLELAYER_SONIC_HAND,
	TITLELAYER_SONIC,
	TITLELAYER_EMBLEM,
	TITLELAYER_BACKGROUND,
};

//Emblem and banner
const RECT titleEmblem = {0, 89, 256, 144};
const RECT titleBanner = {257, 106, 224, 74};
const int titleBannerJoin = 70;
const int titleBannerClipY = 10;

//Selection cursor
const RECT titleSelectionCursor[4] = {
	{257, 89, 8, 8},
	{266, 89, 8, 8},
	{275, 89, 8, 8},
	{266, 89, 8, 8},
};

//Sonic
const RECT titleSonicBody[4] = {
	{0,   0, 80, 72},
	{81,  0, 80, 88},
	{162, 0, 80, 80},
	{243, 0, 72, 80},
};

const struct
{
	RECT framerect;
	POINT jointPos;
} titleSonicHand[3] = {
	{{316, 0, 40, 40}, {36, 36}},
	{{357, 0, 32, 48}, {18, 40}},
	{{390, 0, 40, 48}, {16, 41}},
};

const int sonicHandAnim[14] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 1, 1};

//Text render function
void DrawText(TEXTURE *tex, const char *text, int x, int y)
{
	int dx = x;
	for (const char *current = text; *current != 0; current++)
	{
		RECT thisCharRect = {((*current - 0x20) % 0x20) * 8, 234 + ((*current - 0x20) / 0x20) * 8, 8, 8};
		gSoftwareBuffer->DrawTexture(tex, tex->loadedPalette, &thisCharRect, TITLELAYER_MENU, dx, y, false, false);
		dx += 8;
	}
}

//Background function
static const uint8_t scrollRipple[64] =
{
	1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0,
	2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3,
	1, 2, 1, 3, 1, 2, 2, 1, 2, 3, 1, 2, 1, 2, 0, 0,
	2, 0, 3, 2, 2, 3, 2, 2, 1, 3, 0, 0, 1, 0, 1, 3,
};

void TitleBackground(BACKGROUND *gTitleBackground, bool doScroll, int cameraX, int cameraY)
{
	(void)doScroll;
	(void)cameraY;
	
	//Handle palette cycle
	static int paletteTimer = 0;
	
	if (++paletteTimer >= 8)
	{
		paletteTimer = 0;
		
		COLOUR c9 = gTitleBackground->texture->loadedPalette->colour[0x9];
		COLOUR cA = gTitleBackground->texture->loadedPalette->colour[0xA];
		COLOUR cB = gTitleBackground->texture->loadedPalette->colour[0xB];
		COLOUR cC = gTitleBackground->texture->loadedPalette->colour[0xC];
		gTitleBackground->texture->loadedPalette->colour[0x9] = COLOUR(cC);
		gTitleBackground->texture->loadedPalette->colour[0xA] = COLOUR(c9);
		gTitleBackground->texture->loadedPalette->colour[0xB] = COLOUR(cA);
		gTitleBackground->texture->loadedPalette->colour[0xC] = COLOUR(cB);
	}
	//Get our scroll values
	int scrollBG1 = cameraX / 24;
	int scrollBG2 = cameraX / 32;
	int scrollBG3 = cameraX / 2;
	
	//Draw clouds
	static unsigned int cloudScroll = 0;
	(cloudScroll += 0x6) %= (gTitleBackground->texture->width * 0x10);
	
	RECT clouds = {0,  0, gTitleBackground->texture->width,  32};
	gTitleBackground->DrawStrip(&clouds, TITLELAYER_BACKGROUND,   0, -(scrollBG1 + cloudScroll / 0x10), -(scrollBG1 + cloudScroll / 0x10));
	
	//Draw sky and mountains
	RECT mountains = {0,  32, gTitleBackground->texture->width,  128};
	gTitleBackground->DrawStrip(&mountains, TITLELAYER_BACKGROUND,  32, -scrollBG2, -scrollBG2);
	
	//Draw ocean
	static unsigned int rippleFrame = 0, rippleTimer = 4;
	if (++rippleTimer >= 8)
	{
		rippleTimer = 0;
		rippleFrame++;
	}
	
	RECT strip = {0, 160, gTitleBackground->texture->width, 1};
	for (int i = 160; i < gTitleBackground->texture->height; i++)
	{
		int x = scrollBG2 + (scrollBG3 - scrollBG2) * (i - 160) / (gTitleBackground->texture->height - 160);
		x += scrollRipple[(i + rippleFrame) % 64] * (i - 160) / ((gTitleBackground->texture->height - 160) / 2);
		gTitleBackground->DrawStrip(&strip, TITLELAYER_BACKGROUND, strip.y++, -x, -x);
	}
	
	//Clear screen with sky behind gTitleBackground
	RECT backQuad = {0, 0, gRenderSpec.width, gRenderSpec.height};
	gSoftwareBuffer->DrawQuad(TITLELAYER_BACKGROUND, &backQuad, &gTitleBackground->texture->loadedPalette->colour[0]);
}

// ============================================================================

//Gamemode code
bool GM_Title::Init(bool *bError)
{
	//Load our title sheet and gTitleBackground
	gTitleTexture = new TEXTURE("data/Title.bmp");
	if (gTitleTexture->fail != nullptr)
		return (*bError = !Error(gTitleTexture->fail));
	
	gTitleBackground = new BACKGROUND("data/TitleBackground.bmp", &TitleBackground);
	if (gTitleBackground->fail != nullptr)
		return (*bError = !Error(gTitleBackground->fail));
	
	//Emblem and banner positions
	gTitleEmblemX = (gRenderSpec.width - titleEmblem.w) / 2;
	gTitleEmblemY = (gRenderSpec.height - titleEmblem.h) / 2;
	
	gTitleBannerX = (gRenderSpec.width - titleBanner.w) / 2;
	gTitleBannerY = gTitleEmblemY + titleBannerJoin;
	
	//Title state
	gTitleYShift = gRenderSpec.height * 0x100;
	gTitleYSpeed = -0x107E;
	gTitleYGoal = 0;
	gTitleFrame = 0;
	
	gTitleBackgroundScroll = 0, gTitleBackgroundScrollSpeed = 0;
	
	//Sonic's animation and position
	gTitleSonicTime = 54;
	
	gTitleSonicX = (gRenderSpec.width / 2) * 0x100;
	gTitleSonicY = (gTitleBannerY + 16) * 0x100;
	
	gTitleSonicXsp = -0x400;
	gTitleSonicYsp = -0x400;
	
	gTitleSonicFrame = 0;
	gTitleSonicHandFrame = 0;
	gTitleSonicAnimTimer = 0;
	
	//Selection state
	gTitleSelected = false;
	
	//Make our palette black for fade-in
	FillPaletteBlack(gTitleTexture->loadedPalette);
	FillPaletteBlack(gTitleBackground->texture->loadedPalette);

	ready = true;

	return false;
}

bool GM_Title::Loop(bool *bError)
{
	if (!ready) Init(bError);

	//Handle events
	bool bExit = HandleEvents();
	
	//Fade in/out
	bool bBreak = false;
	
	if (!gTitleSelected)
	{
		//Fade asset sheet and gTitleBackground palette in
		PaletteFadeInFromBlack(gTitleTexture->loadedPalette);
		PaletteFadeInFromBlack(gTitleBackground->texture->loadedPalette);
	}
	else
	{
		//Fade asset sheet and gTitleBackground palette out
		bool res1 = PaletteFadeOutToBlack(gTitleTexture->loadedPalette);
		bool res2 = PaletteFadeOutToBlack(gTitleBackground->texture->loadedPalette);
		bBreak = res1 && res2;
	}
	
	//Move title screen at beginning
	if (gTitleYShift >= gTitleYGoal && gTitleYSpeed >= 0)
	{
		gTitleYSpeed /= -2;
		gTitleYShift = gTitleYGoal;
	}
	else
	{
		gTitleYSpeed += 0x80;
		gTitleYShift += gTitleYSpeed;
	}
	
	//Render gTitleBackground
	gTitleBackground->Draw(true, gTitleBackgroundScroll, 0);
	
	//Render title screen banner and emblem
	gSoftwareBuffer->DrawTexture(gTitleTexture, gTitleTexture->loadedPalette, &titleEmblem, TITLELAYER_EMBLEM, gTitleEmblemX, gTitleEmblemY + gTitleYShift / 0x100, false, false);
	gSoftwareBuffer->DrawTexture(gTitleTexture, gTitleTexture->loadedPalette, &titleBanner, TITLELAYER_BANNER, gTitleBannerX, gTitleBannerY + gTitleYShift / 0x100, false, false);
	
	if (gTitleSonicTime-- <= 0)
	{
		//Clear timer (so there's no underflow)
		gTitleSonicTime = 0;
		
		//Move Sonic
		if ((gTitleSonicX += gTitleSonicXsp) > (gRenderSpec.width / 2) * 0x100)
			gTitleSonicX = (gRenderSpec.width / 2) * 0x100;
		else
			gTitleSonicXsp += 54;
			
		if ((gTitleSonicY += gTitleSonicYsp) < (gTitleBannerY - 70) * 0x100)
			gTitleSonicY = (gTitleBannerY - 70) * 0x100;
		else if ((gTitleSonicYsp += 24) > 0)
			gTitleSonicYsp = 0;
		
		//Animate Sonic
		if (gTitleSonicY < (gTitleBannerY - 40) * 0x100 && ++gTitleSonicAnimTimer >= 5)
		{
			//Reset timer and advance gTitleFrame
			gTitleSonicAnimTimer = 0;
			if (gTitleSonicFrame < 3)
				gTitleSonicFrame++;
		}
		
		//Render Sonic
		RECT bodyRect = titleSonicBody[gTitleSonicFrame];
		
		const int midX = gTitleSonicX / 0x100;
		const int topY = gTitleSonicY / 0x100;
		const int bottomY = (gTitleSonicY / 0x100) + bodyRect.h;
		const int clipY = gTitleBannerY + titleBannerClipY;
		
		if (topY < clipY)
		{
			if (bottomY > clipY)
				bodyRect.h -= (bottomY - clipY);
			gSoftwareBuffer->DrawTexture(gTitleTexture, gTitleTexture->loadedPalette, &bodyRect, TITLELAYER_SONIC, midX - 40, topY + gTitleYShift / 0x100, false, false);
		}
		
		//If animation is complete
		if (gTitleSonicFrame >= 3)
		{
			//Draw Sonic's hand
			int gTitleFrame = sonicHandAnim[gTitleSonicHandFrame];
			gSoftwareBuffer->DrawTexture(gTitleTexture, gTitleTexture->loadedPalette, &titleSonicHand[gTitleFrame].framerect, TITLELAYER_SONIC_HAND, midX + 20 - titleSonicHand[gTitleFrame].jointPos.x, topY + 72 - titleSonicHand[gTitleFrame].jointPos.y + gTitleYShift / 0x100, false, false);
			
			//Update gTitleFrame
			if (gTitleSonicHandFrame + 1 < 14)
			{
				gTitleSonicHandFrame++;
				gTitleBackgroundScrollSpeed++;
			}
			
			//Scroll gTitleBackground
			(gTitleBackgroundScroll += gTitleBackgroundScrollSpeed) %= gTitleBackground->texture->width * 96;
		}
	}
	
	//Handle selection and menus
	if (gController[0].press.a || gController[0].press.b || gController[0].press.c || gController[0].press.start)
		gTitleSelected = true;
	
	//Render our software buffer to the screen
	if ((*bError = gSoftwareBuffer->RenderToScreen(nullptr)) == true)
		bBreak = true;
	
	if (bExit || *bError)
		bBreak = true;

	if (bBreak) {
		Exit(bError);
		return false;
	}
	
	//Increment gTitleFrame
	gTitleFrame++;
	return false;
}

bool GM_Title::Exit(bool *bError)
{
	ready = false;
	gGameLoadLevel = 0;
	gGameLoadCharacter = 0;
	gGameMode = GAMEMODE_GAME;
	return false;
}

GM_Title GM_Title_Inst;