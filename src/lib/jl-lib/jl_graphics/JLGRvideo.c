/*
	JAL5_VIDE

		VIDE is for editing sounds and graphics.
*/
#include "JLGRinternal.h"

#undef HAVE_STDLIB_H
#include "jpeglib.h"

typedef long unsigned int jpeg_long_int_t;

/**
 * Save a JPEG to data
 * @param jl: The library context.
 * @param rtn: The jl_data_t to return.
 * @param quality: The quality to save the data as.
 * @param pxdata: 3-byte item R,G,B pixel array
 * @param w: The width
 * @param h: THe height
 * @returns: The data.
**/
void jl_vi_make_jpeg(jl_t* jl, data_t* rtn, uint8_t quality, uint8_t* pxdata,
	uint16_t w, uint16_t h)
{
	uint8_t* data = NULL;
	jpeg_long_int_t data_size = 0;

	jl_print(jl, "w:%d h:%d", w, h);

	/* This struct contains the JPEG compression parameters and pointers to
	 * working space (which is allocated as needed by the JPEG library).
	 * It is possible to have several such structures, representing multiple
	 * compression/decompression processes, in existence at once.	We refer
	 * to any one struct (and its associated working data) as a "JPEG object".
	 */
	struct jpeg_compress_struct cinfo;
	/* This struct represents a JPEG error handler.	It is declared separately
	 * because applications often want to supply a specialized error handler
	 * (see the second half of this file for an example).	But here we just
	 * take the easy way out and use the standard error handler, which will
	 * print a message on stderr and call exit() if compression fails.
	 * Note that this struct must live as long as the main JPEG parameter
	 * struct, to avoid dangling-pointer problems.
	 */
	struct jpeg_error_mgr jerr;
	/* More stuff */
	JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
	int row_stride;		/* physical row width in image buffer */

	/* Step 1: allocate and initialize JPEG compression object */

	/* We have to set up the error handler first, in case the initialization
	 * step fails.	(Unlikely, but it could happen if you are out of memory.)
	 * This routine fills in the contents of struct jerr, and returns jerr's
	 * address which we place into the link field in cinfo.
	 */
	cinfo.err = jpeg_std_error(&jerr);
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */
	jpeg_mem_dest(&cinfo, &data, &data_size);

	/* Step 3: set parameters for compression */

	/* First we supply a description of the input image.
	 * Four fields of the cinfo struct must be filled in:
	 */
	cinfo.image_width = w; 	/* image width and height, in pixels */
	cinfo.image_height = h;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	/* Now use the library's routine to set default compression parameters.
	 * (You must set at least cinfo.in_color_space before calling this,
	 * since the defaults depend on the source color space.)
	 */
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	 * Here we just illustrate the use of quality (quantization table) scaling:
	 */
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */

	/* TRUE ensures that we will write a complete interchange-JPEG file.
	 * Pass TRUE unless you are very sure of what you're doing.
	 */
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*					 jpeg_write_scanlines(...); */

	/* Here we use the library's state variable cinfo.next_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 * To keep things simple, we pass one scanline per call; you can pass
	 * more if you wish, though.
	 */
	row_stride = w * 3;	/* JSAMPLEs per row in pxdata */

	while (cinfo.next_scanline < cinfo.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		 * Here the array is only one element long, but you could pass
		 * more than one scanline at a time if that's more convenient.
		 */
		row_pointer[0] = & pxdata[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	/* Step 6: Finish compression */

	jpeg_finish_compress(&cinfo);

	/* Step 7: release JPEG compression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

	/* And we're done! */
	jl_data_mkfrom_data(jl, rtn, data_size, data);
}

//void memtester(jl_t* jl, str_t name);

/**
 * Load an image from data.
 * @param jl: The library context.
 * @param data: The data to read.
 * @param w: Pointer to the width variable.
 * @param h: Pointer to the height variable.
 * @returns: Raw pixel data.
**/
uint8_t* jl_vi_load_(jl_t* jl, data_t* data, uint16_t* w, uint16_t* h) {
	SDL_Surface *image; //  Free'd by SDL_free(image);
	SDL_RWops *rw; // Free'd by SDL_RWFromMem
	void* img_file; // Free'd by jl_mem
	data_t pixel_data; // Free'd by jl_mem_string_fstrt
	void* rtn_pixels; // Returned so not free'd.
	uint32_t color = 0;
	uint32_t FSIZE = data->size;
	int i, j;
	uint32_t rgba = 3;

//	memtester(jl, "LoadImg/Start0");
	img_file = jl_memi(jl, FSIZE);
//	memtester(jl, "LoadImg/Start1");
	jl_data_loadto(data, FSIZE, img_file);
//	memtester(jl, "LoadImg/Start2");
	rw = SDL_RWFromMem(img_file, FSIZE);
//	memtester(jl, "LoadImg/Start3");
	if ((image = IMG_Load_RW(rw, 0)) == NULL) {
		jl_print(jl, "Couldn't load image: %s", IMG_GetError());
		exit(-1);
	}
//	memtester(jl, "LoadImg/Start4");
	// Covert SDL_Surface.
	jl_data_init(jl, &pixel_data, image->w * image->h * rgba);
	for(i = 0; i < image->h; i++) {
		for(j = 0; j < image->w; j++) {
			color = _jl_sg_gpix(image, j, i);
			jl_data_saveto(&pixel_data, rgba, &color);
		}
	}
//	memtester(jl, "LoadImg/Start5");
	//Set Return values
	rtn_pixels = jl_data_tostring(jl, &pixel_data);
//	memtester(jl, "LoadImg/End6");
	*w = image->w;
	*h = image->h;
	// Clean-up
	SDL_FreeSurface(image);
//	memtester(jl, "LoadImg/End4");
	SDL_free(rw);
//	memtester(jl, "LoadImg/End3");
	jl_mem(jl, &img_file, 0);
//	memtester(jl, "LoadImg/End1");
	return rtn_pixels;
}
