/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRmenu.c
 *	This file handles the menubar.
**/
#include "JLGRinternal.h"

typedef struct{
	// Used for all icons on the menubar.
	jl_vo_t icon;
	jl_vo_t shadow;
	// Redraw Functions for 10 icons.
	jlgr_fnct redrawfn[10];
	// Draw thread Cursor
	int8_t cursor;
}jl_menu_draw_t;

typedef struct{
	// Pressed & Not Pressed Functions for 10 icons.
	jlgr_input_fnct inputfn[10];
	// Main thread cursor
	int8_t cursor;
	// Draw context
	jl_menu_draw_t draw;
}jl_menu_t;

char *GMessage[3] = {
	"SCREEN: UPPER",
	"SCREEN: LOWER",
	"SCREEN: SINGLE"
};

// Run when the menubar is clicked/pressed
static void jlgr_menu_loop_press__(jlgr_t* jlgr, jlgr_input_t input) {
	jl_menu_t *menu = jlgr_sprite_getcontext(&jlgr->menubar.menubar);
	// Figure out what's selected.
	const uint8_t selected = (uint8_t)((1. - jlgr->main.ct.msx) / .1);

	for(menu->cursor = 0; menu->cursor < 10; menu->cursor++){
		// If A NULL function then, stop looping menubar.
		if( !(menu->inputfn[menu->cursor]) ) break;
		// Run the input loop.
		if(menu->cursor == selected && jlgr->main.ct.msy < .1)
			menu->inputfn[menu->cursor](jlgr, input);
	}
}

static inline void jlgr_menubar_shadow__(jlgr_t* jlgr,jl_menu_draw_t* menu_draw)
{
	// Clear Texture.
	jl_gl_clear(jlgr, 0., 0., 0., 0.);
	// Draw Shadows.
	for(menu_draw->cursor = 0; menu_draw->cursor < 10; menu_draw->cursor++){
		jl_vec3_t tr = { .9 - (.1 * menu_draw->cursor), 0., 0. };
		jlgr_fnct _draw_icon_=menu_draw->redrawfn[menu_draw->cursor];

		if(_draw_icon_ == NULL) break;
		// Draw shadow
		jlgr_draw_vo(jlgr, &menu_draw->shadow, &tr);
		// Draw Icon
		_draw_icon_(jlgr);
	}
}

// Run whenever a redraw is needed for an icon.
static void jlgr_menubar_draw_(jl_t* jl, uint8_t resize, void* ctx_draw) {
	jlgr_t* jlgr = jl->jlgr;
	jl_menu_draw_t* menu_draw = ctx_draw;

	// If needed, draw shadow.
	if(menu_draw->cursor < 0) {
		// Complete redraw of taskbar.
		jlgr_menubar_shadow__(jlgr, menu_draw);
		// Set redraw = true.
		menu_draw->cursor = 0;
		while(menu_draw->redrawfn[menu_draw->cursor]) {
			jlgr_menubar_draw_(jl, resize, menu_draw);
			menu_draw->cursor++;
		}
	}else{
		// Redraw only the selected icon.
		if(menu_draw->redrawfn[menu_draw->cursor]) {
			menu_draw->redrawfn[menu_draw->cursor](jlgr);
		}
	}
}

// Runs every frame when menubar is visible.
static void jlgr_menubar_loop_(jl_t* jl, jl_sprite_t* sprite) {
	jlgr_t* jlgr = jl->jlgr;
	jl_menu_t *menu = jlgr_sprite_getcontext(&jlgr->menubar.menubar);

	// Run the proper loops.
	jlgr_input_do(jlgr, JL_INPUT_PRESS, jlgr_menu_loop_press__, NULL);
	//
	if(menu->draw.cursor == -1) {
		jlgr_sprite_redraw(jlgr, &jlgr->menubar.menubar, &menu->draw);
		menu->draw.cursor = 0;
	}
}

