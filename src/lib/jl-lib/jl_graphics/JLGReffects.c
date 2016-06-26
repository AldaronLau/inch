#include "JLGRinternal.h"

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
	"	gl_FragColor = new_color * \n"
	"		vec4(grayscale, grayscale, grayscale, vcolor.a);\n"
	"}";

void jlgr_effects_init__(jlgr_t* jlgr) {
	JL_PRINT_DEBUG(jlgr->jl, "MAKING EFFECT: ALPHA");
	jlgr_gl_shader_init(jlgr, &jlgr->effects.alpha, NULL,
		JL_EFFECT_ALPHA, "multiply_alpha");
	JL_PRINT_DEBUG(jlgr->jl, "MAKING EFFECT: HUE");
	jlgr_gl_shader_init(jlgr, &jlgr->effects.hue, NULL,
		JL_EFFECT_HUE, "new_color");
}
