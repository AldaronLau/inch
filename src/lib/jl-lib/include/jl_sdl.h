#include "SDL_image.h"
#if JL_PLAT == JL_PLAT_COMPUTER
	#include "SDL.h"
#elif JL_PLAT == JL_PLAT_PHONE
	#include "SDL_test_common.h"
	#include "SDL_main.h"
#endif
