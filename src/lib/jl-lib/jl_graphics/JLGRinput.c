/*
 * (c) Jeron A. Lau
 *
 * jl/input is a library for making input controllers compatible between
 * different
 * devices.
*/

#include "JLGRinternal.h"

#if JL_PLAT == JL_PLAT_COMPUTER
	#define SDL_MENU_KEY SDL_SCANCODE_APPLICATION
#elif JL_PLAT == JL_PLAT_PHONE
	#define SDL_MENU_KEY SDL_SCANCODE_MENU
#endif

/*
 * "Preval" is a pointer to previous key pressed value.
 *	 0 if key isn't pressed
 *	 1 if key is just pressed
 *	 2 if key is held down
 *	 3 if key is released
 * "read" is the input variable.
*/
static void jl_ct_state__(int8_t *preval, const uint8_t read) {
	if(*preval == JLGR_INPUT_PRESS_ISNT || *preval == JLGR_INPUT_PRESS_STOP)
		//If wasn't pressed: Just Pressed / Not pressed
		*preval = read ? JLGR_INPUT_PRESS_JUST : JLGR_INPUT_PRESS_ISNT;
	else if(read ==  0 && (*preval == JLGR_INPUT_PRESS_JUST ||
	 *preval == JLGR_INPUT_PRESS_HELD))
		//If Was Held Down And Now Isnt | 3: Release
		*preval = JLGR_INPUT_PRESS_STOP;
	else
		//2: Held Down
		*preval = JLGR_INPUT_PRESS_HELD;
}

static void jlgr_input_state__(jlgr_t* jlgr, uint8_t read) {
	if(jlgr->input.used[jlgr->main.ct.current_event] == 0) {
		jl_ct_state__(&jlgr->input.states[jlgr->main.ct.current_event],
			read);
		jlgr->input.used[jlgr->main.ct.current_event] = 1;
	}
}

/*
 * Returns 0 if key isn't pressed
 * Returns 1 if key is just pressed
 * Returns 2 if key is held down
 * Returns 3 if key is released
*/
uint8_t jl_ct_key_pressed__(jlgr_t *jlgr, uint8_t key) {
	jl_ct_state__(&jlgr->main.ct.keyDown[key], jlgr->main.ct.keys[key]);
	return jlgr->main.ct.keyDown[key];
}

static void jlgr_input_press2__(jlgr_t* jlgr, uint8_t countit) {
	jlgr_input_state__(jlgr, countit);
	//hrxypk
	jlgr->input.input.h = jlgr->input.states[jlgr->main.ct.current_event];
	jlgr->input.input.r = 0.;
	jlgr->input.input.x = jlgr->main.ct.msx;
	jlgr->input.input.y = jlgr->main.ct.msy;
	if(countit) {
		jlgr->input.input.k = 1;
		jlgr->input.input.p = 1.;
	}else{
		jlgr->input.input.k = 0;
		jlgr->input.input.p = 0.;
	}
}

void jl_ct_key(jlgr_t *jlgr, jlgr_input_fnct inputfn, uint8_t key) {
	uint8_t a = jl_ct_key_pressed__(jlgr, key);

	jlgr->input.input.h = a;
	jlgr->input.input.r = 0.;
	if(a && (a!=3)) {
		jlgr->input.input.p = 1.;
		jlgr->input.input.k = key;
	}else{
		jlgr->input.input.p = 0.;
		jlgr->input.input.k = 0;
	}
	inputfn(jlgr, jlgr->input.input);
}

static void jlgr_input_fourdir(jlgr_t *jlgr, jlgr_input_fnct inputfn,
	uint8_t up, uint8_t dn, uint8_t lt, uint8_t rt)
{
	uint8_t key;
	// XY
	if(up) {
		key = JLGR_INPUT_DIR_UP;
		jlgr->input.input.x = 0.;
		jlgr->input.input.y = -1.;
	}else if(dn) {
		key = JLGR_INPUT_DIR_DN;
		jlgr->input.input.x = 0.;
		jlgr->input.input.y = 1.;
	}else if(lt) {
		key = JLGR_INPUT_DIR_LT;
		jlgr->input.input.x = -1.;
		jlgr->input.input.y = 0.;
	}else if(rt) {
		key = JLGR_INPUT_DIR_RT;
		jlgr->input.input.x = 1.;
		jlgr->input.input.y = 0.;
	}else{
		key = JLGR_INPUT_DIR_NO;
		jlgr->input.input.x = 0.;
		jlgr->input.input.y = 0.;
	}
	// HRPK
	jlgr_input_state__(jlgr, key);
	jlgr->input.input.h = jlgr->input.states[jlgr->main.ct.current_event];
	jlgr->input.input.r = 0.;
	jlgr->input.input.p = key ? 1. : 0.;
	jlgr->input.input.k = key ? key : 0;
	inputfn(jlgr, jlgr->input.input);
}

