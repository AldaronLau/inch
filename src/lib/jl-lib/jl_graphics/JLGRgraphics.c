/*
 * JL_Lib
 * Copyright (c) 2015 Jeron A. Lau 
*/
/** \file
 * JLGRgraphics.c
 *	A High Level Graphics Library that supports sprites, texture loading,
 *	2D rendering & 3D rendering.
 */
#include "JLGRprivate.h"

typedef struct {
	jl_vec3_t where[2];
	jl_vo_t vo[3]; // Vertex object [ Full, Slider 1, Slider 2 ].
}jl_gui_slider_draw;

typedef struct {
	float* x1;
	float* x2;
	uint8_t isRange;
	jl_gui_slider_draw draw;
}jl_gui_slider_main;

/** @cond */

static inline void _jlgr_init_vos(jlgr_t* jlgr) {
	jlgr_vo_init(jlgr, &jlgr->gr.vos.whole_screen);
}

static void _jlgr_popup_loop(jl_t *jl) {
}

static void _jlgr_textbox_cm(jlgr_t* jlgr, jlgr_input_t input) {
	// TODO: Make Cursor Move
	// Left
	jl_ct_typing_disable();
	if(jlgr->gr.textbox_string->curs)
		jlgr->gr.textbox_string->curs--;
	// Right
	jl_ct_typing_disable();
	if(jlgr->gr.textbox_string->curs < jlgr->gr.textbox_string->size)
		jlgr->gr.textbox_string->curs++;
}

/** @endcond */

void jlgr_dont(jlgr_t* jlgr) { }

/**
 * Prepare to draw an image that takes up the entire pre-renderer or
 * screen.
 * @param jlgr: The library context.
 * @param g: the image group that the image pointed to by 'i' is in.
 * @param i:  the ID of the image.
 * @param c: is 0 unless you want to use the image as
 * 	a charecter map, then it will zoom into charecter 'chr'.
**/
void jlgr_fill_image_set(jlgr_t* jlgr, uint32_t tex, uint8_t w, uint8_t h, 
	int16_t c)
{
	jl_rect_t rc = { 0., 0., 1., jl_gl_ar(jlgr) };

	jlgr_vo_set_image(jlgr, &jlgr->gr.vos.whole_screen, rc, tex);
	jlgr_vo_txmap(jlgr, &jlgr->gr.vos.whole_screen, 0, 0, -1);
}

/**
 * Draw the image prepared with jlgr_fill_image_set().
 * @param jl: The library context.
**/
void jlgr_fill_image_draw(jlgr_t* jlgr) {
	jlgr_vo_draw(jlgr, &jlgr->gr.vos.whole_screen, NULL);
}

/**
 * Draw an integer on the screen
 * @param 'jl': library context
 * @param 'num': the number to draw
 * @param 'loc': the position to draw it at
 * @param 'f': the font to use.
 */
void jlgr_draw_int(jlgr_t* jlgr, int64_t num, jl_vec3_t loc, jl_font_t f) {
	char display[10];
	sprintf(display, "%ld", (long int) num);
	jlgr_text_draw(jlgr, display, loc, f);
}

/**
 * draw a floating point on the screen
 * @param 'jl': library context
 * @param 'num': the number to draw
 * @param 'dec': the number of places after the decimal to include.
 * @param 'loc': the position to draw it at
 * @param 'f': the font to use.
 */
void jlgr_draw_dec(jlgr_t* jlgr, double num, uint8_t dec, jl_vec3_t loc,
	jl_font_t f)
{
	char display[10];
	char convert[10];

	sprintf(convert, "%%%df", dec);
	sprintf(display, convert, num);
	jlgr_text_draw(jlgr, display, loc, f);
}

/**
 * Draw text within the boundary of a sprite
 * @param 'jl': library context
 * @param 'spr': the boundary sprite
 * @param 'txt': the text to draw
**/
void jlgr_text_draw_area(jlgr_t* jlgr, jl_sprite_t * spr, const char* txt) {
	float fontsize = .9 / strlen(txt);
	jlgr_text_draw(jlgr, txt,
		(jl_vec3_t) { .05,.5 * (jl_gl_ar(jlgr) - fontsize),0. },
		(jl_font_t) { jlgr->textures.icon, 0, jlgr->fontcolor, 
			fontsize});
}

/**
 * Draw a sprite, then draw text within the boundary of a sprite
 * @param 'jl': library context
 * @param 'spr': the boundary sprite
 * @param 'txt': the text to draw
**/
void jlgr_draw_text_sprite(jlgr_t* jlgr, jl_sprite_t* spr, const char* txt) {
	jlgr_fill_image_set(jlgr, jlgr->textures.icon, 16, 16, 1);
	jlgr_fill_image_draw(jlgr);
	jlgr_text_draw_area(jlgr, spr, txt);
}

