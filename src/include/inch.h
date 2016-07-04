#include "jl.h"

typedef struct{
	data_t output;
	char *fp_filename;
}inch_t;

char* inch_conv_package(jl_t* jl, const char* FolderName);
void inch_conv_conv(jl_t* jl, const char *PackageName);
void inch_conv_save(jl_t* jl);
void inch_conv_init(jl_t* jl);
