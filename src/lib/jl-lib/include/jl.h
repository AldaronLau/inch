/*
 * JL_lib(c) Jeron A. Lau
 * The header to be included within your programs you make with JL_lib.
*/

#ifndef JLL
#define JLL

#include <stdint.h>
#include "jl_me.h" // Simple CPU info

// Return Values
enum {
	JL_RTN_SUCCESS, // 0
	JL_RTN_FAIL, // 1
	JL_RTN_IMPOSSIBLE, // 2
	JL_RTN_SUPER_IMPOSSIBLE, // 3
	JL_RTN_COMPLETE_IMPOSSIBLE, // 4
	JL_RTN_FAIL_IN_FAIL_EXIT, // 5
} JL_RTN;

//ERROR MESSAGES
typedef enum{
	JL_ERR_NERR, //NO ERROR
	JL_ERR_NONE, //Something requested is Non-existant
	JL_ERR_FIND, //Can not find the thing requested
	JL_ERR_NULL, //Something requested is empty/null
}jl_err_t;

typedef enum{
	JL_THREAD_PP_AA, // Push if acceptable
	JL_THREAD_PP_UA, // Push if acceptable, & make unacceptable until pull. 
	JL_THREAD_PP_FF, // Push forcefully.
	JL_THREAD_PP_UF, // Push forcefully, and make unacceptable until pull
}jl_thread_pp_t;

#include "jl_ty.h" // Variable Types
#include "jl_ct.h" // Input Types
#include "clump.h" // LibClump
// Include libs
#include "JLgr.h"
#include "JLau.h"

#define JL_IMG_HEADER "JLVM0:JYMJ\0" // File format for images
//1=format,4=size,x=data
#define JL_IMG_SIZE_FLS 5 // How many bytes start for images.

void jl_dont(jl_t* jl);
void* jl_get_context(jl_t* jl);
int jl_start(jl_fnct fnc_init_, const char* name, uint64_t ctx_size);

// "JLmem.c"
void *jl_mem(jl_t* jl, void *a, uint32_t size);
void *jl_memi(jl_t* jl, uint32_t size);
void *jl_mem_copy(jl_t* jl, const void *src, uint64_t size);
uint64_t jl_mem_tbiu(void);
void jl_mem_leak_init(jl_t* jl);
void jl_mem_leak_fail(jl_t* jl, str_t fn_name);
void jl_mem_clr(void* mem, uint64_t size);
void jl_mem_copyto(const void* src, void* dst, uint64_t size);
void jl_mem_format(char* rtn, str_t format, ... );
uint32_t jl_mem_random_int(uint32_t a);
void *jl_mem_temp(jl_t* jl, void *mem);
double jl_mem_addwrange(double v1, double v2);
double jl_mem_difwrange(double v1, double v2);

// "JLdata_t.c"
void jl_data_clear(jl_t* jl, data_t* pa);
void jl_data_init(jl_t* jl, data_t* a, uint32_t size);
void jl_data_free(data_t* pstr);
void jl_data_mkfrom_str(data_t* a, str_t string);
void jl_data_mkfrom_data(jl_t* jl, data_t* a, uint32_t size, const void *data);
void jl_data_data(jl_t *jl, data_t* a, const data_t* b, uint64_t bytes);
void jl_data_merg(jl_t *jl, data_t* a, const data_t* b);
void jl_data_trunc(jl_t *jl, data_t* a, uint32_t size);
uint8_t jl_data_byte(data_t* pstr);
void jl_data_loadto(data_t* pstr, uint32_t varsize, void* var);
void jl_data_saveto(data_t* pstr, uint32_t varsize, const void* var);
void jl_data_add_byte(data_t* pstr, uint8_t pvalue);
void jl_data_delete_byte(jl_t *jl, data_t* pstr);
void jl_data_resize(jl_t *jl, data_t* pstr, uint32_t newsize);
void jl_data_insert_byte(jl_t *jl, data_t* pstr, uint8_t pvalue);
void jl_data_insert_data(jl_t *jl, data_t* pstr, const void* data, uint32_t size);
char* jl_data_tostring(jl_t* jl, data_t* a);
uint8_t jl_data_test_next(data_t* script, str_t particle);
void jl_data_read_upto(jl_t* jl, data_t* compiled, data_t* script, uint8_t end,
	uint32_t psize);

