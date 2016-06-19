/*
 * (c) Jeron A. Lau
 *
 * INCH: INclude C Header
*/

#include "header/inch.h"

static inline void inch_conv_add(jl_t* jl, const char* printed) {
//	jl_print(jl, "CONV ADD...");
	inch_t* ctx = jl_get_context(jl);

	// Insert the data
	jl_data_insert_data(jl, &ctx->output, printed, strlen(printed));
}

char* inch_conv_package(jl_t* jl, const char* FolderName) {
	char* pk_compress;

	pk_compress = jl_file_pk_compress(jl, FolderName);
	jl_print(jl, "Compressed folder: %s -> %s.", FolderName, pk_compress);
	return pk_compress;
}

void inch_conv_conv(jl_t* jl, const char *PackageName) {
	int i;
	data_t input;
	char *file_name = jl_memi(jl, 100);
	uint32_t filename_size = strlen(PackageName);
	inch_t* ctx = jl_get_context(jl);

	// 
	strcat((void*)file_name,(void*)PackageName);
	jl_print_rewrite(jl, "loading input file %s [....]", file_name);
	jl_file_load(jl, &input, file_name);
	jl_print_rewrite(jl, "loading input file %s [DONE]", file_name);
	jl_print(jl, NULL);
	char boolean = 0;
	int k;
	i = filename_size;
	while(1) {
		if(PackageName[i] == '/' || PackageName[i] == '!' || i < 0) break;
		if(PackageName[i] == '.') boolean = i;
		i--;
	}
	k = ++i;
	ctx->fp_filename = jl_memi(jl, (boolean - k) + 1);
	for(; i < boolean; i++) {
//		char pn[2]; pn[0] = PackageName[i]; pn[1] = '\0';
//		jl_print(jl, "# %d: %s", (i - k), pn);
		ctx->fp_filename[(i - k)] = PackageName[i];
	}
	ctx->fp_filename[(boolean - k)] = '\0';
	char printed[80];
	jl_mem_format(printed, "unsigned char %s[]=\"", ctx->fp_filename);
	inch_conv_add(jl, printed);
	// Other Values
	for(i = 0; i < input.size; i++) {
		// Format the code
		jl_mem_format(printed, "\\x%X", input.data[i]);
		inch_conv_add(jl, printed);
		// Notify % finished.
		if(i%20 == 0) {
			jl_print_rewrite(jl, "Adding data to file [%4f%%]",
				100.f * ((float)i) / ((float)input.size));
		}
	}
	jl_mem_format(printed, "\";size_t %s_size = %d;", ctx->fp_filename,
		input.size);
	inch_conv_add(jl, printed);
	jl_print(jl, "Adding data to file [DONE]");
}

void inch_conv_save(jl_t* jl) {
	inch_t* ctx = jl_get_context(jl);
	char save[100]; jl_mem_format(save, "!src/media/%s.h", ctx->fp_filename);
	char* output = jl_data_tostring(jl, &ctx->output);

	jl_print_rewrite(jl, "Saving output to \"%s\" [....]", save);
	jl_file_save(jl, output, save, ctx->output.curs);
	jl_print(jl, "Saving output to \"%s\" [DONE]", save);
	jl_print(jl, "");
	jl_print(jl, "variables:");
	jl_print(jl, "");
	jl_print(jl, "\tunsigned char %s[];", ctx->fp_filename);
	jl_print(jl, "");
	return;
}

void inch_conv_init(jl_t* jl) {
	inch_t* ctx = jl_get_context(jl);
	jl_data_init(jl, &ctx->output, 0);
}
