#include "JLprivate.h"

//
// Internal Functions
//

static void jl_data_truncate_curs__(data_t* pstr) {
	if(pstr->curs > pstr->size) {
		pstr->curs = pstr->size;
	}
}

static void jl_data_increment(data_t* pstr, uint8_t incrementation) {
	pstr->curs += incrementation;
	jl_data_truncate_curs__(pstr);
}

//
// Exported Functions
//

/**
 * Clears an already existing string and resets it's cursor value.
 * @param pa: string to clear.
*/
void jl_data_clear(jl_t* jl, data_t* pa) {
	pa->curs = 0;
//	jl_data_resize(jl, pa, 0);
	jl_mem_clr(pa->data, pa->size + 1);
}

/**
 * Allocates a "strt" of size "size" and returns it.
 * @param jl: The library context.
 * @param a: The jl_data_t to initialize.
 * @param size: How many bytes/characters to allocate.
 * @returns: A new initialized "strt".
*/
void jl_data_init(jl_t* jl, data_t* a, uint32_t size) {
	a->data = jl_memi(jl, size+1);
	a->size = size;
	a->curs = 0;
}

/**
 * frees a "strt".
 * @param pstr: the "strt" to free
*/
void jl_data_free(data_t* pstr) {
	free(pstr->data);
}

/**
 *
*/
void jl_data_mkfrom_data(jl_t* jl, data_t* a, uint32_t size, const void *data) {
	jl_data_init(jl, a, size);
	jl_mem_copyto(data, a->data, size);
	a->data[size] = '\0'; // Null terminalte
}

/**
 * Converts "string" into a data_t* and returns it.
 * @param string: String to convert
 * @returns: new "strt" with same contents as "string".
*/
void jl_data_mkfrom_str(data_t* a, const char* string) {
	return jl_data_mkfrom_data(NULL, a, strlen(string), string);
}

/**
 * Returns the byte at the cursor of a "strt".
 * @param pstr: the string to read.
 * @returns: byte at the cursor of "pstr"
*/
uint8_t jl_data_byte(data_t* pstr) {
	jl_data_truncate_curs__(pstr);
	return pstr->data[pstr->curs];
}

/**
 * Get the byte at the cursor of "strt", and increment the cursor value
**/
uint8_t jl_data_get_byte(data_t* pstr) {
	uint8_t* area = ((void*)pstr->data) + pstr->curs;
	jl_data_increment(pstr, 1);
	return *area;
}

/**
 * Get data at the cursor of "pstr", and increment the cursor value.
 * @param pstr: the string to read.
 * @param varsize: the size of variable pointed to by "var" in bytes (1,2,4,8).
 * @param var: the variable to save the data to.
**/
void jl_data_loadto(data_t* pstr, uint32_t varsize, void* var) {
	void* area = ((void*)pstr->data) + pstr->curs;

	jl_mem_copyto(area, var, varsize);
	jl_data_increment(pstr, varsize);
}

/**
 * Add variable data at the cursor of "pstr", and increment the cursor value.
 * @param pstr: the string to read.
 * @param: pval: the integer to add to "pstr"
*/
void jl_data_saveto(data_t* pstr, uint32_t varsize, const void* var) {
	void* area = ((void*)pstr->data) + pstr->curs;

	jl_mem_copyto(var, area, varsize);
	jl_data_increment(pstr, varsize);
}

/**
 * Add a byte ( "pvalue" ) at the cursor in a string ( "pstr" ), then increment
 * the cursor value [ truncated to the string size ]
 * @param pstr: The string to add a byte to.
 * @param pvalue: the byte to add to "pstr".
*/
void jl_data_add_byte(data_t* pstr, uint8_t pvalue) {
	jl_data_truncate_curs__(pstr);
	pstr->data[pstr->curs] = pvalue;
	jl_data_increment(pstr, 1);
}

/**
 * Delete byte at cursor in string.
*/
void jl_data_delete_byte(jl_t *jl, data_t* pstr) {
	int i;
	
	if(pstr->size == 0) return;
	for(i = pstr->curs; i < pstr->size - 1; i++)
		pstr->data[i] = pstr->data[i+1];
	pstr->size--;
	pstr->data[pstr->size] = '\0';
	pstr->data = jl_mem(jl, pstr->data, pstr->size);
	jl_data_truncate_curs__(pstr);
}

void jl_data_resize(jl_t *jl, data_t* pstr, uint32_t newsize) {
	pstr->size = newsize;
	pstr->data = jl_mem(jl, pstr->data, pstr->size);
}