// "cl.c"
void jl_cl_list_alphabetize(struct cl_list *list);
void jl_clump_list_iterate(jl_t* jl, struct cl_list *list, jl_data_fnct fn);

// "JLmode.c"
void jl_mode_set(jl_t* jl, uint16_t mode, jl_mode_t loops);
void jl_mode_override(jl_t* jl, jl_mode_t loops);
void jl_mode_reset(jl_t* jl);
void jl_mode_switch(jl_t* jl, uint16_t mode);
void jl_mode_exit(jl_t* jl);

// "JLprint.c"
void jl_print_set(jl_t* jl, jl_print_fnt fn_);
void jl_print(jl_t* jl, str_t format, ... );
void jl_print_rewrite(jl_t* jl, const char* format, ... );
void jl_print_function(jl_t* jl, str_t fn_name);
void jl_print_return(jl_t* jl, str_t fn_name);
void jl_print_stacktrace(jl_t* jl);
#ifdef DEBUG
	#define JL_PRINT_DEBUG(jl, ...) jl_print(jl, __VA_ARGS__)
#else
	#define JL_PRINT_DEBUG(jl, ...)
#endif

// "JLfile.c"
void jl_file_print(jl_t* jl, str_t fname, str_t msg);
uint8_t jl_file_exist(jl_t* jl, str_t path);
void jl_file_rm(jl_t* jl, str_t filename);
void jl_file_save(jl_t* jl, const void *file, const char *name,
	uint32_t bytes);
void jl_file_load(jl_t* jl, data_t* load, str_t file_name);
char jl_file_pk_save(jl_t* jl, str_t packageFileName, str_t fileName,
	void *data, uint64_t dataSize);
char* jl_file_pk_compress(jl_t* jl, const char* folderName);
void jl_file_pk_load_fdata(jl_t* jl,data_t* rtn,data_t* data,str_t file_name);
void jl_file_pk_load(jl_t* jl, data_t* rtn, const char *packageFileName,
	const char *filename);
uint8_t jl_file_dir_mk(jl_t* jl, const char* path);
struct cl_list * jl_file_dir_ls(jl_t* jl,const char* dirname,uint8_t recursive);
str_t jl_file_get_resloc(jl_t* jl, str_t prg_folder, str_t fname);

// "JLthread.c"
uint8_t jl_thread_new(jl_t *jl, str_t name, SDL_ThreadFunction fn);
uint8_t jl_thread_current(jl_t *jl);
int32_t jl_thread_old(jl_t *jl, uint8_t threadnum);
SDL_mutex* jl_thread_mutex_new(jl_t *jl);
void jl_thread_mutex_lock(jl_t *jl, SDL_mutex* mutex);
void jl_thread_mutex_unlock(jl_t *jl, SDL_mutex* mutex);
void jl_thread_mutex_use(jl_t *jl, SDL_mutex* mutex, jl_fnct fn_);
void jl_thread_mutex_cpy(jl_t *jl, SDL_mutex* mutex, void* src,
	void* dst, uint32_t size);
void jl_thread_mutex_old(jl_t *jl, SDL_mutex* mutex);
jl_comm_t* jl_thread_comm_make(jl_t* jl, uint32_t size);
void jl_thread_comm_send(jl_t* jl, jl_comm_t* comm, const void* src);
void jl_thread_comm_recv(jl_t* jl, jl_comm_t* comm, jl_data_fnct fn);
void jl_thread_comm_kill(jl_t* jl, jl_comm_t* comm);
void jl_thread_pvar_init(jl_t* jl, jl_pvar_t* pvar, void* data, uint64_t size);
void jl_thread_pvar_push(jl_pvar_t* pvar, void* data, jl_thread_pp_t b);
void* jl_thread_pvar_edit(jl_pvar_t* pvar, void** data);
void jl_thread_pvar_pull(jl_pvar_t* pvar, void* data);
void jl_thread_pvar_free(jl_t* jl, jl_pvar_t* pvar);
void jl_thread_wait(jl_t* jl, jl_wait_t* wait);
void jl_thread_wait_init(jl_t* jl, jl_wait_t* wait);
void jl_thread_wait_stop(jl_t* jl, jl_wait_t* wait);

// "JLsdl.c"
double jl_sdl_timer(jl_t* jl, double* timer);

#endif
/*
 *	This a Jeron Lau project. JL_lib (c) 2014 
*/