// Get Input

static void jlgr_input_return__(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
	jl_ct_key(jlgr, inputfn, SDL_SCANCODE_RETURN);
}

static void jlgr_input_arrow__(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
	jlgr_input_fourdir(jlgr, inputfn,
		jlgr->main.ct.keys[SDL_SCANCODE_UP],
		jlgr->main.ct.keys[SDL_SCANCODE_DOWN],
		jlgr->main.ct.keys[SDL_SCANCODE_LEFT],
		jlgr->main.ct.keys[SDL_SCANCODE_RIGHT]);
}

static void jlgr_input_wasd__(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
	jlgr_input_fourdir(jlgr, inputfn,
		jlgr->main.ct.keys[SDL_SCANCODE_W],
		jlgr->main.ct.keys[SDL_SCANCODE_S],
		jlgr->main.ct.keys[SDL_SCANCODE_A],
		jlgr->main.ct.keys[SDL_SCANCODE_D]);
}

/*void jlgr_input_touch_center(jlgr_t *jlgr, jlgr_input_fnct inputfn) { //touch center
	jlgr_input_press2__(jlgr, jlgr->main.ct.input.click &&
		(jlgr->main.ct.msy > .4f * jlgr->wm.ar) &&
		(jlgr->main.ct.msy < .6f * jlgr->wm.ar) &&
		(jlgr->main.ct.msx > .4f) &&
		(jlgr->main.ct.msx < .6f));
	inputfn(jlgr, jlgr->input.input);
}*/

void jlgr_input_dirnear__(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
	uint8_t near_right = jlgr->main.ct.input.click &&
		(jlgr->main.ct.msx > .6f) &&
		(jlgr->main.ct.msx < .8f) &&
		(jlgr->main.ct.msy > .2f * jlgr->wm.ar) &&
		(jlgr->main.ct.msy < .8f * jlgr->wm.ar);
	uint8_t near_left = jlgr->main.ct.input.click &&
		(jlgr->main.ct.msx < .4f) &&
		(jlgr->main.ct.msx > .2f) &&
		(jlgr->main.ct.msy > .2f * jlgr->wm.ar) &&
		(jlgr->main.ct.msy < .8f * jlgr->wm.ar);
	uint8_t near_up = jlgr->main.ct.input.click &&
		(jlgr->main.ct.msy < .4f * jlgr->wm.ar) &&
		(jlgr->main.ct.msy > .2f * jlgr->wm.ar) &&
		(jlgr->main.ct.msx >.2f) &&
		(jlgr->main.ct.msx <.8f);
	uint8_t near_down = jlgr->main.ct.input.click &&
		(jlgr->main.ct.msy > .6f * jlgr->wm.ar) &&
		(jlgr->main.ct.msy < .8f * jlgr->wm.ar) &&
		(jlgr->main.ct.msx > .2f) &&
		(jlgr->main.ct.msx < .8f);

	jlgr_input_fourdir(jlgr,inputfn,near_up,near_down,near_left,near_right);
}

void jlgr_input_dirfar__(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
	uint8_t far_right = jlgr->main.ct.input.click &&
		(jlgr->main.ct.msx>.8f);
	uint8_t far_left = jlgr->main.ct.input.click &&
		(jlgr->main.ct.msx<.2f);
	uint8_t far_up = jlgr->main.ct.input.click &&
		(jlgr->main.ct.msy<.2f * jlgr->wm.ar);
	uint8_t far_down = jlgr->main.ct.input.click &&
		(jlgr->main.ct.msy>.8f * jlgr->wm.ar);

	jlgr_input_fourdir(jlgr,inputfn, far_up, far_down, far_left, far_right);
}

