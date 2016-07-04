/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRmode.c
 *	Handles things called modes.  An example is: your title screen
 *	of a game and the actual game would be on different modes.
**/
#include "JLprivate.h"

//
// Static functions.
//

static void _jl_mode_add(jl_t* jl) {
	// Allocate a new mode.
	jl->mode.mdes = jl_mem(jl, jl->mode.mdes,
		(jl->mode.count + 1) * sizeof(jl_mode_t));
	// Set the mode.
	jl->mode.mdes[jl->mode.count] =
		(jl_mode_t) { jl_dont, jl_dont, jl_mode_exit };
	// Add to mode count.
	jl->mode.count++;
}

//
// Export Functions
//

/**
 * Set the loop functions for a mode.
 *
 * @param jl: The library context.
 * @param mode: The mode to change the loop functions of.
 * @param loops: Which loop to change.
 *	JL_MODE_INIT: Called when mode is switched in.
 *	JL_MODE_EXIT: Called when "Back Button" Is Pressed.  "Back Button" is:
 *		- 3DS/WiiU: Select
 *		- Android: Back
 *		- Computer: Escape
 *		The default is to quit the program.  If set to something else
 *		then the function will loop forever unless interupted by a
 *		second press of the "Back Button" or unless the mode is changed.
 *	JL_MODE_LOOP: Called repeatedly.
 * @param loop: What to change the loop to.
*/
void jl_mode_set(jl_t* jl, uint16_t mode, jl_mode_t loops) {
	while(mode >= jl->mode.count) _jl_mode_add(jl);
	jl->mode.mdes[mode] = loops;
}

/**
 * Temporarily change the mode functions without actually changing the mode.
 * @param jl: The library context.
 * @param loops: the overriding functions.
 */
void jl_mode_override(jl_t* jl, jl_mode_t loops) {
	jl->mode.mode = loops;
}

/**
 * Reset any functions overwritten with jl_sg_mode_override().
 * @param jl: The library context.
 */
void jl_mode_reset(jl_t* jl) {
	jl->mode.mode = jl->mode.mdes[jl->mode.which];
}

/**
 * Switch which mode is in use.
 * @param jl: The library context.
 * @param mode: The mode to switch to.
 */
void jl_mode_switch(jl_t* jl, uint16_t mode) {
	if(jl->mode_switch_skip) return;

	jl_fnct kill_ = jl->mode.mode.kill;
	jl_fnct init_;

	// Run the previous mode's kill function
	jl->mode_switch_skip = 1;
	kill_(jl);
	jl->mode_switch_skip = 0;
	// Switch mode
	jl->mode.which = mode;
	// Update mode functions
	jl_mode_reset(jl);
	// Run the new mode's init functions.
	init_ = jl->mode.mode.init;
	init_(jl);
}

/**
 * Run the exit routine for the mode.  If the mode isn't switched in the exit
 *	routine, then the program will halt.
 * @param jl: The library context.
 */
void jl_mode_exit(jl_t* jl) {
	uint16_t which = jl->mode.which;
	jl_fnct kill_ = jl->mode.mode.kill;

	// Run exit routine.
	kill_(jl);
	// If mode is same as before, then quit.
	if(which == jl->mode.which) jl->mode.count = 0;
}

// Internal functions

void jl_mode_init__(jl_t* jl) {
	// Set up modes:
	jl->mode.which = 0;
	jl->mode.count = 0;
	jl->mode.mdes = NULL;
	_jl_mode_add(jl);
	// Clear User Loops
	jl_mode_override(jl, (jl_mode_t) { jl_dont, jl_dont, jl_dont });
}
