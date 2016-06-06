/*
 * (c) Jeron A. Lau
 *
 * INCH: INclude C Header
*/

#include "header/inch.h"

char *fp_filename;

static inline void inch_conv_add(jl_t* jl, const char* printed) {
//	jl_print(jl, "CONV ADD...");
	inch_t* ctx = jl_get_context(jl);

	// Insert the data
	jl_data_insert_data(jl, &ctx->output, printed, strlen(printed));
}

char* inch_conv_package(jl_t* jl, const char* FolderName) {
	return jl_file_pk_compress(jl, FolderName);
}

void inch_conv_conv(jl_t* jl, const char *PackageName) {
	int i;
	data_t input;
	char *file_name = jl_memi(jl, 100);
	u32_t filename_size = strlen(PackageName);

	// Copy Contents of PackageName into fp_filename
	fp_filename = malloc(filename_size + 1);
	jl_mem_copyto(PackageName, fp_filename, filename_size);
	fp_filename[filename_size] = '\0';
	// 
	strcat((void*)file_name,(void*)PackageName);
	jl_print(jl, "loading input file %s....", file_name);
	jl_file_load(jl, &input, file_name);
	jl_print(jl, "loaded input file.");
	char boolean = 0;
	int k;
	i = filename_size;
	while(1) {
		if(fp_filename[i] == '/') break;
		if(fp_filename[i] == '.') boolean = i;
		i--;
	}
	k = i;
	for(; i < boolean; i++) {
		fp_filename[(i - k)-1] = fp_filename[i];
	}
	fp_filename[(boolean - k) - 1] = '\0';
	jl_print(jl, "\tchar %s[]={", fp_filename);
	SDL_Delay(1000);
	jl_print(jl, "creating text...");
	SDL_Delay(1000);
	char printed[80];
	jl_mem_format(printed, "\tchar %s[]={", fp_filename);
	jl_print(jl, "adding text...\n");
	inch_conv_add(jl, printed);
	jl_print(jl, "adding data to file...");
	// First Value
	jl_mem_format(printed, "%d", input.data[i]);
	inch_conv_add(jl, printed);
	// Other Values
	for(i = 0; i < input.size; i++) {
		// Format the code
		jl_mem_format(printed, ",%d", input.data[i]);
		inch_conv_add(jl, printed);
		// Notify % finished.
		if(i%20 == 0) {
			jl_print_rewrite(jl, "adding data : %4f%%",
				100.f * ((float)i) / ((float)input.size));
		}
	}
	jl_print(jl, "added data!");
	jl_mem_format(printed, "};");
	inch_conv_add(jl, printed);
	jl_print(jl, "Saving....");
}

void inch_conv_save(jl_t* jl) {
	inch_t* ctx = jl_get_context(jl);
	char *save = malloc(100);
	int i;
	data_t VariableString; jl_data_init(jl, &VariableString, 0);
	char append[80]; jl_mem_format(append, "\tchar %s[];", fp_filename);
	uint64_t cursor = ctx->output.curs;
	char* output = jl_data_tostring(jl, &ctx->output);

	jl_data_insert_data(jl, &VariableString, append, strlen(append));

	for(i = 0; i < 100; i++) {
		save[i] = '\0';
	}
	strcat(save, "!src/media/");
	strcat(save, fp_filename);
	strcat(save, ".h");
	jl_print(jl, "Saving output to \"%s\"....", save);
	jl_file_save(jl, output, save, cursor);
	jl_print(jl, "");
	jl_print(jl, "variables:");
	jl_print(jl, "");
	jl_print(jl, (char*)VariableString.data);
	jl_print(jl, "");
	jl_data_free(&VariableString);
	return;
}

void inch_conv_init(jl_t* jl) {
	inch_t* ctx = jl_get_context(jl);
	jl_data_init(jl, &ctx->output, 1);
}
