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
	return jl_file_pk_compress(jl, FolderName);
}

void inch_conv_conv(jl_t* jl, const char *PackageName) {
	int i;
	data_t input;
	char *file_name = jl_memi(jl, 100);
	u32_t filename_size = strlen(PackageName);
	inch_t* ctx = jl_get_context(jl);

	// 
	strcat((void*)file_name,(void*)PackageName);
	jl_print(jl, "loading input file %s....", file_name);
	jl_file_load(jl, &input, file_name);
	jl_print(jl, "loaded input file.");
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
	jl_print(jl, "unsigned char %s[]={", ctx->fp_filename);
	jl_print(jl, "Creating text...");
	char* printed = jl_memi(jl, 80);
	jl_mem_format(printed, "unsigned char %s[]={", ctx->fp_filename);
	jl_print(jl, "Adding text....");
	inch_conv_add(jl, printed);
	jl_print(jl, "Adding data to file....");
	// First Value
	jl_mem_format(printed, "%d", input.data[0]);
	inch_conv_add(jl, printed);
	// Other Values
	for(i = 1; i < input.size; i++) {
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
	char save[100]; jl_mem_format(save, "!src/media/%s.h", ctx->fp_filename);
	data_t VariableString; jl_data_init(jl, &VariableString, 0);
	char append[80];
		jl_mem_format(append, "unsigned char %s[];", ctx->fp_filename);
	uint64_t cursor = ctx->output.curs;
	char* output = jl_data_tostring(jl, &ctx->output);

	jl_data_insert_data(jl, &VariableString, append, strlen(append));

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
	jl_data_init(jl, &ctx->output, 0);
}
