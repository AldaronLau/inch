/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRvo.c
 *	Vertex Buffer Objects -> draw things on the screen.
 */
#include "JLGRprivate.h"

/** @cond */

static void jlgr_vo_vertices__(jlgr_t* jlgr, jl_vo_t* vo, const float *xyzw,
	uint32_t vertices)
{
	vo->vc = vertices;
	if(vertices) {
		// Re-Allocate vo->cc
		vo->cc = jl_mem(jlgr->jl, vo->cc, vertices * sizeof(float) * 4);
		// Re-Allocate vo->cv
		vo->cv = jl_mem(jlgr->jl, vo->cv, vertices * sizeof(float) * 3);
		// Set vo->cv & vo->gl
		jlgr_opengl_vertices_(jlgr, xyzw, vertices, vo->cv, &vo->gl);
	}
}

static void jlgr_vo_color_buffer__(jlgr_t* jlgr, jl_vo_t* vo, float* cc) {
	vo->tx = 0;
	jl_mem_copyto(cc, vo->cc, vo->vc * 4 * sizeof(float));
	// Set Color Buffer "vo->bt" to "vo->cc"
	jlgr_opengl_buffer_set_(jlgr, &vo->bt, vo->cc, vo->vc * 4);
}

// Set texturing to a bitmap
static void jlgr_vo_txtr__(jlgr_t* jlgr, jl_vo_t* vo, uint32_t tx) {
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	// Make sure non-textured colors aren't attempted
	vo->tx = tx;
#ifdef JL_DEBUG
	if(!vo->tx) {
		jl_print(jlgr->jl, "Error: Texture=0!");
		jl_print_stacktrace(jlgr->jl);
		exit(-1);
	}
#endif
	jlgr_vo_txmap(jlgr, vo, 0, 0, -1);
}

// Set vertices for a polygon.
static void jlgr_vo_poly__(jlgr_t* jlgr, jl_vo_t* vo, uint32_t vertices,
	const float *xyzw)
{
	const float FS_RECT[] = {
		0.,jl_gl_ar(jlgr),0.,
		0.,0.,0.,
		1.,0.,0.,
		1.,jl_gl_ar(jlgr),0.
	};

	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	if(xyzw == NULL) xyzw = FS_RECT;
	// Rendering Style = polygon
	vo->rs = 0;
	// Set the vertices of vertex object "vo"
	jlgr_vo_vertices__(jlgr, vo, xyzw, vertices);
}

/** @endcond */

/**
 * Create an empty vertex object.
 * @param jl: The library context.
 * @param vo: A uninitialized vertex object - to initailize with 0 vertices.
**/
void jlgr_vo_init(jlgr_t* jlgr, jl_vo_t* vo) {
	// GL VBO
	vo->gl = 0;
	// GL Texture Coordinate Buffer
	vo->bt = 0;
	// Converted Vertices
	vo->cv = NULL;
	// Vertex Count
	vo->vc = 0;
	// Converted Colors
	vo->cc = NULL;
	// Rendering Style = Polygon
	vo->rs = 0;
	// Texture
	vo->tx = 0;
}

/**
 * Set a Vertex object to vector graphics.
 * @param jlgr: The library context.
 * @param vo: The vertex object to set.
**/
void jlgr_vo_set_vg(jlgr_t* jlgr, jl_vo_t *vo, uint16_t tricount,
	float* triangles, float* colors, uint8_t multicolor)
{
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	// Rendering Style = triangles
	vo->rs = 1;
	// Set the vertices of vertex object "vo"
	jlgr_vo_vertices__(jlgr, vo, triangles, tricount * 3);
	// Texture the vertex object
	if(multicolor) jlgr_vo_color_gradient(jlgr, vo, colors);
	else jlgr_vo_color_solid(jlgr, vo, colors);
}

/**
 * Set a vertex object to a rectangle.
 * @param jl: The library context.
 * @param vo: The vertex object
 * @param rc: The rectangle coordinates.
 * @param colors: The color(s) to use - [ R, G, B, A ]
 * @param multicolor: If 0: Then 1 color is used.
 *	If 1: Then 1 color per each vertex is used.
**/
void jlgr_vo_set_rect(jlgr_t* jlgr, jl_vo_t *vo, jl_rect_t rc, float* colors,
	uint8_t multicolor)
{
	float rectangle_coords[] = {
		rc.x,		rc.y + rc.h,	0.f,
		rc.x,		rc.y,		0.f,
		rc.x + rc.w,	rc.y,		0.f,
		rc.x + rc.w,	rc.y + rc.h,	0.f };

	// Overwrite the vertex object
	jlgr_vo_poly__(jlgr, vo, 4, rectangle_coords);
	// Texture the vertex object
	if(multicolor) jlgr_vo_color_gradient(jlgr, vo, colors);
	else jlgr_vo_color_solid(jlgr, vo, colors);
}

/**
 * Set a vertex object to an Image.
 *
 * @param jlgr: The library context
 * @param vo: The vertex object
 * @param rc: the rectangle to draw the image in.
 * @param tex:  the ID of the image.
**/
void jlgr_vo_set_image(jlgr_t* jlgr, jl_vo_t *vo, jl_rect_t rc, uint32_t tex) {
	//From bottom left & clockwise
	float Oone[] = {
		rc.x,		rc.y + rc.h,	0.f,
		rc.x,		rc.y,		0.f,
		rc.x + rc.w,	rc.y,		0.f,
		rc.x + rc.w,	rc.y + rc.h,	0.f };
	// Overwrite the vertex object
	jlgr_vo_poly__(jlgr, vo, 4, Oone);
	// Texture the vertex object
	jlgr_vo_txtr__(jlgr, vo, tex);
}

