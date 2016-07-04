#include "jlgr.h"
#include "JLprivate.h"

// Full texture
#define DEFAULT_TC (const float[]) { \
	0., 1., \
	0., 0., \
	1., 0., \
	1., 1. \
}

typedef enum{
	JLGR_ID_NULL,
	JLGR_ID_UNKNOWN,
	JLGR_ID_FLIP_IMAGE,
	JLGR_ID_SLOW_IMAGE,
	JLGR_ID_GOOD_IMAGE,
	JLGR_ID_TASK_MAX //how many taskbuttons
}jlgr_id_t;

typedef enum{
	JLGR_COMM_NONE,		/** Does nothing */
	JLGR_COMM_RESIZE,	/** main --> draw: Resize the drawing screen */
	JLGR_COMM_KILL,		/** main --> draw: Close the window*/
	JLGR_COMM_INIT,		/** main --> draw: Send program's init func. */
	JLGR_COMM_SEND,		/** main --> draw: Send redraw func.'s */
	JLGR_COMM_NOTIFY,	/** main --> draw: Draw a notification */
}jlgr_thread_asdf_t;

typedef struct{
	uint8_t id;
	char string[256];
}jlgr_comm_notify_t;

uint32_t _jl_sg_gpix(/*in */ SDL_Surface* surface, int32_t x, int32_t y);
void jl_gl_viewport_screen(jlgr_t* jlgr);
void jlgr_opengl_transform_(jlgr_t* jlgr, jlgr_glsl_t* sh,
	float xe, float ye, float ze, float xm, float ym, float zm, float ar);
void jl_gl_vo_free(jlgr_t* jlgr, jl_vo_t *pv);
uint32_t jl_gl_w(jlgr_t* jlgr);

// JLGRopengl.c
void jlgr_opengl_buffer_set_(jlgr_t* jlgr, GLuint *buffer,
	const void *buffer_data, uint16_t buffer_size);
void jlgr_opengl_buffer_old_(jlgr_t* jlgr, uint32_t *buffer);
void jlgr_opengl_uniform1f_(jlgr_t* jlgr, GLint uv, float a);
void jlgr_opengl_uniform3f_(jlgr_t* jlgr, GLint uv, float x, float y, float z);
void jlgr_opengl_uniform4f_(jlgr_t* jlgr, GLint uv, float x, float y, float z,
	float w);
void jlgr_opengl_setv(jlgr_t* jlgr, uint32_t* buff, uint32_t vertexAttrib,
	uint8_t xyzw);
void jlgr_opengl_vertices_(jlgr_t* jlgr, const float *xyzw, uint8_t vertices,
	float* cv, uint32_t* gl);
void jlgr_opengl_texture_bind_(jlgr_t* jlgr, uint32_t tex);
void jlgr_opengl_draw_arrays_(jlgr_t* jlgr, GLenum mode, uint8_t count);

//DL
void _jl_sg_loop(jlgr_t* jlgr);
float jl_sg_seconds_past_(jl_t* jlc);

// Resize function
void jl_wm_resz__(jlgr_t* jlgr, uint16_t x, uint16_t y);
void jl_sg_resz__(jl_t* jlc);
void jlgr_resz(jlgr_t* jlgr, uint16_t x, uint16_t y);
void jlgr_menu_resize_(jlgr_t* jlgr);
// init functions.
void jl_wm_init__(jlgr_t* jlgr);
void jlgr_text_init__(jlgr_t* jlgr);
void jl_sg_init__(jlgr_t* jlgr);
void jl_gl_init__(jlgr_t* jlgr);
void jlgr_init__(jlgr_t* jlgr);
void jl_ct_init__(jlgr_t* jlgr);
void jlgr_fl_init(jlgr_t* jlgr);
void jlgr_menubar_init__(jlgr_t* jlgr);
void jlgr_mouse_init__(jlgr_t* jlgr);
void jlgr_thread_init(jlgr_t* jlgr);
void jlgr_effects_init__(jlgr_t* jlgr);
// loop
void jl_ct_loop__(jlgr_t* jlgr);
void jl_wm_loop__(jlgr_t* jlgr);
void _jlgr_loopa(jlgr_t* jlgr);
// kill
void jl_wm_kill__(jlgr_t* jlgr);
void jlgr_thread_kill(jlgr_t* jlgr);
void jlgr_file_kill_(jlgr_t* jlgr);
//
void jlgr_thread_send(jlgr_t* jlgr,uint8_t id,uint16_t x,uint16_t y,jl_fnct fn);

//
void jl_wm_updatewh_(jlgr_t* jlgr);
