/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRsg.c
 *	sg AKA. Simple Graphics does the window handling.
**/
#include "JLGRinternal.h"

// SG Prototypes
void jl_gl_draw_prendered(jlgr_t* jlgr, jl_vo_t* pv);

// Constants
	//ALL IMAGES: 1024x1024
	#define TEXTURE_WH 1024*1024 
	//1bpp Bitmap = 1048832 bytes (Color Key(256)*RGBA(4), 1024*1024)
	#define IMG_FORMAT_LOW 1048832 
	//2bpp HQ bitmap = 2097664 bytes (Color Key(256*2=512)*RGBA(4), 2*1024*1024)
	#define IMG_FORMAT_MED 2097664
	//3bpp Picture = 3145728 bytes (No color key, RGB(3)*1024*1024)
	#define IMG_FORMAT_PIC 3145728
	//
	#define IMG_SIZE_LOW (1+strlen(JL_IMG_HEADER)+(256*4)+(1024*1024)+1)
	
//Functions:

//Get a pixels RGBA values from a surface and xy
uint32_t _jl_sg_gpix(/*in */ SDL_Surface* surface, int32_t x, int32_t y) {
	int32_t bpp = surface->format->BytesPerPixel;
	uint8_t *p = (uint8_t *)surface->pixels + (y * surface->pitch) + (x * bpp);
	uint32_t color_orig;
	uint32_t color;
	uint8_t* out_color = (void*)&color;

	if(bpp == 1) {
		color_orig = *p;
	}else if(bpp == 2) {
		color_orig = *(uint16_t *)p;
	}else if(bpp == 3) {
		if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			color_orig = p[0] << 16 | p[1] << 8 | p[2];
		else
			color_orig = p[0] | p[1] << 8 | p[2] << 16;
	}else{ // 4
		color_orig = *(uint32_t *)p;
	}
	SDL_GetRGBA(color_orig, surface->format, &(out_color[0]),
		&(out_color[1]), &(out_color[2]), &(out_color[3]));
	return color;
}

void _jl_sg_load_jlpx(jlgr_t* jlgr,data_t* data,void **pixels,int *w,int *h) {
	SDL_Surface *image;
	SDL_RWops *rw;
	uint32_t color = 0;
	data_t pixel_data;
	int i, j;

	if(data->data[0] == 0) {
		jl_print(jlgr->jl, "NO DATA!");
		exit(-1);
	}

	jl_print_function(jlgr->jl, "SG_Jlpx");

	JL_PRINT_DEBUG(jlgr->jl, "File Size = %d", data->size);
	rw = SDL_RWFromMem(data->data + data->curs, data->size);
	if ((image = IMG_Load_RW(rw, 1)) == NULL) {
		jl_print(jlgr->jl, "Couldn't load image: %s",
			IMG_GetError());
		exit(-1);
	}
	// Covert SDL_Surface.
	jl_data_init(jlgr->jl, &pixel_data, image->w * image->h * 4);
	for(i = 0; i < image->h; i++) {
		for(j = 0; j < image->w; j++) {
			color = _jl_sg_gpix(image, j, i);
			jl_data_saveto(&pixel_data, 4, &color);
		}
	}
	//Set Return values
	*pixels = jl_data_tostring(jlgr->jl, &pixel_data);
	*w = image->w;
	*h = image->h;
	// Clean-up
	SDL_free(image);

	jl_print_return(jlgr->jl, "SG_Jlpx");
}

//Load the images in the image file
static inline uint32_t jl_sg_add_image__(jlgr_t* jlgr, data_t* data) {
	void *fpixels = NULL;
	int fw;
	int fh;
	uint32_t rtn;

	jl_print_function(jlgr->jl, "SG_InitImgs");
	JL_PRINT_DEBUG(jlgr->jl, "size = %d", data->size);
//load textures
	_jl_sg_load_jlpx(jlgr, data, &fpixels, &fw, &fh);
	JL_PRINT_DEBUG(jlgr->jl, "creating image....");
	rtn = jl_gl_maketexture(jlgr, fpixels, fw, fh, 0);
	JL_PRINT_DEBUG(jlgr->jl, "created image!");
	jl_print_return(jlgr->jl, "SG_InitImgs");
	return rtn;
}

/**
 * Load an image from a zipfile.
 * @param jlgr: The library context
 * @param zipdata: data for a zip file.
 * @param filename: Name of the image file in the package.
 * @returns: Texture object.
*/
uint32_t jl_sg_add_image(jlgr_t* jlgr, data_t* zipdata, const char* filename) {
	jl_print_function(jlgr->jl, "SG_LImg");
	data_t img;

	// Load image into "img"
	jl_file_pk_load_fdata(jlgr->jl, &img, zipdata, filename);
	if(jlgr->jl->errf) exit(-1);

	JL_PRINT_DEBUG(jlgr->jl, "Loading Image....");
	uint32_t rtn = jl_sg_add_image__(jlgr, &img);
	JL_PRINT_DEBUG(jlgr->jl, "Loaded Image!");
	jl_print_return(jlgr->jl, "SG_LImg");
	return rtn;
}

