#include "Filesystem.h"
#include "Render.h"
#include "Audio.h"
#include "Input.h"
#include "Game.h"
#include "Error.h"
#include "Log.h"

//Backend usage
#ifdef BACKEND_SDL2
	#ifdef SWITCH
		#include <switch.h>
		#include <SDL.h>
	#else
		#include "SDL.h"
	#endif
	
	#define BACKEND_INIT	SDL_Init(SDL_INIT_EVERYTHING) < 0
	#define BACKEND_ERROR	SDL_GetError()
	#define BACKEND_QUIT	SDL_Quit()
#endif

#include "MathUtil.h"


// ============================================================================
// SWITCH DEBUGGING CODE
// ============================================================================

#ifdef ENABLE_NXLINK
#define TRACE(fmt,...) ((void)0)
static int s_nxlinkSock = -1;

static void initNxLink()
{
    if (R_FAILED(socketInitializeDefault()))
        return;

    s_nxlinkSock = nxlinkStdio();
    if (s_nxlinkSock >= 0)
        TRACE("printf output now goes to nxlink server");
    else
        socketExit();
}

static void deinitNxLink()
{
    if (s_nxlinkSock >= 0)
    {
        close(s_nxlinkSock);
        socketExit();
        s_nxlinkSock = -1;
    }
}

extern void userAppInit()
{
	initNxLink();
}

extern void userAppExit()
{
    deinitNxLink();
}
#endif

int main(int argc, char *argv[])
{
	(void)argc; (void)argv;
	
	//Initialize our backend
	LOG(("Initializing backend... "));
	
	if (BACKEND_INIT)
	{
		Error(BACKEND_ERROR);
		return -1; //Nothing to clean up, we can just return our failure
	}
	
	LOG(("Success!\n"));
	
	//Initialize game sub-systems
	bool error = false;
	if ((error = (InitializePath() || InitializeRender() || InitializeAudio() || InitializeInput())) == false)
		error = EnterGameLoop();
	
	//End game sub-systems
	QuitInput();
	QuitAudio();
	QuitRender();
	QuitPath();
	
	//Quit backend
	LOG(("Ending backend... "));
	BACKEND_QUIT;
	LOG(("Success!\n"));
	
	//Failed exit
	if (error)
		return -1;
	
	//Successful exit
	return 0;
}
