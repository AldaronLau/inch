#include "JLGRprivate.h"

/** @cond */

const char *JL_EFFECT_ALPHA = 
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"uniform float multiply_alpha;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main() {\n"
	"	vec4 vcolor = texture2D(texture, texcoord);\n"
	"	gl_FragColor = vec4(vcolor.rgb, vcolor.a * multiply_alpha);\n"
	"}";

const char *JL_EFFECT_HUE = 
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"uniform vec4 new_color;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main() {\n"
	"	vec4 vcolor = texture2D(texture, texcoord);\n"
	"	float grayscale = (vcolor.r + vcolor.g + vcolor.b) / 3.f;\n"
	"	gl_FragColor = \n"
	"		vec4(new_color.r * grayscale, new_color.g * grayscale,"
	"		new_color.b * grayscale, new_color.a * vcolor.a);\n"
	"}";

static void jlgr_effect_pr_hue__(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;
	jlgr_vo_set_image(jlgr, &jlgr->gl.temp_vo, (jl_rect_t) {
		0., 0., 1., jl_gl_ar(jlgr) }, jlgr->gl.cp->tx);
	jlgr_effects_vo_hue(jlgr, &jlgr->gl.temp_vo, (jl_vec3_t) {
		0.f, 0.f, 0.f }, jlgr->effects.colors);
}

/** @endcond */

/**
 * Set the effect uniform variable in a shader to a float.
 * @param jlgr: The library context.
 * @param sh: The shader object.
 * @param x: The float value.
**/
void jlgr_effects_uniform1(jlgr_t* jlgr, jlgr_glsl_t* sh, float x) {
	jlgr_opengl_uniform1f_(jlgr, sh->uniforms.newcolor_malpha, x);
}

/**
 * Set the effect uniform variable in a shader to a vec3.
 * @param jlgr: The library context.
 * @param sh: The shader object.
 * @param x, y, z: The vec3 value.
**/
void jlgr_effects_uniform3(jlgr_t* jlgr, jlgr_glsl_t* sh, float x, float y,
	float z)
{
	jlgr_opengl_uniform3f_(jlgr, sh->uniforms.newcolor_malpha, x, y, z);
}

/**
 * Set the effect uniform variable in a shader to a vec4.
 * @param jlgr: The library context.
 * @param sh: The shader object.
 * @param x, y, z, w: The vec4 value.
**/
void jlgr_effects_uniform4(jlgr_t* jlgr, jlgr_glsl_t* sh, float x, float y,
	float z, float w)
{
	jlgr_opengl_uniform4f_(jlgr, sh->uniforms.newcolor_malpha, x, y, z, w);
}

/**
 * Draw a vertex object with alpha effect.
 * @param jlgr: The library context.
 * @param vo: The vertex object to draw.
 * @param offs: The offset vector to translate by.
 * @param a: The alpha value to multiply each pixel by. [ 0.f - 1.f ]
**/
void jlgr_effects_vo_alpha(jlgr_t* jlgr, jl_vo_t* vo, jl_vec3_t offs, float a) {
	// Bind shader
	jlgr_opengl_draw1(jlgr, &jlgr->effects.alpha);
	// Translate by offset vector
	jlgr_opengl_transform_(jlgr, &jlgr->effects.alpha,
		offs.x, offs.y, offs.z, 1., 1., 1., jl_gl_ar(jlgr));
	// Set Alpha Value In Shader
	jlgr_effects_uniform1(jlgr, &jlgr->effects.alpha, a);
	// Draw on screen
	jlgr_vo_draw2(jlgr, vo, &jlgr->effects.alpha);
}

/**
 * Draw a vertex object, changing te hue of each pixel.
 * @param jlgr: The library context.
 * @param vo: The vertex object to draw.
 * @param offs: The offset to draw it at.
 * @param c: The new hue ( r, g, b, a ) [ 0.f - 1.f ]
**/
void jlgr_effects_vo_hue(jlgr_t* jlgr, jl_vo_t* vo, jl_vec3_t offs, float c[]) {
	// Bind shader
	jlgr_opengl_draw1(jlgr, &jlgr->effects.hue);
	// Translate by offset vector
	jlgr_opengl_transform_(jlgr, &jlgr->effects.hue,
		offs.x, offs.y, offs.z, 1., 1., 1., jl_gl_ar(jlgr));
	// Set Hue Value In Shader
	jlgr_effects_uniform4(jlgr, &jlgr->effects.hue, c[0], c[1], c[2], c[3]);
	// Draw on screen
	jlgr_vo_draw2(jlgr, vo, &jlgr->effects.hue);
}

void jlgr_effects_alpha(jlgr_t* jlgr, float a) {
	
}

/**
 * Apply a hue change effect to everything that's been drawn on the current pre-
 * renderer so far.
 * @param jlgr: The library context.
 * @param c: The hue to make everything.
**/
void jlgr_effects_hue(jlgr_t* jlgr, float c[]) {
	jl_mem_copyto(c, jlgr->effects.colors, sizeof(float) * 4);
	jlgr_pr(jlgr, jlgr->gl.cp, jlgr_effect_pr_hue__);
}

void jlgr_effects_init__(jlgr_t* jlgr) {
	JL_PRINT_DEBUG(jlgr->jl, "MAKING EFFECT: ALPHA");
	jlgr_gl_shader_init(jlgr, &jlgr->effects.alpha, NULL,
		JL_EFFECT_ALPHA, "multiply_alpha");
	JL_PRINT_DEBUG(jlgr->jl, "MAKING EFFECT: HUE");
	jlgr_gl_shader_init(jlgr, &jlgr->effects.hue, NULL,
		JL_EFFECT_HUE, "new_color");
	JL_PRINT_DEBUG(jlgr->jl, "MADE EFFECTS!");
}
