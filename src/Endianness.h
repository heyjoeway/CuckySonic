#pragma once
#ifdef BACKEND_SDL2
	#ifdef SWITCH
		#include <SDL_endian.h>
	#else
		#include "SDL_endian.h"
	#endif
	#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
		#define CPU_BIGENDIAN
	#else
		#define CPU_LILENDIAN
	#endif
#endif