/**
 * Draw centered text across screen
 * @param 'jl': library context.
 * @param 'str': the text to draw
 * @param 'yy': y coordinate to draw it at
 * @param 'color': 1.f = opaque, 0.f = invisible
 */
void jlgr_draw_ctxt(jlgr_t* jlgr, char *str, float yy, float* color) {
	jlgr_text_draw(jlgr, str,
		(jl_vec3_t) { 0., yy, 0. },
		(jl_font_t) { jlgr->textures.icon, 0, color, 
			1. / ((float)strlen(str))} );
}

// TODO: MOVE
static void jlgr_gui_slider_touch(jlgr_t* jlgr, jlgr_input_t input) {
	jl_sprite_t* spr = input.data;
	jl_gui_slider_main* slider = jlgr_sprite_getcontext(spr);

	if(jlgr_sprite_collide(jlgr, spr, &jlgr->mouse) == 0 ||
	 input.h == 0)
		return;
	float x = jlgr->main.ct.msx - (jl_gl_ar(jlgr) * .05 * spr->pr.cb.ofs.x);
	x -= spr->pr.cb.pos.x;
	x /= spr->pr.cb.ofs.x;
//		x += 1.5;// - (jl_gl_ar(jl->jlgr) * .1);
	if(x <= 0.) x = 0.;
	if(x > 1. - (jl_gl_ar(jlgr) * .15))
		x = 1. - (jl_gl_ar(jlgr) * .15);
//
	if(slider->isRange) {
		double v0 = fabs((*slider->x1) - x);
		double v1 = fabs((*slider->x2) - x);
		if(v1 < v0) {
			(*slider->x2) = x /
				(1. - (jl_gl_ar(jlgr) * .15));
			slider->draw.where[1].x = x;
		}else{
			(*slider->x1) = x /
				(1. - (jl_gl_ar(jlgr) * .15));
			slider->draw.where[0].x = x;
		}
	}else{
		(*slider->x1) = x / (1. - (jl_gl_ar(jlgr) * .15));
		slider->draw.where[0].x = x;
	}
	jlgr_sprite_redraw(jlgr, spr, &slider->draw);
}

static void jlgr_gui_slider_singleloop(jl_t* jl, jl_sprite_t* spr) {
	jlgr_input_do(jl->jlgr, JL_INPUT_PRESS, jlgr_gui_slider_touch, spr);
}

static void jlgr_gui_slider_doubleloop(jl_t* jl, jl_sprite_t* spr) {
	jlgr_input_do(jl->jlgr, JL_INPUT_PRESS, jlgr_gui_slider_touch, spr);
}

static void jlgr_gui_slider_draw(jl_t* jl, uint8_t resize, void* data) {
	jl_gui_slider_draw* slider = data;
	jlgr_t* jlgr = jl->jlgr;

	jl_rect_t rc = { 0.005, 0.005, .99, jl_gl_ar(jlgr) - .01 };
	jl_rect_t rc1 = { 0.0012, 0.0012, (jl_gl_ar(jlgr) * .5) + .0075,
		jl_gl_ar(jlgr) - .0024};
	jl_rect_t rc2 = { 0.005, 0.005, (jl_gl_ar(jlgr) * .5) -.001,
		jl_gl_ar(jlgr) - .01};
	float colors[] = { .06f, .04f, 0.f, 1.f };

	jl_gl_clear(jlgr, .01, .08, 0., 1.);
	jlgr_vo_set_image(jlgr, &(slider->vo[0]), rc, jlgr->textures.font);
	jlgr_vo_txmap(jlgr, &(slider->vo[0]), 	16, 16, 235);
	jlgr_vo_set_image(jlgr, &(slider->vo[1]), rc2, jlgr->textures.game);
	jlgr_vo_txmap(jlgr, &(slider->vo[1]), 	16, 16, 16);
	
	jlgr_vo_set_rect(jlgr, &(slider->vo[2]), rc1, colors, 0);
	// Draw Sliders
	jlgr_vo_draw(jlgr, &(slider->vo[0]), NULL);
	// Draw Slide 1
	jlgr_vo_draw(jlgr, &(slider->vo[2]), &slider->where[0]);
	jlgr_vo_draw(jlgr, &(slider->vo[1]), &slider->where[0]);
	// Draw Slide 2
	jlgr_vo_draw(jlgr, &(slider->vo[2]), &slider->where[1]);
	jlgr_vo_draw(jlgr, &(slider->vo[1]), &slider->where[1]);
}

