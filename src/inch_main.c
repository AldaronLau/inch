/*
 * (c) Jeron A. Lau
 *
 * INCH: INclude C Header
*/

#include "header/inch.h"

char* inputfile = NULL;

static void inch_init(jl_t* jl) {
	char* zipfile = NULL;

	jl_print(jl, "INCH: INclude C Header V0.3");
	zipfile = inch_conv_package(jl, inputfile);
	jl_print(jl, "Compressing folder: %s -> %s", inputfile, zipfile);
	inch_conv_init(jl);
	inch_conv_conv(jl, zipfile);
	inch_conv_save(jl);
	jl_print(jl, "Complete!");
	exit(0);
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		printf("Use:\n\tinch -media/\n");
		return -1;
	}
	inputfile = argv[1];
	if(inputfile[0] == '-') inputfile[0] = '!';
	return jl_start(inch_init, jl_dont, "inch", sizeof(inch_t));
}