/**
 * Inserts a byte at cursor in string pstr.  If not enough size is available,
 * the new memory will be allocated. Value 0 is treated as null byte - dont use.
*/
void jl_data_insert_byte(jl_t *jl, data_t* pstr, uint8_t pvalue) {
	if(strlen((char*)pstr->data) == pstr->size) {
		jl_data_resize(jl, pstr, pstr->size + 1);
	}
	if(jl_data_byte(pstr) == '\0') {
		jl_data_add_byte(pstr, pvalue);
		jl_data_add_byte(pstr, '\0');
	}else{
		int i;
		uint32_t pstr_len = pstr->size;
		pstr->data[pstr_len] = '\0';
		for(i = pstr_len - 1; i > pstr->curs; i--)
			pstr->data[i] = pstr->data[i-1];
		jl_data_add_byte(pstr, pvalue);
	}
}

void jl_data_insert_data(jl_t *jl, data_t* pstr, const void* data, uint32_t size) {
	// Add size
	jl_data_resize(jl, pstr, pstr->size + size);
	// Copy data.
	jl_mem_copyto(data, pstr->data + pstr->curs, size);
	// Increase cursor
	pstr->curs+=size;
}

/**
 * At the cursor in string 'a' replace 'bytes' bytes of 'b' at it's cursor.
 * jl_data_data(jl, { data="HELLO", curs=2 }, { "WORLD", curs=2 }, 2);
 *  would make 'a'
 *	"HELLO"-"LL" = "HE\0\0O"
 *	"WORLD"[2] and [3] = "RL"
 *	"HE"+"RL"+"O" = "HERLO"
 * @param jl: library context
 * @param a: string being modified
 * @param b: string being copied into 'a'
 * @param bytes: the number of bytes to copy over
 */
void jl_data_data(jl_t *jl, data_t* a, const data_t* b, uint64_t bytes) {
	int32_t i;
	uint32_t size = a->size;
	uint32_t sizeb = a->curs + bytes;

	if(a == NULL) {
		jl_print(jl, "jl_data_data: NULL A STRING");
		exit(-1);
	}else if(b == NULL) {
		jl_print(jl, "jl_data_data: NULL B STRING");
		exit(-1);
	}
	if(sizeb > size) size = sizeb;
	a->data = jl_mem(jl, a->data, size + 1);
	for(i = 0; i < bytes; i++) {
		a->data[i + a->curs] = b->data[i + b->curs];
	}
	a->size = size;
	a->data[a->size] = '\0';
}

/**
 * Add string "b" at the end of string "a"
 * @param 'jl': library context
 * @param 'a': string being modified
 * @param 'b': string being appended onto "a"
 */
void jl_data_merg(jl_t *jl, data_t* a, const data_t* b) {
	a->curs = a->size;
	jl_data_data(jl, a, b, b->size);
}

/**
 * Truncate the string to a specific length.
 * @param 'jl': library context
 * @param 'a': string being modified
 * @param 'size': size to truncate to.
 */
void jl_data_trunc(jl_t *jl, data_t* a, uint32_t size) {
	a->curs = 0;
	a->size = size;
	a->data = jl_mem(jl, a->data, a->size + 1);
}

/**
 * Get a string ( char * ) from a 'strt'.  Then, free the 'strt'.
 * @param jl: The library context.
 * @param a: the 'strt' to convert to a string ( char * )
 * @returns: a new string (char *) with the same contents as "a"
*/
char* jl_data_tostring(jl_t* jl, data_t* a) {
	char *rtn = (void*)a->data;
	return rtn;
}

/**
 * Tests if the next thing in array script is equivalent to particle.
 * @param script: The array script.
 * @param particle: The phrase to look for.
 * @return 1: If particle is at the cursor.
 * @return 0: If particle is not at the cursor.
*/
uint8_t jl_data_test_next(data_t* script, const char* particle) {
	char * point = (void*)script->data + script->curs; //the cursor
	char * place = strstr(point, particle); //search for partical
	if(place == point) {//if partical at begining return true otherwise false
		return 1;
	}else{
		return 0;
	}
}

/**
 * Returns string "script" truncated to "psize" or to the byte "end" in
 * "script".  It is dependant on which happens first.
 * @param jl: The library context.
 * @param script: The array script.
 * @param end: byte to end at if found.
 * @param psize: maximum size of truncated "script" to return.
 * @returns: a "strt" that is a truncated array script.
*/
void jl_data_read_upto(jl_t* jl, data_t* compiled, data_t* script, uint8_t end,
	uint32_t psize)
{
	jl_data_init(jl, compiled, psize);
	compiled->curs = 0;
	while((jl_data_byte(script) != end) && (jl_data_byte(script) != 0)) {
		strncat((void*)compiled->data,
			(void*)script->data + script->curs, 1);
		script->curs++;
		compiled->curs++;
	}
	jl_data_trunc(jl, compiled, compiled->curs);
}
