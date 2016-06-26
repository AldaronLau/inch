/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGR.c
 *	A High Level Graphics Library that supports sprites, texture loading,
 *	2D rendering & 3D rendering.
 */
#include "JLGRinternal.h"

static void jlgr_loop_(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	// Update events.
	jl_ct_loop__(jlgr);
	// Run any selected menubar items.
	jlgr_sprite_loop(jlgr, &jlgr->menubar.menubar);
	// Update mouse
	if(jlgr->mouse.mutex) jlgr_sprite_loop(jlgr, &jlgr->mouse);
	// Run Main Loop
	main_loop_(jl);
}

//
// Global Functions
//

/**
 * Create a window.
 * @param jl: The library context.
 * @param fullscreen: 0 for windowed mode, 1 for fullscreen.
 * @param fn_: Graphic initialization function run on graphical thread.
 * @returns The jlgr library context.
**/
jlgr_t* jlgr_init(jl_t* jl, uint8_t fullscreen, jl_fnct fn_) {
	jlgr_t* jlgr = jl_memi(jl, sizeof(jlgr_t));
	jlgr_thread_packet_t packet = { JLGR_COMM_INIT, 0, 0, fn_ };

	jl_print_function(jl, "JL/GR/INIT");
	jl->jlgr = jlgr;
	jl->loop = jlgr_loop_;
#if JL_PLAT == JL_PLAT_COMPUTER
	jlgr->wm.fullscreen = fullscreen;
#endif
	jlgr->jl = jl;
	jlgr->fl.inloop = 1;
	// Initialize Subsystem
	JL_PRINT_DEBUG(jl, "Initializing Input....");
	jl_ct_init__(jlgr); // Prepare to read input.
	JL_PRINT_DEBUG(jl, "Initialized CT! / Initializing file viewer....");
	jlgr_fl_init(jlgr);
	JL_PRINT_DEBUG(jl, "Initializing file viewer!");
	jl_print_return(jl, "JL/GR/INIT");
	// Create mutex for multi-threading
	jlgr->mutex = jl_thread_mutex_new(jl);
	jlgr->mutexs.usr_ctx = jl_thread_mutex_new(jl);
	// Create communicators for multi-threading
	jlgr->comm2draw = jl_thread_comm_make(jl,sizeof(jlgr_thread_packet_t));
	jl_thread_wait_init(jl, &jlgr->wait);
	// Start Drawing thread.
	jlgr_thread_init(jlgr);
	// Send graphical Init function
	jl_thread_comm_send(jl, jlgr->comm2draw, &packet);
	return jlgr;
}

/**
 * Set the functions to be called when the window redraws.
 * @param jlgr: The jlgr library context.
 * @param onescreen: The function to redraw the screen when there's only 1 
 *  screen.
 * @param upscreen: The function to redraw the upper or primary display.
 * @param downscreen: The function to redraw the lower or secondary display.
 * @param resize: The function called when window is resized.
**/
void jlgr_loop_set(jlgr_t* jlgr, jl_fnct onescreen, jl_fnct upscreen,
	jl_fnct downscreen, jl_fnct resize)
{
	// Wait for drawing thread to initialize, if not initialized already.
	jl_thread_wait(jlgr->jl, &jlgr->wait);
	//
	jl_fnct redraw[4] = { onescreen, upscreen, downscreen, resize };
	jlgr_thread_packet_t packet;
	int i;

	// 
	for(i = 0; i < 4; i++) {
		packet = (jlgr_thread_packet_t) {
			JLGR_COMM_SEND, i, 0, redraw[i]
		};
		jl_thread_comm_send(jlgr->jl, jlgr->comm2draw, &packet);
	}
}

/**
 * Resize the window.
 * @param jlgr: The library context.
**/
void jlgr_resz(jlgr_t* jlgr, uint16_t w, uint16_t h) {
	jlgr_thread_send(jlgr, JLGR_COMM_RESIZE, w, h, NULL);
}

/**
 * Destroy the window and free the jlgr library context.
 * @param jlgr: The jlgr library context.
**/
void jlgr_kill(jlgr_t* jlgr) {
	jlgr_thread_packet_t packet = { JLGR_COMM_KILL, 0, 0, NULL };

	JL_PRINT_DEBUG(jlgr->jl, "Sending Kill to threads....");
	jl_thread_comm_send(jlgr->jl, jlgr->comm2draw, &packet);
	JL_PRINT_DEBUG(jlgr->jl, "Waiting on threads....");
	jlgr_thread_kill(jlgr); // Shut down thread.
	JL_PRINT_DEBUG(jlgr->jl, "Threads are dead....");
	jlgr_file_kill_(jlgr); // Remove clump filelist for fileviewer.
	JL_PRINT_DEBUG(jlgr->jl, "Fileviewer is dead....");
}

// End of file.
