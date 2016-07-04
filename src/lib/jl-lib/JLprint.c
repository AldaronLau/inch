/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLprint.c
 *	is a library for printing to various terminals and creating a stack
 *	trace.
**/

#include "JLprivate.h"

static void _jl_print_current(jl_t *jl, uint8_t thread_id) {
	int i;

	JL_PRINT(" <");
	for(i = 0; i < jl->jl_ctx[thread_id].print.level; i++) {
		JL_PRINT("%s", jl->jl_ctx[thread_id].print.stack[i+1]);
		JL_PRINT("/");
	}
	JL_PRINT("\b>\n");
}

static void jl_print_indent__(jl_t *jl, int8_t o, uint8_t thread_id) {
	int i;
	// Print enough spaces for the open blocks.
	for(i = 0; i < jl->jl_ctx[thread_id].print.level + o; i++)
		JL_PRINT(" ");
}

static inline void _jl_print_new_block(jl_t *jl, uint8_t thread_id) {
	int i;
	uint8_t ofs2 = jl->jl_ctx[thread_id].print.ofs2;
	uint8_t level = jl->jl_ctx[thread_id].print.level;

	jl->jl_ctx[thread_id].print.ofs2 = 0;
	JL_PRINT("[");
	for(i = level - ofs2; i < level; i++) {
		JL_PRINT("/");
		JL_PRINT("%s", jl->jl_ctx[thread_id].print.stack[i+1]);
	}
	JL_PRINT("]");
	_jl_print_current(jl, thread_id);
}

static inline void _jl_print_old_block(jl_t *jl, uint8_t thread_id) {
	int i;
	uint8_t ofs2 = jl->jl_ctx[thread_id].print.ofs2;
	uint8_t level = jl->jl_ctx[thread_id].print.level;

	jl->jl_ctx[thread_id].print.ofs2 = 0;
	JL_PRINT("[\\");
	for(i = level; i > level + ofs2; i--) {
		JL_PRINT("%s", jl->jl_ctx[thread_id].print.stack[i+1]);
		JL_PRINT("\\");
	}
	JL_PRINT("\b]");
	_jl_print_current(jl, thread_id);
}

static inline void jl_print_descriptor_(jl_t* jl, uint8_t thread_id) {
	if(jl->jl_ctx[thread_id].print.ofs2 > 0) {
		jl_print_indent__(jl, -1, thread_id);
		_jl_print_new_block(jl, thread_id);
	}else if(jl->jl_ctx[thread_id].print.ofs2 < 0) {
		jl_print_indent__(jl, 0, thread_id);
		_jl_print_old_block(jl, thread_id);
	}
	jl_print_indent__(jl, 0, thread_id);
	JL_PRINT("[%s] ", jl->jl_ctx[thread_id].print.stack
		[jl->jl_ctx[thread_id].print.level]);
}

static void jl_print_test_overreach(jl_t* jl, uint8_t thread_id) {
	uint8_t level = jl->jl_ctx[thread_id].print.level;
	
	if(level > 49) {
		JL_PRINT("Overreached block count %d!!!\n", level);
		JL_PRINT("Quitting....\n");
		exit(0);
	}
}

static void jl_print_reset_print_descriptor_(jl_t* jl, uint8_t thread_id) {
	// Check to see if too many blocks are open.
	jl_print_test_overreach(jl, thread_id);
	// Print the print descriptor.
	jl_print_descriptor_(jl, thread_id);
}

static void jl_print_toconsole__(jl_t* jl, const char* input) {
	uint8_t thread_id = jl_thread_current(jl);
	int i = 0;
	char buffer[256];
	char buffer2[80];

	jl_mem_copyto(input, buffer, strlen(input) + 1);
	while(i != -1) {
		const char* text = buffer + (80 * i);
		// If string is empty; quit
		if((!text) || (!text[0])) break;
		// Clear and reset the print buffer
		jl_print_reset_print_descriptor_(jl, thread_id);
		// Print upto 80 characters to the terminal
		int chr_cnt = 73 - jl->jl_ctx[thread_id].print.level;
		char convert[10];
		if(strlen(text) > chr_cnt - 1) {
			i++;
		}else{
			chr_cnt = strlen(text);
			i = -1; // break
		}

		sprintf(convert, "%%%ds\n", chr_cnt);

		jl_mem_format(buffer2, convert, text);
		JL_PRINT("%s", buffer2);


		jl_file_print(jl, jl->fl.paths.errf, buffer2);
	}
}

//
// Exported Functions
//

/**
 * Set the function used to print out information.
 * @param jl: The library context.
 * @param fn_: The function to run when printing.  For default use NULL.
*/
void jl_print_set(jl_t* jl, jl_print_fnt fn_) {
	if(fn_) //User-Defined function
		jl->print.printfn = fn_;
	else //NULL
		jl->print.printfn = jl_print_toconsole__;
}

static void jl_print_function__(jl_t* jl,const char* fn_name,uint8_t thread_id){
//	uint8_t thread_id = jl_thread_current(jl);
	int size = strlen(fn_name);

	jl->jl_ctx[thread_id].print.level++;
	jl->jl_ctx[thread_id].print.ofs2++;
	jl_mem_copyto(fn_name, jl->jl_ctx[thread_id].print.stack
		[jl->jl_ctx[thread_id].print.level], size);
	jl->jl_ctx[thread_id].print.stack
		[jl->jl_ctx[thread_id].print.level][size] = '\0';

/*	jl->jl_ctx[thread_id].print.level++;*/
/*	jl->jl_ctx[thread_id].print.ofs2++;*/
/*	jl_mem_copyto(jl->jl_ctx[jl_thread_current(jl)].temp,*/
/*		jl->jl_ctx[thread_id].print.stack*/
/*			[jl->jl_ctx[thread_id].print.level], 255);*/
/*	i32_t size = strlen(jl->jl_ctx[thread_id].print.stack*/
/*			[jl->jl_ctx[thread_id].print.level]);*/
/*	jl->jl_ctx[thread_id].print.stack*/
/*		[jl->jl_ctx[thread_id].print.level][size] = '\0';*/
}

