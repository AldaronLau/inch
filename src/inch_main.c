/*
 * (c) Jeron A. Lau
 *
 * INCH: INclude C Header
*/

#include "inch.h"

char* inputfile = NULL;

static void inch_init(jl_t* jl) {
	char* zipfile;

	la_print(LA_PBOLD LA_PBLINK LA_PGREEN "INCH" INCH_PRESET ": " LA_PBOLD "I" INCH_PRESET "nclude media i" LA_PBOLD "N C" INCH_PRESET " & be " LA_PBOLD "H" INCH_PRESET "appy V0.5" LA_PRESET);
	zipfile = inch_conv_package(jl, "resources");
	inch_conv_init(jl);
	inch_conv_conv(jl, zipfile, inputfile);
	inch_conv_save(jl);
	exit(0);
}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		la_print("Use:\n\tinch media");
		la_print("\t>> void* media_data(void);");
		la_print("\t>> uint64_t media_size(void);");
		return -1;
	}
	inputfile = argv[1];
	return la_start(inch_init, la_dont, 0, "inch", sizeof(inch_t));
}