/**
 * Change the character map for a texture.
 * @param jl: The library context.
 * @param vo: The vertext object to change.
 * @param w: How many characters wide the texture is.
 * @param h: How many characters high the texture is.
 * @param map: The character value to map.  -1 for full texture.
**/
void jlgr_vo_txmap(jlgr_t* jlgr,jl_vo_t* vo,uint8_t w,uint8_t h,int16_t map) {
	if(map != -1) {
		float ww = (float)w;
		float hh = (float)h;
		float CX = ((float)(map%h))/ww;
		float CY = ((float)(map/h))/hh;
		float tex1[] = {
			(DEFAULT_TC[0]/ww) + CX, (DEFAULT_TC[1]/hh) + CY,
			(DEFAULT_TC[2]/ww) + CX, (DEFAULT_TC[3]/hh) + CY,
			(DEFAULT_TC[4]/ww) + CX, (DEFAULT_TC[5]/hh) + CY,
			(DEFAULT_TC[6]/ww) + CX, (DEFAULT_TC[7]/hh) + CY
		};
		jlgr_opengl_buffer_set_(jlgr, &vo->bt, tex1, 8);
	}else{
		jlgr_opengl_buffer_set_(jlgr, &vo->bt, DEFAULT_TC, 8);
	}
}

/**
 * Change the coloring scheme for a vertex object to a gradient.
 * @param jl: The library context.
 * @param vo: The Vertex Object
 * @param rgba: { (4 * vertex count) values }
**/
void jlgr_vo_color_gradient(jlgr_t* jlgr, jl_vo_t* vo, float* rgba) {
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	jlgr_vo_color_buffer__(jlgr, vo, rgba);
}

/**
 * Change the coloring scheme for a vertex object to a solid.
 * @param jl: The library context.
 * @param vo: The Vertex Object
 * @param rgba: { 4 values }
**/
void jlgr_vo_color_solid(jlgr_t* jlgr, jl_vo_t* vo, float* rgba) {
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	float rgbav[4 * vo->vc];
	uint32_t i;

	for(i = 0; i < vo->vc; i++) {
		jl_mem_copyto(rgba, &(rgbav[i * 4]), 4 * sizeof(float));
	}
	jlgr_vo_color_buffer__(jlgr, vo, rgbav);
}

/**
 * Draw vertex object.
 * @param jlgr: The library context.
 * @param vo: The vertex object to draw.
 * @param sh: The shader to use ( must be the same one used with
 *	jlgr_opengl_draw1(). )
**/
void jlgr_vo_draw2(jlgr_t* jlgr, jl_vo_t* vo, jlgr_glsl_t* sh) {
	jl_print_function(jlgr->jl, "GL/Draw");
	// Use Temporary Vertex Object If no vertex object.
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	if(vo->tx) {
		// Bind Texture Coordinates to shader
		jlgr_opengl_setv(jlgr, &vo->bt,
			jlgr->effects.alpha.attributes.texpos_color, 2);
		// Bind the texture
		jlgr_opengl_texture_bind_(jlgr, vo->tx);
	}else{
		// Bind Colors to shader
		jlgr_opengl_setv(jlgr, &vo->bt,
			jlgr->gl.prg.color.attributes.texpos_color, 4);
	}
	// Update the position variable in shader.
	jlgr_opengl_setv(jlgr, &vo->gl, sh->attributes.position, 3);
	// Draw the image on the screen!
	jlgr_opengl_draw_arrays_(jlgr, vo->rs ? GL_TRIANGLES : GL_TRIANGLE_FAN,
		vo->vc);
	jl_print_return(jlgr->jl, "GL/Draw");
}

/**
 * Draw a vertex object with offset by translation.
 * @param jl: The library context.
 * @param vo: The vertex object to draw.
 * @param vec: The vector of offset/translation.
**/
void jlgr_vo_draw(jlgr_t* jlgr, jl_vo_t* vo, jl_vec3_t* vec) {
	jlgr_glsl_t* shader = vo->tx ?
		&jlgr->gl.prg.texture : &jlgr->gl.prg.color;

	jlgr_opengl_draw1(jlgr, shader);
	if(vec == NULL) {
		jlgr_opengl_transform_(jlgr, shader,
			0.f, 0.f, 0.f, 1., 1., 1., jl_gl_ar(jlgr));
	}else{
		jlgr_opengl_transform_(jlgr, shader,
			vec->x, vec->y, vec->z, 1., 1., 1., jl_gl_ar(jlgr));
	}
	jlgr_vo_draw2(jlgr, vo, shader);
}

/**
 * Free a vertex object.
 * @param jl: The library context
 * @param vo: The vertex object to free
**/
void jlgr_vo_free(jlgr_t* jlgr, jl_vo_t *vo) {
	// Free GL VBO
	jlgr_opengl_buffer_old_(jlgr, &vo->gl);
	// Free GL Texture Buffer
	jlgr_opengl_buffer_old_(jlgr, &vo->bt);
	// Free Converted Vertices & Colors
	if(vo->cv) vo->cv = jl_mem(jlgr->jl, vo->cv, 0);
	if(vo->cc) vo->cc = jl_mem(jlgr->jl, vo->cc, 0);
	// Free main structure
	vo = jl_mem(jlgr->jl, (void**)&vo, 0);
}
