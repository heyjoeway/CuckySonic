#include "Filesystem.h"
#include "Render.h"
#include "Audio.h"
#include "Input.h"
#include "Game.h"
#include "Error.h"
#include "Log.h"

#ifdef SWITCH
	#include <switch.h>
#endif

//Backend usage
#ifdef BACKEND_SDL2
	#include "SDL.h"
	
	#define BACKEND_INIT	SDL_Init( \
		SDL_INIT_TIMER | \
		SDL_INIT_AUDIO | \
		SDL_INIT_VIDEO | \
		SDL_INIT_JOYSTICK | \
		SDL_INIT_GAMECONTROLLER | \
		SDL_INIT_EVENTS \
	) < 0
	#define BACKEND_ERROR	SDL_GetError()
	#define BACKEND_QUIT	SDL_Quit()
#endif

#include "MathUtil.h"


// ============================================================================
// SWITCH DEBUGGING CODE
// ============================================================================

int main(int argc, char *argv[])
{
	#ifdef ENABLE_NXLINK
	socketInitializeDefault();
	nxlinkStdio();
	#endif

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
	
	#ifdef ENABLE_NXLINK
    socketExit();
	#endif

	//Failed exit
	if (error)
		return -1;
	
	//Successful exit
	return 0;
}