void jlgr_input_press__(jlgr_t *jlgr, jlgr_input_fnct inputfn) {//Any touch
	jlgr_input_press2__(jlgr, jlgr->main.ct.input.click);
	inputfn(jlgr, jlgr->input.input);
}

//

void jlgr_input_dont(jlgr_t* jlgr, jlgr_input_t input) { }

void jl_ct_key_menu(jlgr_t *jlgr, jlgr_input_fnct inputfn) {
	jl_ct_key(jlgr, inputfn, SDL_MENU_KEY); //xyrhpk
}

void jl_ct_txty(void) {
	SDL_StartTextInput();
}

void jl_ct_txtn(void) {
	SDL_StopTextInput();
}

static inline void jlgr_input_handle_events_platform_dependant__(jlgr_t* jlgr) {
#if JL_PLAT == JL_PLAT_PHONE
	if( jlgr->main.ct.event.type==SDL_FINGERDOWN ) {
		jlgr->main.ct.msx = jlgr->main.ct.event.tfinger.x;
		jlgr->main.ct.input.click = 1;
		jlgr->main.ct.msy = jlgr->main.ct.event.tfinger.y * jlgr->wm.ar;
		if(jlgr->sg.cs != JL_SCR_SS) {
			jlgr->main.ct.msy = jlgr->main.ct.msy * 2.;
			jlgr->main.ct.msy -= jlgr->wm.ar;
			if(jlgr->main.ct.msy < 0.) jlgr->main.ct.input.click = 0;
		}

	}else if( jlgr->main.ct.event.type==SDL_FINGERUP ) {
		jlgr->main.ct.input.click = 0;
	}else if( jlgr->main.ct.event.type==SDL_KEYDOWN || jlgr->main.ct.event.type==SDL_KEYUP) {
		if( jlgr->main.ct.event.key.keysym.scancode == SDL_SCANCODE_AC_BACK)
			jlgr->main.ct.input.back =
				(jlgr->main.ct.event.type==SDL_KEYDOWN); //Back Key
	}
#elif JL_PLAT == JL_PLAT_COMPUTER
	uint8_t isNowDown = jlgr->main.ct.event.type == SDL_MOUSEBUTTONDOWN;
	uint8_t isNowUp = jlgr->main.ct.event.type == SDL_MOUSEBUTTONUP;
	if( isNowDown || isNowUp) {
		if(jlgr->main.ct.event.button.button == SDL_BUTTON_LEFT)
			jlgr->main.ct.input.click = isNowDown;
		else if(jlgr->main.ct.event.button.button == SDL_BUTTON_RIGHT)
			jlgr->main.ct.input.click_right = isNowDown;
		else if(jlgr->main.ct.event.button.button == SDL_BUTTON_MIDDLE)
			jlgr->main.ct.input.click_middle = isNowDown;
	}else if(jlgr->main.ct.event.wheel.type == SDL_MOUSEWHEEL) {
		uint8_t flip = (jlgr->main.ct.event.wheel.direction ==
			SDL_MOUSEWHEEL_FLIPPED) ? -1 : 1;
		int32_t x = flip * jlgr->main.ct.event.wheel.x;
		int32_t y = flip * jlgr->main.ct.event.wheel.y;
		if(jlgr->main.ct.event.wheel.y > 0)
			jlgr->main.ct.input.scroll_up = (y > 0) ? y : -y;
		else if(jlgr->main.ct.event.wheel.y < 0)
			jlgr->main.ct.input.scroll_down = (y > 0) ? y : -y;
		if(jlgr->main.ct.event.wheel.x > 0)
			jlgr->main.ct.input.scroll_right = (x > 0) ? x : -x;
		else if(jlgr->main.ct.event.wheel.x < 0)
			jlgr->main.ct.input.scroll_left = (x > 0) ? x : -x;
	}

#endif
}

static void jl_ct_handle_resize__(jlgr_t* jlgr) {
	if(jlgr->main.ct.event.type==SDL_WINDOWEVENT) { //Resize
		switch(jlgr->main.ct.event.window.event) {
			case SDL_WINDOWEVENT_RESIZED: {
				JL_PRINT_DEBUG(jlgr->jl, "INPUT/RESIZE");
				jlgr_resz(jlgr,
					jlgr->main.ct.event.window.data1,
					jlgr->main.ct.event.window.data2);
				break;
			}
#if JL_PLAT == JL_PLAT_COMPUTER
			case SDL_WINDOWEVENT_CLOSE: {
				jl_print(jlgr->jl, "CLOSE");
				jlgr->main.ct.back = 1;
				break;
			}
#endif
			default: {
				break;
			}
		}
	}
}

