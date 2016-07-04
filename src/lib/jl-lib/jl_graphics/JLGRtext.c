/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRtext.c
 *	Draw text on screen.
 */
#include "JLGRprivate.h"

/**
 * Draw text on the current pre-renderer.
 * @param jlgr: The library context
 * @param str: The text to draw
 * @param loc: The position to draw it at
 * @param f: The font to use.
**/
void jlgr_text_draw(jlgr_t* jlgr, const char* str, jl_vec3_t loc, jl_font_t f) {
	if(str == NULL) return;

	const uint8_t *text = (void*)str;
	uint32_t i;
	jl_rect_t rc = { loc.x, loc.y, f.size, f.size };
	jl_vec3_t tr = { 0., 0., 0. };
	jl_vo_t* vo = &jlgr->gl.temp_vo;

	jlgr_vo_set_image(jlgr, vo, rc, jlgr->textures.font);
	for(i = 0; i < strlen(str); i++) {
		if(text[i] == '\n') {
			tr.x = 0, tr.y += f.size;
			continue;
		}
		//Font 0:0
		jlgr_vo_txmap(jlgr, vo, 16, 16, text[i]);
		jlgr_effects_vo_hue(jlgr, vo, tr, f.colors);
		tr.x += f.size * ( 3. / 4. );
	}
}

void jlgr_text_init__(jlgr_t* jlgr) {
}
