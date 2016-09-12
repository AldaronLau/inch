/*
 * (c) Jeron A. Lau
 *
 * INCH: INclude C Header
*/

#include "inch.h"
#include "la_buffer.h"
#include "la_memory.h"
#include "la_file.h"

static inline void inch_conv_add(jl_t* jl, const char* printed) {
	inch_t* ctx = la_context(jl);

	// Insert the data
	jl_data_insert_data(jl, &ctx->output, printed, strlen(printed));
}

char* inch_conv_package(jl_t* jl, const char* FolderName) {
	char* pk_compress;

	pk_compress = jl_file_pk_compress(jl, FolderName);
	return pk_compress;
}

void inch_conv_conv(jl_t* jl, const char *PackageName, const char* varname) {
	int i;
	data_t input;
	char *file_name = jl_memi(jl, 100);
	inch_t* ctx = la_context(jl);

	// 
	strcat((void*)file_name,(void*)PackageName);
	la_file_load(&input, file_name);
	la_file_rm(file_name);
	ctx->fp_filename = la_memory_makecopy(varname, strlen(varname) + 1);
	char printed[80];
	inch_conv_add(jl, "#include <stdint.h>\n");
	inch_conv_add(jl, "static uint8_t inch_data[]=\"");
	// Other Values
	la_print(INCH_BOLD);
	for(i = 0; i < input.size; i++) {
		// Format the code
		jl_mem_format(printed, "\\x%X", input.data[i]);
		inch_conv_add(jl, printed);
		// Notify % finished.
		if(i%(input.size / 80) == 0) {
			printf(".");
		}
	}
	la_print(LA_PRESET);
	jl_mem_format(printed, "\";static uint64_t inch_size=%d;\n",input.size);
	inch_conv_add(jl, printed);
	jl_mem_format(printed, "void* %s_data(void){return inch_data;}\n", ctx->fp_filename);
	inch_conv_add(jl, printed);
	jl_mem_format(printed, "uint64_t %s_size(void){return inch_size;}", ctx->fp_filename);
	inch_conv_add(jl, printed);
}

void inch_conv_save(jl_t* jl) {
	inch_t* ctx = la_context(jl);
	char* output = la_buffer_tostring(&ctx->output);

	jl_file_save(jl, output, SAVE, ctx->output.curs);
	la_print(NULL);
	printf("void* %s_data(void);\n", ctx->fp_filename);
	printf("uint64_t %s_size(void);\n", ctx->fp_filename);
	la_print(NULL);
	return;
}

void inch_conv_init(jl_t* jl) {
	inch_t* ctx = la_context(jl);
	jl_data_init(jl, &ctx->output, 0);
}