static void jl_sg_draw_up(jl_t* jl, uint8_t resize, void* data) {
	jlgr_t* jlgr = jl->jlgr;
	
	// Clear the screen.
	jl_gl_clear(jl->jlgr, 0., .5, .66, 1.);
	// Run the screen's redraw function
	(jlgr->sg.cs == JL_SCR_UP) ? ((jl_fnct)jlgr->draw.redraw.lower)(jl) :
		((jl_fnct)jlgr->draw.redraw.upper)(jl);
}

static void jl_sg_draw_dn(jl_t* jl, uint8_t resize, void* data) {
	jlgr_t* jlgr = jl->jlgr;

	// Clear the screen.
	jl_gl_clear(jlgr, 1., .5, 0., 1.);
	// Run the screen's redraw function
	(jlgr->sg.cs == JL_SCR_UP) ? ((jl_fnct)jlgr->draw.redraw.upper)(jl) :
		((jl_fnct)jlgr->draw.redraw.lower)(jl);
	// Draw Menu Bar & Mouse
	_jlgr_loopa(jl->jlgr);
}

// Double screen loop
static void _jl_sg_loop_ds(jlgr_t* jlgr) {
	// Draw upper screen - alternate screen
	jlgr_sprite_redraw(jlgr, &jlgr->sg.bg.up, NULL);
	jlgr_sprite_draw(jlgr, &jlgr->sg.bg.up);
	// Draw lower screen - default screen
	jlgr_sprite_redraw(jlgr, &jlgr->sg.bg.dn, NULL);
	jlgr_sprite_draw(jlgr, &jlgr->sg.bg.dn);
}

// Single screen loop
static void _jl_sg_loop_ss(jlgr_t* jlgr) {
	// Draw lower screen - default screen
	jlgr_sprite_redraw(jlgr, &jlgr->sg.bg.dn, NULL);
	jlgr_sprite_draw(jlgr, &jlgr->sg.bg.dn);
}

// Run the current loop.
void _jl_sg_loop(jlgr_t* jlgr) {
	jl_print_function(jlgr->jl, "SG_LP");
	((jlgr_fnct)jlgr->sg.loop)(jlgr);
	jl_print_return(jlgr->jl, "SG_LP");
}

static void jl_sg_init_ds_(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	jl_rect_t rcrd = {
		0., 0.,
		1., .5 * jlgr->wm.ar
	};

	jlgr_sprite_resize(jlgr, &jlgr->sg.bg.up, &rcrd);
	rcrd.y = .5 * jlgr->wm.ar;
	jlgr_sprite_resize(jlgr, &jlgr->sg.bg.dn, &rcrd);
	// Set double screen loop.
	jlgr->sg.loop = _jl_sg_loop_ds;
	if(jlgr->sg.cs == JL_SCR_SS) jlgr->sg.cs = JL_SCR_DN;
	//
	//jlgr->sg.bg.up->pr->ar = jlgr->wm.ar / 2.;
	//jlgr->sg.bg.dn->pr->ar = jlgr->wm.ar / 2.;
}

static void jl_sg_init_ss_(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	jl_rect_t rcrd = {
		0.f, 0.f,
		1., jlgr->wm.ar
	};

	jlgr_sprite_resize(jlgr, &jlgr->sg.bg.dn, &rcrd);
	// Set single screen loop.
	jlgr->sg.loop = _jl_sg_loop_ss;
	jlgr->sg.cs = JL_SCR_SS;
	//
	jlgr->sg.bg.dn.pr.ar = jlgr->wm.ar;
}

void jl_sg_resz__(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	// Check screen count.
	if(jlgr->sg.cs == JL_SCR_SS)
		jl_sg_init_ss_(jl);
	else
		jl_sg_init_ds_(jl);
}

void jl_sg_init__(jlgr_t* jlgr) {
	jl_rect_t rc = { 0., 0., 1., jl_gl_ar(jlgr) };

	// Initialize redraw routines to do nothing.
	jlgr->draw.redraw = (jlgr_redraw_t){jl_dont, jl_dont, jl_dont, jl_dont};
	// Create upper and lower screens
	jlgr_sprite_init(jlgr, &jlgr->sg.bg.up, rc,
		jlgr_sprite_dont, jl_sg_draw_up, NULL, 0, NULL, 0);
	jlgr_sprite_init(jlgr, &jlgr->sg.bg.dn, rc,
		jlgr_sprite_dont, jl_sg_draw_dn, NULL, 0, NULL, 0);
	// Flip upside-down
	jlgr->sg.bg.up.pr.scl.y = -1.;
	jlgr->sg.bg.dn.pr.scl.y = -1.;
	// Resize.
	jlgr->sg.cs = JL_SCR_SS; // JL_SCR_DN
	jl_sg_resz__(jlgr->jl);
}