static inline void jlgr_input_handle_events__(jlgr_t* jlgr) {
	if ( jlgr->main.ct.event.type == SDL_TEXTINPUT) {
//		JL_PRINT("%1s\n", &(jlgr->main.ct.event.text.text[0]));
		int i;
		for(i = 0; i < 32; i++)
			jlgr->main.ct.text_input[i] =
				jlgr->main.ct.event.text.text[i];
		jlgr->main.ct.read_cursor = 0;
	}else{
		jl_ct_handle_resize__(jlgr);
	}
	jlgr_input_handle_events_platform_dependant__(jlgr);
}

static void jl_ct_testquit__(jlgr_t* jlgr) {
	if(jlgr->main.ct.back == 1) jl_mode_exit(jlgr->jl); // Back Button/Key
}

static inline uint8_t jlgr_input_do__(jlgr_t *jlgr, jlgr_control_t events) {
	int8_t event;

	#if JL_PLAT == JL_PLAT_COMPUTER
		event = events.computer;
	#elif JL_PLAT == JL_PLAT_PHONE
		event = events.phone;
	#else // Game
		event = events.game;
	#endif
	if(events.computer == JLGR_INPUT_NONE) return -1;
	if(jlgr->input.states[events.computer] == -1) {
		#if JL_PLAT == JL_PLAT_COMPUTER
			event = events.computer_backup;
		#elif JL_PLAT == JL_PLAT_PHONE
			event = events.phone_backup;
		#else // Game
			event = events.game_backup;
		#endif
		if(events.computer == JLGR_INPUT_NONE) return -1;
	}
	return event;
}

/**
 * THREAD: Main thread.
 * For a certain input, do something.
 * @param jlgr: The library context.
 * @param event: The event id
 * @param fn: The function to run for the event.
 * @param data: Parameter to "fn": "jlgr_input_t->data"
 * @returns -1 on failure, 0 on success.
*/
int8_t jlgr_input_do(jlgr_t *jlgr, jlgr_control_t events, jlgr_input_fnct fn,
	void* data)
{
	int8_t event = jlgr_input_do__(jlgr, events);
	if(event == -1) return -1;
	void (*FunctionToRun_)(jlgr_t *jlgr, jlgr_input_fnct fn) =
		jlgr->main.ct.getEvents[event];

	if(jlgr->main.ct.getEvents[event] == NULL) {
		jl_print(jlgr->jl,"Null Pointer: jlgr->main.ct.getEvents.Event");
		jl_print(jlgr->jl,"event=%d", event);
		exit(-1);
	}
	// Get the input.
	jlgr->main.ct.current_event = event;
	// Run input function.
	jlgr->input.input.data = data;
	FunctionToRun_(jlgr, fn);
	return 0;
}

void jl_ct_getevents_(jlgr_t* jlgr) {
	jlgr->main.ct.keys = SDL_GetKeyboardState(NULL);
#if JL_PLAT == JL_PLAT_COMPUTER
	jlgr->main.ct.back = jl_ct_key_pressed__(jlgr, SDL_SCANCODE_ESCAPE);
#endif
	while(SDL_PollEvent(&jlgr->main.ct.event))
		jlgr_input_handle_events__(jlgr);
#if JL_PLAT == JL_PLAT_PHONE
	jl_ct_state__(&jlgr->main.ct.back, jlgr->main.ct.input.back);
#endif
}

void jl_ct_quickloop_(jlgr_t* jlgr) {
	jl_print_function(jlgr->jl, "INPUT_QUICKLOOP");
	jlgr->jl->has.quickloop = 1;
	if(jlgr->jl->has.input) {
		jl_ct_getevents_(jlgr);
		jl_ct_testquit__(jlgr);
	}else{
		while(SDL_PollEvent(&jlgr->main.ct.event))
			jl_ct_handle_resize__(jlgr);
	}
	jl_print_return(jlgr->jl, "INPUT_QUICKLOOP");
}