void jlgr_menubar_init__(jlgr_t* jlgr) {
	jl_rect_t rc = { 0.f, 0.f, 1.f, .11f };
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_rect_t rc_shadow = { -.01, .01, .1, .1 };
	float shadow_color[] = { 0.f, 0.f, 0.f, .5f };
	jl_menu_t menu;

	jl_gl_vo_init(jlgr, &menu.draw.icon);
	jl_gl_vo_init(jlgr, &menu.draw.shadow);

	// Make the shadow vertex object.
	jlgr_vos_rec(jlgr, &menu.draw.shadow, rc_shadow, shadow_color, 0);
	// Make the icon vertex object.
	jlgr_vos_image(jlgr, &menu.draw.icon, rc_icon, jlgr->textures.icon, 1.);
	jl_gl_vo_txmap(jlgr, &menu.draw.icon, 16, 16, JLGR_ID_UNKNOWN);
	// Clear the menubar & make pre-renderer.
	for( menu.draw.cursor = 0; menu.draw.cursor < 10;
		menu.draw.cursor++)
	{
		menu.inputfn[menu.draw.cursor] = NULL;
		menu.draw.redrawfn[menu.draw.cursor] = NULL;
	}
	menu.draw.cursor = -1;
	// Make the menubar.
	jlgr_sprite_init(jlgr, &jlgr->menubar.menubar, rc,
		jlgr_menubar_loop_, jlgr_menubar_draw_,
		&menu, sizeof(jl_menu_t),
		&menu.draw, sizeof(jl_menu_draw_t));
	// Redraw menubar.
	// Set the loop.
	jlgr->menubar.menubar.loop = jlgr_menubar_loop_;
}

static void jlgr_menubar_text__(jlgr_t* jlgr,float* color,float y,str_t text) {
	jl_menu_draw_t* menu_draw = jlgr_sprite_getdrawctx(&jlgr->menubar.menubar);
	jl_vec3_t tr = { .9 - (.1 * menu_draw->cursor), y, 0. };

	jlgr_draw_text(jlgr, text, tr,
		(jl_font_t) { jlgr->textures.icon, 0, color, 
			.1 / strlen(text)});
}

static void jlgr_menu_flip_draw__(jlgr_t* jlgr) {
	jlgr_menu_draw_icon(jlgr, jlgr->textures.icon, JLGR_ID_FLIP_IMAGE);
}

static void jlgr_menu_flip_press__(jlgr_t* jlgr, jlgr_input_t input) {
	if(input.h != JLGR_INPUT_PRESS_JUST) return;
	// Actually Flip the screen.
	if(jlgr->sg.cs == JL_SCR_UP) {
		jlgr->sg.cs = JL_SCR_SS;
	}else if(jlgr->sg.cs == JL_SCR_DN) {
		jlgr->sg.cs = JL_SCR_UP;
	}else{
		jlgr->sg.cs = JL_SCR_DN;
	}
	jlgr_notify(jlgr, GMessage[jlgr->sg.cs]);
	jlgr_resz(jlgr, 0, 0);
}

static void jlgr_menu_name_draw2__(jlgr_t* jlgr) {
	jlgr_menu_draw_icon(jlgr, jlgr->textures.icon, JLGR_ID_UNKNOWN);
}

static void jlgr_menu_name_draw__(jlgr_t* jlgr) {
	jl_menu_t* menu = jlgr_sprite_getcontext(&jlgr->menubar.menubar);
	float text_size = jl_gl_ar(jlgr) * .5;

	jlgr_menu_name_draw2__(jlgr);
	jlgr_draw_text(jlgr, jlgr->wm.windowTitle[0],
		(jl_vec3_t) { 1. - (jl_gl_ar(jlgr) * (menu->draw.cursor+1.)),
			0., 0. },
		(jl_font_t) { jlgr->textures.icon, 0, jlgr->fontcolor, 
			text_size});
	jlgr_draw_text(jlgr, jlgr->wm.windowTitle[1],
		(jl_vec3_t) { 1. - (jl_gl_ar(jlgr) * (menu->draw.cursor+1.)),
			text_size, 0. },
		(jl_font_t) { jlgr->textures.icon, 0, jlgr->fontcolor, 
			text_size});
}

