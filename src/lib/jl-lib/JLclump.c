/*
 * JL_lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * cl.c
 *	Clump extensions.
 *	Uses Clump:
 *		Clump is a handy memory library that includes various variable
 *		structures which are: Bitarrays, Linked List, Hash Tables,
 *		Data Trees and Huffman Codecs.
*/
#include "JLprivate.h"

static inline void *_jl_cl_list_alphabetize_lowest(struct cl_list *list) {
	int i, j, k;
	char *rtn = NULL;
	//Allocate and clear checked
	uint8_t *notchecked = malloc(sizeof(uint8_t) * cl_list_count(list));
	for(i = 0; i < cl_list_count(list); i++) notchecked[i] = 1;
	j = 0;
	while(1) {
		uint8_t ckv = 255;
		uint8_t count = 0;
		struct cl_list_iterator *iterator = cl_list_iterator_create(list);
		for(i = 0; i < cl_list_count(list); i++) {
			char *s = cl_list_iterator_next(iterator);
			if(notchecked[i]) {
				if(s[j] > ckv) {
					notchecked[i] = 0; //check & cancel
				}else if(s[j] == ckv) {
					count++;
				}else if(s[j] < ckv) {
					//Invalidate everything that came before
					//Because this is more that all that.
					for(k = 0; k < i; k++)
						notchecked[k] = 0;
					ckv = s[j];
					count = 1;
					rtn = s;
				}
			}
		}
		cl_list_iterator_destroy(iterator);
		if(count == 1) break;
		else j++;
	}
	free(notchecked);
	return rtn;
}

void jl_cl_list_alphabetize(struct cl_list *list) {
	int i;

	struct cl_list *alphabetized = cl_list_create();
	while(!cl_list_is_empty(list)) {
		void *l = _jl_cl_list_alphabetize_lowest(list);
		cl_list_remove(list, l);
		cl_list_add_tail(alphabetized, l);
	}
	cl_list_clear(list);
	struct cl_list_iterator *iterator =
		cl_list_iterator_create(alphabetized);
	for(i = 0; i < cl_list_count(alphabetized); i++) {
		cl_list_add_tail(list, cl_list_iterator_next(iterator));
	}
	cl_list_destroy(alphabetized);
}

/**
 * Iterate through a clump list.
 * @param jl: The library context.
 * @param list: The clump list.
 * @param fn: The function that reads the data.
**/
void jl_clump_list_iterate(jl_t* jl, struct cl_list *list, jl_data_fnct fn) {
	int i;
	struct cl_list_iterator *iterator = cl_list_iterator_create(list);

	for(i = 0; i < cl_list_count(list); i++)
		fn(jl, cl_list_iterator_next(iterator));
	cl_list_iterator_destroy(iterator);
}
