/** \file
 * jl_ty.h
 * 	Variable Types.
**/

#include "jl_sdl.h"

typedef const char chr_t;	// Character Constant
typedef const char* str_t;	// String Constant
typedef char m_chr_t;		// Character Modifiable
typedef char* m_str_t;		// String Modifiable

typedef struct{
	float x, y, z;
}jl_vec3_t;

//4 bytes of information about the string are included
typedef struct{
	uint8_t* data; //Actual String
	uint32_t size; //Allocated Space In String
	uint32_t curs; //Cursor In String
}data_t;

typedef struct{
	SDL_mutex *lock;	/** The mutex lock on the "data" */
	uint8_t pnum;		/** Number of packets in structure (upto 16 ) */
	uint32_t size;		/** Size of "data" */
	void* data[32];		/** The data attached to the mutex */
}jl_comm_t;

// Thread-Protected Variable
typedef struct{
	SDL_mutex *lock;	/** The mutex lock on the "data" */
	void* data;		/** The data attached to the mutex */
	uint64_t size;		/** Size of "data" */
	uint8_t acceptable;	/** Accepts push 0/1 **/
}jl_pvar_t;

// Thread-Wait Variable
typedef struct{
	SDL_atomic_t wait;
}jl_wait_t;

//Standard Mode Class
typedef struct {
	void* init;
	void* loop;
	void* kill;
}jl_mode_t;

// Thread-Specific context.
typedef struct{
	SDL_Thread* thread;
	SDL_threadID thread_id;

	struct {
		int8_t ofs2;
		char stack[50][30];
		uint8_t level;
	}print;

	void* temp_ptr;
}jl_ctx_t;

typedef struct{
	struct {
		uint8_t graphics; //graphics are enabled
		uint8_t fileviewer; //Fileviewer is enabled
		uint8_t filesys; // Filesystem is enabled.
		uint8_t input; // Input is enabled.
		uint8_t quickloop; // Quickloop is enabled
	}has;
	struct{
		void* printfn; // Function for printing
		uint8_t bkspc; // Backspace.
		SDL_mutex* mutex; // Mutex for printing to terminal
	}print;
	struct{
		double psec; // Seconds since last frame.
		double timer; // Time 1 frame ago started
	}time;
	struct {
		jl_mode_t *mdes; // Array Sizof Number Of Modes
		jl_mode_t mode; // Current Mode Data
		uint16_t which;
		uint16_t count;
	}mode;
	struct {
		struct{
			char* root; // The root directory "-- JL_Lib/"
			char* cprg; // The current program "-- JL_Lib/program"
			char* errf; // The error file "-- JL_Lib/errf.txt"
		}paths; // Paths to different files.

		data_t separator;
	}fl;
	void* loop; // The main loop.
	m_str_t name; // The name of the program.
	uint32_t info; // @startup:# images loaded from media.zip.Set by others.
	jl_err_t errf; // Set if error
	//
	uint8_t mode_switch_skip;
	//
	jl_ctx_t jl_ctx[16];
	// Program's context.
	void* prg_context;
	// Built-in library pointers.
	void* jlgr;
	void* jlau;
}jl_t;

typedef void(*jl_fnct)(jl_t* jl);
typedef void(*jl_data_fnct)(jl_t* jl, void* data);
typedef void(*jl_print_fnt)(jl_t* jl, const char * print);

//