/**
 * Create a slider sprite.
 * THREAD: Drawing thread only.
 * @param jlgr: The library context.
 * @param sprite: Uninitialized sprite to initialize.
 * @param rectange: Area to put the slider in.
 * @param isdouble: 1 to select range, 0 to select a specific value.
 * @param x1: Pointer to a permanent location for the slider value.
 * @param x2: Pointer to a permanent location for the second slider
	value.  Ignored if #isdouble is 0.
 * @returns: The slider sprite.
**/
void jlgr_gui_slider(jlgr_t* jlgr, jl_sprite_t* sprite, jl_rect_t rectangle,
	uint8_t isdouble, float* x1, float* x2)
{
	jlgr_sprite_loop_fnt jlgr_gui_slider_loop;

	if(isdouble) {
		jlgr_gui_slider_loop = jlgr_gui_slider_doubleloop;
	}else{
		jlgr_gui_slider_loop = jlgr_gui_slider_singleloop;
	}

	jl_gui_slider_main slider;

	slider.draw.where[0] = (jl_vec3_t) { 0., 0., 0. };
	slider.draw.where[1] = (jl_vec3_t) { 1. - (jl_gl_ar(jlgr) * .075),
		0., 0. };
	slider.x1 = x1, slider.x2 = x2;
	(*slider.x1) = 0.;
	(*slider.x2) = 1.;
	jlgr_vo_init(jlgr, &slider.draw.vo[0]);
	jlgr_vo_init(jlgr, &slider.draw.vo[1]);
	jlgr_vo_init(jlgr, &slider.draw.vo[2]);
	slider.isRange = isdouble;

	jlgr_sprite_init(jlgr, sprite, rectangle,
		jlgr_gui_slider_loop, jlgr_gui_slider_draw,
		&slider, sizeof(jl_gui_slider_main),
		&slider.draw, sizeof(jl_gui_slider_draw));
}

/**
 * Draw a background on the screen
**/
void jlgr_draw_bg(jlgr_t* jlgr, uint32_t tex, uint8_t w, uint8_t h, int16_t c) {
	jlgr_fill_image_set(jlgr, tex, w, h, c);
	jlgr_fill_image_draw(jlgr);
}

void jlgr_draw_loadingbar(jlgr_t* jlgr, double loaded) {
	jl_rect_t bar = { .05, jl_gl_ar(jlgr)*.4,
		.95,jl_gl_ar(jlgr)*.45};
	float colors[] = { 0., 1., 0., 1. };

	jlgr_vo_set_rect(jlgr, NULL, bar, colors, 0);
}

//TODO: MOVE
void jlgr_draw_msge__(jl_t* jl) {
	jlgr_t* jlgr = jl->jlgr;

	jlgr_draw_bg(jlgr, jlgr->gr.msge.t, 16, 16, jlgr->gr.msge.c);
	if(jlgr->gr.msge.message[0])
		jlgr_draw_ctxt(jlgr, jlgr->gr.msge.message, 9./32.,
			jlgr->fontcolor);
}

/**
 * Display on the screen.
 * @param jlgr: The library context.
 * @param draw_routine: Function that draws on screen.
**/
void jlgr_draw_loadscreen(jlgr_t* jlgr, jl_fnct draw_routine) {
	jlgr_redraw_t old_redrawfns = jlgr->draw.redraw;
	uint8_t inloop = jlgr->fl.inloop;

	// Set Graphical loops.
	jlgr->draw.redraw = (jlgr_redraw_t) {
		draw_routine, draw_routine,
		draw_routine, jl_dont };
	jlgr->fl.inloop = 1;
	// Update events ( minimal )
	jl_ct_quickloop_(jlgr);
	// Redraw screen.
	_jl_sg_loop(jlgr);
	// Update Screen.
	jl_wm_loop__(jlgr);
	//
	jlgr->draw.redraw = old_redrawfns;
	jlgr->fl.inloop = inloop;
}

/**
 * Print message on the screen.
 * @param jlgr: The library context.
 * @param tex: The background texture.
 * @param c: The character map setting.
 * @param format: The message
 */
