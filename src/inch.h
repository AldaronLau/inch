#include "jl.h"
#include "la_port.h"
#define SAVE "src/resources.c"

#define INCH_PRESET LA_PRESET LA_PBLUE
#define INCH_BOLD LA_PWHITE LA_PBOLD

typedef struct{
	data_t output;
	char *fp_filename;
}inch_t;

char* inch_conv_package(jl_t* jl, const char* FolderName);
void inch_conv_conv(jl_t* jl, const char *PackageName, const char* varname);
void inch_conv_save(jl_t* jl);
void inch_conv_init(jl_t* jl);
