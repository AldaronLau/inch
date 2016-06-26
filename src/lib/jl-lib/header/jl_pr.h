// JL_LIB
	#include "jl.h"
// Standard Libraries
	#include <stdio.h>
	#include <dirent.h>
	#include <errno.h>

#define MAXFILELEN 1000 * 100000 //100,000 kb

//resolutions
#define JGR_STN 0 //standard 1280 by 960
#define JGR_LOW 1 //Low Res: 640 by 480
#define JGR_DSI 2 //DSi res: 256 by 192

#define VAR_POSITION 0
#define VAR_COLORS 1
#define VAR_TEXTUREI 2

// Printing
#if JL_PLAT == JL_PLAT_COMPUTER
	#define JL_PRINT(...) printf(__VA_ARGS__)
#else
	#define JL_PRINT(...) SDL_Log(__VA_ARGS__)
#endif

// Files
#define JL_FILE_SEPARATOR "/"
#define JL_ROOT_DIRNAME "JL_Lib"
#define JL_ROOT_DIR JL_ROOT_DIRNAME JL_FILE_SEPARATOR
#define JL_MAIN_DIR "PlopGrizzly_JLL"
#define JL_MAIN_MEF "media.zip"

// Replacement for NULL
#define STRT_NULL "(NULL)"
// target frames per second
#define JL_FPS 60

// Media To Include
void *jl_gem(void);
uint32_t jl_gem_size(void);

// Main - Prototypes
	char* jl_file_convert__(jl_t* jl, str_t filename);
	jl_ctx_t* jl_thread_get_safe__(jl_t* jl);
	void main_loop_(jl_t* jl);

	// LIB INITIALIZATION fn(Context)
	void jl_cm_init_(jl_t* jl);
	void jl_file_init_(jl_t * jl);
	jl_t* jl_mem_init_(void);
	void jl_print_init__(jl_t* jl);
	void jl_thread_init__(jl_t* jl);
	void jl_mode_init__(jl_t* jl);
	void jl_sdl_init__(jl_t* jl);

	// LIB KILLS
	void jl_mem_kill_(jl_t* jl);
	void jl_print_kill__(jl_t* jl);
	void jl_sdl_kill__(jl_t* jl);
	void jlau_kill(jlau_t* jlau);
	void jlgr_kill(jlgr_t* jlgr);

	// LIB THREAD INITS
	void jl_print_init_thread__(jl_t* jl, uint8_t thread_id);