//Main Input Loop
void jl_ct_loop__(jlgr_t* jlgr) {
	if(jlgr->jl->has.quickloop) jlgr->jl->has.quickloop = 0;
	//Get the information on current events
	jl_ct_getevents_(jlgr);
	#if JL_PLAT == JL_PLAT_COMPUTER
		//Get Whether mouse is down or not and xy coordinates
		SDL_GetMouseState(&jlgr->main.ct.msxi,&jlgr->main.ct.msyi);

		int32_t mousex = jlgr->main.ct.msxi;
		int32_t mousey = jlgr->main.ct.msyi;
		//translate integer into float by clipping [0-1]
		jlgr->main.ct.msx = ((float)mousex) / jlgr_wm_getw(jlgr);
		jlgr->main.ct.msy = ((float)mousey) / jlgr_wm_geth(jlgr);
		if(jlgr->sg.cs != JL_SCR_SS) jlgr->main.ct.msy -= .5;
		jlgr->main.ct.msy *= jlgr->wm.ar;

		// Ignore mouse input on upper screen.
		if(jlgr->sg.cs != JL_SCR_SS && jlgr->main.ct.msy < 0.) {
			jlgr->main.ct.msy = 0.;
			// Show native cursor.
			if(!jlgr->main.ct.sc) {
				SDL_ShowCursor(SDL_ENABLE);
				jlgr->main.ct.sc = 1;
			}
		}else{
			// Show JL_Lib cursor
			if(jlgr->main.ct.sc) {
				SDL_ShowCursor(SDL_DISABLE);
				jlgr->main.ct.sc = 0;
			}
		}
		// F11 toggle fullscreen.
		if(jl_ct_key_pressed__(jlgr, SDL_SCANCODE_F11) == 1)
			jlgr_wm_togglefullscreen(jlgr);
	#endif
	jl_ct_testquit__(jlgr);
	int i;
	for(i = 0; i < JLGR_INPUT_NONE; i++) {
		jlgr->input.used[i] = 0;
	}
}

static inline void jlgr_input_fn_init__(jlgr_t* jlgr) {
	// COMPUTER1
	jlgr->main.ct.getEvents[JLGR_INPUT_RETURN] = jlgr_input_return__;
	jlgr->main.ct.getEvents[JLGR_INPUT_WASD] = jlgr_input_wasd__;
	jlgr->main.ct.getEvents[JLGR_INPUT_ARROW] = jlgr_input_arrow__;
	// PRESS
	jlgr->main.ct.getEvents[JLGR_INPUT_PRESS] = jlgr_input_press__;
//	jlgr->main.ct.getEvents[JLGR_INPUT_SWIPE] = jlgr_input_swipe__;
//	jlgr->main.ct.getEvents[JLGR_INPUT_PRESS_SDIR] = jlgr_input_sdir__;
	jlgr->main.ct.getEvents[JLGR_INPUT_PRESS_FDIR] = jlgr_input_dirfar__;
	jlgr->main.ct.getEvents[JLGR_INPUT_PRESS_NDIR] = jlgr_input_dirnear__;
	// MENU
	jlgr->main.ct.getEvents[JLGR_INPUT_MENU] = jl_ct_key_menu;
}

static inline void jl_input_enable_event_press__(jlgr_t* jlgr) {
	jlgr->input.states[JLGR_INPUT_PRESS] = 0;
	jlgr->input.states[JLGR_INPUT_SWIPE] = 0;
	jlgr->input.states[JLGR_INPUT_PRESS_SDIR] = 0;
	jlgr->input.states[JLGR_INPUT_PRESS_FDIR] = 0;
	jlgr->input.states[JLGR_INPUT_PRESS_NDIR] = 0;
}

static inline void jl_input_enable_event_computer__(jlgr_t* jlgr) {
	jlgr->input.states[JLGR_INPUT_ARROW] = 0;
	jlgr->input.states[JLGR_INPUT_WASD] = 0;
	jlgr->input.states[JLGR_INPUT_SPACE] = 0;
	jlgr->input.states[JLGR_INPUT_RETURN] = 0;
	jlgr->input.states[JLGR_INPUT_SHIFT] = 0;
	jlgr->input.states[JLGR_INPUT_TAB] = 0;
	jlgr->input.states[JLGR_INPUT_KEY] = 0;
	jlgr->input.states[JLGR_INPUT_SCROLL] = 0;
	jlgr->input.states[JLGR_INPUT_CLICK] = 0;
	jlgr->input.states[JLGR_INPUT_LOOK] = 0;
}