static void jlgr_menu_slow_draw__(jlgr_t* jlgr) {
	jl_t* jl = jlgr->jl;
	float color[] = { .5, .5, 1., 1. };
	char formated[80];

	// Draw the icon based on whether on time or not.
	jlgr_menu_draw_icon(jlgr, jlgr->textures.icon, jlgr->sg.on_time ?
		JLGR_ID_GOOD_IMAGE : JLGR_ID_SLOW_IMAGE);
	// Report the seconds that passed.
	jl_mem_format(formated, "DrawFPS:%d", (int)(1. / jlgr->psec));
	jlgr_menubar_text__(jlgr, color, 0., formated);
	jl_mem_format(formated, "MainFPS:%d", (int)(1. / jl->time.psec));
	jlgr_menubar_text__(jlgr, color, .05, formated);
}

static void jlgr_menu_slow_loop__(jlgr_t* jlgr, jlgr_input_t input) {
	jl_menu_t* menu = jlgr_sprite_getcontext(&jlgr->menubar.menubar);
	menu->draw.cursor = menu->cursor;

	jlgr_sprite_redraw(jlgr, &jlgr->menubar.menubar, &menu->draw);
}

void jlgr_menu_resize_(jlgr_t* jlgr) {
	if(jlgr->menubar.menubar.mutex) {
		jl_menu_draw_t* menu_draw =
			jlgr_sprite_getdrawctx(&jlgr->menubar.menubar);
		menu_draw->cursor = -1;
		jlgr_sprite_resize(jlgr, &jlgr->menubar.menubar, NULL);
	}
}

//
// Exported Functions
//

/**
 * Toggle whether or not to show the menu bar.
 *
 * @param jlgr: The library context
**/
void jlgr_menu_toggle(jlgr_t* jlgr) {
	if(jlgr->menubar.menubar.loop == jlgr_sprite_dont)
		jlgr->menubar.menubar.loop = jlgr_menubar_loop_;
	else
		jlgr->menubar.menubar.loop = jlgr_sprite_dont;
}

void jlgr_menu_draw_icon(jlgr_t* jlgr, uint32_t tex, uint8_t c) {
	jl_rect_t rc_icon = { 0., 0., .1, .1};
	jl_menu_draw_t* menu_draw = jlgr_sprite_getdrawctx(&jlgr->menubar.menubar);
	jl_vec3_t tr = { .9 - (.1 * menu_draw->cursor), 0., 0. };

	jlgr_vos_image(jlgr, &menu_draw->icon, rc_icon, tex, 1.);
	jl_gl_vo_txmap(jlgr, &menu_draw->icon, 16, 16, c);
	jlgr_draw_vo(jlgr, &menu_draw->icon, &tr);
}

/**
 * Add an icon to the menubar
 *
 * @param jlgr: the libary context
 * @param inputfn: The function to run when the icon is / isn't pressed.
 * @param rdr: the function to run when redraw is called.
**/
void jlgr_menu_addicon(jlgr_t* jlgr, jlgr_input_fnct inputfn, jlgr_fnct rdr) {
	jl_menu_t* menu = jlgr_sprite_getcontext(&jlgr->menubar.menubar);
	uint8_t i;

	menu->draw.cursor = -1;
	for(i = 0; i < 10; i++) if(!menu->inputfn[i]) break;
	// Set functions for: draw, press, not press
	menu->inputfn[i] = inputfn;
	menu->draw.redrawfn[i] = rdr;
}

/**
 * Add the flip screen icon to the menubar.
 * @param jlgr: the libary context
**/
void jlgr_menu_addicon_flip(jlgr_t* jlgr) {
	jlgr_menu_addicon(jlgr, jlgr_menu_flip_press__, jlgr_menu_flip_draw__);	
}

/**
 * Add slowness detector to the menubar.
 * @param jlgr: the libary context
**/
void jlgr_menu_addicon_slow(jlgr_t* jlgr) {
	jlgr_menu_addicon(jlgr, jlgr_menu_slow_loop__, jlgr_menu_slow_draw__);
}

/**
 * Add program title to the menubar.
 * @param jlgr: the libary context
**/
void jlgr_menu_addicon_name(jlgr_t* jlgr) {
	int i;
	for(i = 0; i < 4; i++) {
		jlgr_menu_addicon(jlgr, jlgr_input_dont, jlgr_menu_name_draw2__);
	}
	jlgr_menu_addicon(jlgr, jlgr_input_dont, jlgr_menu_name_draw__);
}