void jlgr_draw_msge(jlgr_t* jlgr, uint32_t tex, uint8_t c, char* format, ...) {
	JL_PRINT_DEBUG(jlgr->jl, "jlgr_draw_msge");
	if(format) {
		va_list arglist;

		// Print on screen.
		va_start(arglist, format);
		vsprintf(jlgr->gr.msge.message, format, arglist);
		va_end(arglist);
	}else{
		jlgr->gr.msge.message[0] = '\0';
	}
	jl_print_function(jlgr->jl, "JLGR_MSGE");
	jlgr->gr.msge.t = tex;
	jlgr->gr.msge.c = c;
	JL_PRINT_DEBUG(jlgr->jl, "DRAW LOADSCREEN");
	
	jlgr_draw_loadscreen(jlgr, jlgr_draw_msge__);
	JL_PRINT_DEBUG(jlgr->jl, "DREW LOADSCREEN");
	jl_print_return(jlgr->jl, "JLGR_MSGE");
}

/**
 * Print a message on the screen and then terminate the program
 * @param 'jl': library context
 * @param 'message': the message 
 */
void jlgr_term_msge(jlgr_t* jlgr, char *message) {
	jlgr_draw_msge(jlgr, jlgr->textures.icon, 1, message);
	jl_print(jlgr->jl, message);
	exit(-1);
}

/**
 * Create a popup window.
 */
void jlgr_popup(jlgr_t* jlgr, char *name, char *message,
	jl_popup_button_t *btns, uint8_t opt_count)
{
	jlgr->gr.popup.window_name = name;
	jlgr->gr.popup.message = message;
	jlgr->gr.popup.btns = btns;
	jl_mode_override(jlgr->jl, (jl_mode_t)
		{jl_mode_exit, _jlgr_popup_loop, jl_dont});
}

/**
 * Re-draw/-size a slide button, and activate if it is pressed.
 * @param jlgr: The library context.
 * @param spr: The slide button sprite.
 * @param txt: The text to draw on the button.
**/
void jlgr_slidebtn_rsz(jlgr_t* jlgr, jl_sprite_t * spr, const char* txt) {
	jlgr_draw_text_sprite(jlgr, spr, txt);
}

/**
 * Run the Slide Button loop. ( activated when pressed, moved when
 *  hovered over. ) - And Draw Slide Button.
 * @param 'jl': the libary context
 * @param 'spr': the Slide Button Sprite.
 * @param 'defaultx': the default x position of the button.
 * @param 'slidex': how much the x should change when hovered above.
 * @param 'prun': the function to run when pressed.
**/
void jlgr_slidebtn_loop(jlgr_t* jlgr, jl_sprite_t * spr, float defaultx,
	float slidex, jlgr_input_fnct prun)
{
	spr->pr.cb.pos.x = defaultx;
	if(jlgr_sprite_collide(jlgr, &jlgr->mouse, spr)) {
		jlgr_input_do(jlgr, JL_INPUT_PRESS, prun, NULL);
		spr->pr.cb.pos.x = defaultx + slidex;
	}
	jlgr_sprite_draw(jlgr, spr);
}

/**
 * Draw a glow button, and activate if it is pressed.
 * @param 'jl': the libary context
 * @param 'spr': the sprite to draw
 * @param 'txt': the text to draw on the button.
 * @param 'prun': the function to run when pressed.
**/
void jlgr_glow_button_draw(jlgr_t* jlgr, jl_sprite_t * spr,
	char *txt, jlgr_input_fnct prun)
{
//		jlgr_sprite_redraw(jlgr, spr);
	jlgr_sprite_draw(jlgr, spr);
	if(jlgr_sprite_collide(jlgr, &jlgr->mouse, spr)) {
		jl_rect_t rc = { spr->pr.cb.pos.x, spr->pr.cb.pos.y,
			spr->pr.cb.ofs.x, spr->pr.cb.ofs.y };
		float glow_color[] = { 1., 1., 1., .25 };

		// Draw glow
		jlgr_vo_set_rect(jlgr, &jlgr->gl.temp_vo, rc, glow_color, 0);
		jlgr_vo_draw(jlgr, &jlgr->gl.temp_vo, NULL);
		// Description
		jlgr_text_draw(jlgr, txt,
			(jl_vec3_t)
				{0., jl_gl_ar(jlgr) - .0625, 0.},
			(jl_font_t) { jlgr->textures.icon, 0,
				jlgr->fontcolor, .05 });
		// Run if press
		jlgr_input_do(jlgr, JL_INPUT_PRESS, prun, NULL);
	}
}

