/*
 * me: memory manager
 * 
 * A simple memory library.  Includes creating variables, setting and
 * getting variables, and doing simple and complicated math functions on
 * the variables.  Has a specialized string type.
*/

#include "JLprivate.h"
#include <malloc.h>

/**
 * Return Amount Of Total Memory Being Used
 * @returns The total amount of memory being used in bytes.
**/
uint64_t jl_mem_tbiu(void) {
	struct mallinfo mi;

#if JL_PLAT != JL_PLAT_PHONE
	malloc_trim(0); //Remove Free Memory
#endif
	mi = mallinfo();
	return mi.uordblks;
}

/**
 * Start checking for memory leaks.  Pair up with 1 or multiple calls to
 * jl_mem_leak_fail() to check for memory leaks.
 * @param jl: The library context.
**/
void jl_mem_leak_init(jl_t* jl) {
	jl->info = jl_mem_tbiu();
}

/**
 * Exit if there's been a memory leak since the last call to jl_mem_leak_init().
 * @param jl: The library context.
 * @param fn_name: Recommended that it is the name of function that leak could
 * happen in, but can be whatever you want.
**/
void jl_mem_leak_fail(jl_t* jl, const char* fn_name) {
	if(jl_mem_tbiu() != jl->info) {
		jl_print(jl, "%s: Memory Leak Fail", fn_name);
		exit(-1);
	}
}

/**
 * Allocate, Resize, or Free Dynamic Memory.  All memory allocated by this
 * function is uninitialized.
 * To allocate dynamic memory:	void* memory = fn(jl, NULL, size);
 * To resize dynamic memory:	memory = fn(jl, memory, new_size);
 * To free dynamic memory:	memory = fn(jl, memory, 0);
 * @param jl: The library context.
 * @param a: Pointer to the memory to resize/free, or NULL if allocating memory.
 * @param size: # of bytes to resize to/allocate, or 0 to free.
**/
void *jl_mem(jl_t* jl, void *a, uint32_t size) {
	if(size == 0) { // Free
		if(a == NULL) {
			jl_print(jl, "Double Free or free on NULL pointer");
			exit(-1);
		}else{
			free(a);
		}
		return NULL;
	}else if(a == NULL) {
		return malloc(size);
	}else{ // Allocate or Resize
		if((a = realloc(a, size)) == NULL) {
			jl_print(jl, "realloc() failed! Out of memory?");
			exit(-1);
		}
	}
	return a;
}

/**
 * Allocate & Initialize Dynamic Memory.  All memory allocated by this function
 * is initialized as 0.
 * @param jl: The library context.
 * @param size: # of bytes to allocate.
**/
void *jl_memi(jl_t* jl, uint32_t size) {
	// Make sure size is non-zero.
	if(!size) {
		if(jl) jl_print(jl, "jl_memi(): size must be more than 0");
		else JL_PRINT("jl_memi(): size must be more than 0");
		exit(-1);
	}
	// Allocate Memory.
	void* a = jl_mem(jl, NULL, size);

	// Clear the memory.
	jl_mem_clr(a, size);
	// Return the memory
	return a;
}

/**
 * Clear memory pointed to by "mem" of size "size"
 * @param pmem: memory to clear
 * @param size: size of "mem"
**/
void jl_mem_clr(void* mem, uint64_t size) {
	memset(mem, 0, size);
}

/**
 * Copy memory from one place to another.
 * @param src: The place to copy memory from
 * @param dst: The place to copy memory to
 * @param size: The size of src & dst in bytes.
**/
void jl_mem_copyto(const void* src, void* dst, uint64_t size) {
	memcpy(dst, src, size);
}

/**
 * Copy "size" bytes of "src" to a new pointer of "size" bytes and return it.
 * @param jl: The library context.
 * @param src: source buffer
 * @param size: # of bytes of "src" to copy to "dst"
 * @returns: a new pointer to 
*/
void *jl_mem_copy(jl_t* jl, const void *src, uint64_t size) {
	void *dest = jl_memi(jl, size);
	jl_mem_copyto(src, dest, size);
	return dest;
}

/**
 * Format a string.
 * @param rtn: A variable to put the formated string.  It is assumed the size is
 *	80 bytes ( char rtn[80] )
 * @param format: The format string, can include %s, %f, %d, etc.
**/
void jl_mem_format(char* rtn, const char* format, ... ) {
	rtn[0] = '\0';
	if(format) {
		va_list arglist;

		va_start( arglist, format );
		vsnprintf( rtn, 80, format, arglist );
		va_end( arglist );
		//printf("done %s\n", (char*)rtn);
	}
}

/**
 * Generate a random integer from 0 to "a"
 * @param a: 1 more than the maximum # to return
 * @returns: a random integer from 0 to "a"
*/
uint32_t jl_mem_random_int(uint32_t a) {
	return rand()%a;
}

/**
 * Save up to 256 bytes to a buffer, return the previous value of the buffer.
 * @param jl: The library context.
 * @param mem: The new memory to save to the buffer.
 * @param size: Size of pointer.
 * @returns: The old/previous value of the pointer.
**/
void *jl_mem_temp(jl_t* jl, void *mem) {
	void* rtn = jl->jl_ctx[jl_thread_current(jl)].temp_ptr;

	jl->jl_ctx[jl_thread_current(jl)].temp_ptr = mem;
	return rtn;
}

/**
 * Add 2 Numbers, Keeping within a range of 0-1.
**/
double jl_mem_addwrange(double v1, double v2) {
	double rtn = v1 + v2;
	while(rtn < 0.) rtn += 1.;
	while(rtn > 1.) rtn -= 1.;
	return rtn;
}

/**
 * Find the smallest difference within a range of 0-1.
**/
double jl_mem_difwrange(double v1, double v2) {
	double rtn1 = fabs(jl_mem_addwrange(v1, -v2)); // Find 1 distance
	double rtn2 = 1. - rtn1; // Find complimentary distance
	if(rtn1 < rtn2)
		return rtn1;
	else
		return rtn2;
}

jl_t* jl_mem_init_(void) {
	jl_t* jl = jl_memi(NULL, sizeof(jl_t));
	//Prepare user data structure
	jl->errf = JL_ERR_NERR; // No error
	//Make sure that non-initialized things aren't used
	jl->has.graphics = 0;
	jl->has.fileviewer = 0;
	jl->has.filesys = 0;
	jl->has.input = 0;
	return jl;
}

void jl_mem_kill_(jl_t* jl) {
	free(jl);
//	cl_list_destroy(g_vmap_list);
}