/**
 * Print text to the terminal.
 * @param jl: the library context.
 * @param format: what to print.
*/
void jl_print(jl_t* jl, const char* format, ... ) {
	char temp[256];
	jl_thread_mutex_lock(jl, jl->print.mutex);

	uint8_t thread_id = jl_thread_current(jl);
	jl_print_fnt print_out_ = jl->print.printfn;
	va_list arglist;

	// Store the format in jl->temp.
	va_start( arglist, format );
	vsprintf( temp, format, arglist );
	va_end( arglist );
	// Check to see if too many blocks are open.
	jl_print_test_overreach(jl, thread_id);
	// Print out.
	if(jl->print.bkspc) JL_PRINT("\r                                       "
		"                                         \r");
	print_out_(jl, temp);
	jl->print.bkspc = 0;
	jl_thread_mutex_unlock(jl, jl->print.mutex);
}

void jl_print_rewrite(jl_t* jl, const char* format, ... ) {
	char temp[256];
	char print[80];
	jl_thread_mutex_lock(jl, jl->print.mutex);

	uint8_t thread_id = jl_thread_current(jl);
	va_list arglist;

	// Store the format in jl->temp.
	va_start( arglist, format );
	vsprintf( temp, format, arglist );
	va_end( arglist );
	// Check to see if too many blocks are open.
	jl_print_test_overreach(jl, thread_id);
	// Print out.
	if(jl->print.bkspc) JL_PRINT("\r");
	jl_print_reset_print_descriptor_(jl, thread_id);
	memset(print, ' ', 79);
	print[79] = '\0';
	JL_PRINT("%s", temp);
	jl->print.bkspc = strlen(temp);
	jl_thread_mutex_unlock(jl, jl->print.mutex);
}

/**
 * Open a printing block.
 * @param jl: The library context.
 * @param fn_name: The name of the block.
**/
void jl_print_function(jl_t* jl, const char* fn_name) {
	uint8_t thread_id = jl_thread_current(jl);

	jl_thread_mutex_lock(jl, jl->print.mutex);
	jl_print_function__(jl, fn_name, thread_id);
	jl_thread_mutex_unlock(jl, jl->print.mutex);
}

/**
 * Close a printing block.
 * @param jl: The library context.
 * @param fn_name: The name of the block.
**/
void jl_print_return(jl_t* jl, const char* fn_name) {
	uint8_t thread_id = jl_thread_current(jl);

	jl_thread_mutex_lock(jl, jl->print.mutex);
	if(strcmp(fn_name, jl->jl_ctx[thread_id].print.stack
		[jl->jl_ctx[thread_id].print.level]))
	{
		jl_print(jl, "Error returning \"%s\" on thread #%d:\n",
			fn_name, thread_id);
		jl_print(jl, "\tFunction \"%s\" didn't return.",
			jl->jl_ctx[thread_id].print.stack[
				jl->jl_ctx[thread_id].print.level]);
		exit(-1);
	}
	jl_mem_clr(jl->jl_ctx[thread_id].print.stack
		[jl->jl_ctx[thread_id].print.level], 30);
	jl->jl_ctx[thread_id].print.level--;
	jl->jl_ctx[thread_id].print.ofs2 -= 1;
	jl_thread_mutex_unlock(jl, jl->print.mutex);
}

/**
 * Print out a stacktrace.
 * @param jl: The libary context.
**/
void jl_print_stacktrace(jl_t* jl) {
	uint8_t thread_id = jl_thread_current(jl);
	int i;

	jl_print(jl, "Stacktrace for thread #%d (Most Recent Call Last):",
		thread_id);
	jl_thread_mutex_lock(jl, jl->print.mutex);
	for(i = 0; i <= jl->jl_ctx[thread_id].print.level; i++) {
		jl_thread_mutex_unlock(jl, jl->print.mutex);
		jl_print(jl, jl->jl_ctx[thread_id].print.stack[i]);
		jl_thread_mutex_lock(jl, jl->print.mutex);
	}
	jl_thread_mutex_unlock(jl, jl->print.mutex);
}

void jl_print_init_thread__(jl_t* jl, uint8_t thread_id) {
	uint8_t i;

	for(i = 0; i < 50; i++) {
		jl_mem_clr(jl->jl_ctx[thread_id].print.stack[i], 30);
	}
	jl->jl_ctx[thread_id].print.level = 0;
	jl->jl_ctx[thread_id].print.ofs2 = 0;
	jl_print_function__(jl, "JL_Lib", thread_id);
}

void jl_print_init__(jl_t* jl) {
	jl->print.mutex = jl_thread_mutex_new(jl);
	jl_print_set(jl, NULL);
	jl_print_init_thread__(jl, 0);
}

void jl_print_kill__(jl_t * jl) {
	JL_PRINT_DEBUG(jl, "Killing printing....");
	jl_print_return(jl, "JL_Lib");
	JL_PRINT_DEBUG(jl, "Killed Printing!");
	jl_thread_mutex_old(jl, jl->print.mutex);
}