/**
 * Draw A Textbox.
 * @return 1 if return/enter is pressed.
 * @return 0 if not.
*/
uint8_t jlgr_draw_textbox(jlgr_t* jlgr, float x, float y, float w,
	float h, data_t* string)
{
	uint8_t bytetoinsert = 0;

	jlgr->gr.textbox_string = string;
	if((bytetoinsert = jl_ct_typing_get(jlgr))) {
		if(bytetoinsert == '\b') {
			if(string->curs == 0) return 0;
			string->curs--;
			jl_data_delete_byte(jlgr->jl, string);
		}else if(bytetoinsert == '\02') {
			jl_data_delete_byte(jlgr->jl, string);
		}else if(bytetoinsert == '\n') {
			return 1;
		}else{
			jl_data_insert_byte(jlgr->jl, string,
				 bytetoinsert);
		}
//			JL_PRINT("inserting %1s\n", &bytetoinsert);
	}
	jlgr_input_do(jlgr, JL_INPUT_JOYC, _jlgr_textbox_cm, NULL);
//		jlgr_draw_image(jl, 0, 0, x, y, w, h, ' ', 1.);
	jlgr_text_draw(jlgr, (char*)(string->data),
		(jl_vec3_t) {x, y, 0.},
		(jl_font_t) {jlgr->textures.icon,0,jlgr->fontcolor,h});
//		jlgr_draw_image(jl, 0, 0,
//			x + (h*((float)string->curs-.5)), y, h, h, 252, 1.);
	return 0;
}

/**
 * THREAD: Main thread.
 * Pop-Up a notification bar.
 * @param jl: the libary context
 * @param notification: The message to display.
*/
void jlgr_notify(jlgr_t* jlgr, const char* notification) {
	jlgr_comm_notify_t packet;
	packet.id = JLGR_COMM_NOTIFY;
	jl_mem_copyto(notification, packet.string, 256);
	packet.string[strlen(notification)] = '\0';

	jl_thread_comm_send(jlgr->jl, jlgr->comm2draw, &packet);
}

/***      @cond       ***/
/************************/
/***  ETOM Functions  ***/
/************************/

void _jlgr_loopb(jlgr_t* jlgr) {
	//Message Display
	if(jlgr->gr.notification.timeTilVanish > 0.f) {
		if(jlgr->gr.notification.timeTilVanish > .5) {
			float color[] = { 1., 1., 1., 1. };
			jlgr_draw_ctxt(jlgr, jlgr->gr.notification.message, 0,
				color);
		}else{
			float color[] = { 1., 1., 1.,
				(jlgr->gr.notification.timeTilVanish / .5)};
			jlgr_draw_ctxt(jlgr, jlgr->gr.notification.message, 0,
				color);
		}
		jlgr->gr.notification.timeTilVanish-=jlgr->psec;
	}
}

void _jlgr_loopa(jlgr_t* jlgr) {
	if(!jlgr->menubar.menubar.mutex) return;
	jl_print_function(jlgr->jl, "GR_LP");
	// Draw the pre-rendered Menubar.
	if(!jlgr->fl.inloop) jlgr_sprite_draw(jlgr, &jlgr->menubar.menubar);
	// Update messages.
	_jlgr_loopb(jlgr);
	jl_print_return(jlgr->jl, "GR_LP");
	// Draw mouse
	if(jlgr->mouse.mutex) jlgr_sprite_draw(jlgr, &jlgr->mouse);
}

void jlgr_init__(jlgr_t* jlgr) {
	data_t packagedata;

	jl_data_mkfrom_data(jlgr->jl, &packagedata, jl_gem_size(), jl_gem());
	_jlgr_init_vos(jlgr);
	jlgr->textures.logo = jl_sg_add_image(jlgr, &packagedata,
		"/images/JL_Lib.png");
	JL_PRINT_DEBUG(jlgr->jl, "Draw Loading Screen");
	jlgr_draw_msge(jlgr, jlgr->textures.logo, 0, 0);
	JL_PRINT_DEBUG(jlgr->jl, "Drew Loading Screen");
	// Load Graphics
	jlgr->textures.font = jl_sg_add_image(jlgr, &packagedata,
		"/images/font.png");
	// Create Font
	jlgr->fontcolor[0] = 0.;
	jlgr->fontcolor[1] = 0.;
	jlgr->fontcolor[2] = 0.;
	jlgr->fontcolor[3] = 1.;
	jlgr->font = (jl_font_t)
		{ jlgr->textures.font, 0, jlgr->fontcolor, .04 };
	// Draw message on the screen
	jlgr_draw_msge(jlgr, jlgr->textures.logo, 0, "LOADING JL_LIB....");
	// Set other variables
	jlgr->gr.notification.timeTilVanish = 0.f;
	// Load other images....
	jlgr->textures.icon = jl_sg_add_image(jlgr, &packagedata,
		"/images/taskbar_items.png");
	jlgr->textures.game = jl_sg_add_image(jlgr, &packagedata,
		"/images/landscape.png");
}

/**      @endcond      **/
/***   #End of File   ***/
