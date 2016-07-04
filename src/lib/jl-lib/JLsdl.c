#include "JLprivate.h"
#include "SDL.h"

#include <time.h>

#define JL_SDL_SUBSYSTEMS SDL_INIT_AUDIO|SDL_INIT_VIDEO

/**
 * Time things up to a second.
 * @param jl: The library context.
 * @param timer: Pointer to timer variable.
 * @returns: Seconds passed.
**/
double jl_sdl_timer(jl_t* jl, double* timer) {
	double prev_tick = *timer; // Temporarily Save Old Value

#ifdef SDL_TIMER
	*timer = SDL_GetTicks(); // Set New Value
	// milliseconds / 1000 to get seconds
	return ((double)(*timer - prev_tick)) / 1000.;
#else
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	double nanoseconds = (double)time.tv_nsec;
	*timer = nanoseconds * 0.000000001; // Convert Nanoseconds to Seconds.
	return jl_mem_difwrange(*timer, prev_tick); // Find difference In Seconds
#endif
}

// internal functions:

void jl_sdl_init__(jl_t* jl) {
	SDL_Init(JL_SDL_SUBSYSTEMS);
	jl->time.psec = 0.f;
}

void jl_sdl_kill__(jl_t* jl) {
	SDL_QuitSubSystem(JL_SDL_SUBSYSTEMS);
	SDL_Quit();
}