static inline void jl_input_enable_event_game1__(jlgr_t* jlgr) {
	jlgr->input.states[JLGR_INPUT_A] = 0;
	jlgr->input.states[JLGR_INPUT_B] = 0;
	jlgr->input.states[JLGR_INPUT_X] = 0;
	jlgr->input.states[JLGR_INPUT_Y] = 0;
	jlgr->input.states[JLGR_INPUT_ABXY] = 0;
	jlgr->input.states[JLGR_INPUT_DPAD] = 0;
}

static inline void jl_input_enable_event_game2__(jlgr_t* jlgr) {
	jlgr->input.states[JLGR_INPUT_L] = 0;
	jlgr->input.states[JLGR_INPUT_R] = 0;
	jlgr->input.states[JLGR_INPUT_LR] = 0;
	jlgr->input.states[JLGR_INPUT_START] = 0;
}

static inline void jl_input_enable_event_joy1__(jlgr_t* jlgr) {
	jlgr->input.states[JLGR_INPUT_LJOY] = 0;
}

static inline void jl_input_enable_event_joy2__(jlgr_t* jlgr) {
	jlgr->input.states[JLGR_INPUT_RJOY] = 0;
}

static inline void jl_input_enable_event_menu__(jlgr_t* jlgr) {
	jlgr->input.states[JLGR_INPUT_MENU] = 0;
}

static inline void jl_ct_var_init__(jlgr_t* jlgr) {
	int i;
	for(i = 0; i < 255; i++)
		jlgr->main.ct.keyDown[i] = 0;
	jlgr->main.ct.read_cursor = 0;
	for(i = 0; i < 32; i++)
		jlgr->main.ct.text_input[i] = 0;
	jlgr->main.ct.sc = 0;
	// Disable All Input
	for(i = 0; i < JLGR_INPUT_NONE; i++) {
		jlgr->input.states[i] = -1;
	}
	#if JL_PLAT == JL_PLAT_COMPUTER
		jl_input_enable_event_press__(jlgr);
		jl_input_enable_event_computer__(jlgr);
		jl_input_enable_event_menu__(jlgr);
//		jl_input_enable_event_joy1__(jlgr); // TODO: Only if plugged in
//		jl_input_enable_event_joy2__(jlgr); // TODO: Only if plugged in
	#elif JL_PLAT == JL_PLAT_PHONE
		jl_input_enable_event_press__(jlgr);
		jl_input_enable_event_game1__(jlgr);
		jl_input_enable_event_joy1__(jlgr);
		jl_input_enable_event_joy2__(jlgr);
		jl_input_enable_event_menu__(jlgr); // TODO: Only on Android
	#else // GAME
		jl_input_enable_event_press__(jlgr); // TODO: Only if not XBOX
		jl_input_enable_event_game1__(jlgr);
		jl_input_enable_event_game2__(jlgr);
		jl_input_enable_event_joy1__(jlgr);
		jl_input_enable_event_joy2__(jlgr); // TODO: Only if not 3DS
		jl_input_enable_event_menu__(jlgr); // TODO: Only if not XBOX
	#endif
}

void jl_ct_init__(jlgr_t* jlgr) {
	jlgr_input_fn_init__(jlgr);
	jl_ct_var_init__(jlgr);
	jlgr->jl->has.input = 1;
}

/**
 * Get any keys that are currently being typed.  Output in Aski.
 * If phone, pops up keyboard if not already up.  If nothing is being typed,
 * returns 0.
*/
uint8_t jl_ct_typing_get(jlgr_t *jlgr) {
	if(!SDL_IsTextInputActive()) SDL_StartTextInput();
	uint8_t rtn = jlgr->main.ct.text_input[jlgr->main.ct.read_cursor];
	if(jl_ct_key_pressed__(jlgr, SDL_SCANCODE_BACKSPACE) == 1) return '\b';
	if(jl_ct_key_pressed__(jlgr, SDL_SCANCODE_DELETE) == 1) return '\02';
	if(jl_ct_key_pressed__(jlgr, SDL_SCANCODE_RETURN) == 1) return '\n';
	if(!rtn) return 0;
	jlgr->main.ct.read_cursor++;
	return rtn;
}

/**
 * If phone, hides keyboard.
*/
void jl_ct_typing_disable(void) {
	SDL_StopTextInput();
}
