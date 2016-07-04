#ifndef JLGR
#define JLGR

#include "jl.h"

#if JL_GLTYPE == JL_GLTYPE_SDL_GL2  // SDL OpenGL 2
	#include "SDL_opengl.h"
	#include "SDL_opengl_glext.h"
#elif JL_GLTYPE == JL_GLTYPE_OPENGL2 // OpenGL 2
	#if JL_PLAT == JL_PLAT_COMPUTER
		#include "lib/glext.h"
	#else
		#error "JL_GLTYPE_OPENGL2 ain't supported by non-pc comps, man!"
	#endif
	#include "lib/glew/glew.h"
	#define JL_GLTYPE_HAS_GLEW
#elif JL_GLTYPE == JL_GLTYPE_SDL_ES2 // SDL OpenGLES 2
	#include "SDL_opengles2.h"
#elif JL_GLTYPE == JL_GLTYPE_OPENES2 // OpenGLES 2
	#include <GLES2/gl2.h>
	#include <GLES2/gl2ext.h>
#endif
#include "SDL_events.h"

#ifdef GL_ES_VERSION_2_0
	#define GLSL_HEAD "#version 100\nprecision highp float;\n"
#else
	#define GLSL_HEAD "#version 100\n"
#endif

typedef enum{
	JLGR_INPUT_PRESS_ISNT, // User is not currently using the control
	JLGR_INPUT_PRESS_JUST, // User just started using the control
	JLGR_INPUT_PRESS_HELD, // User is using the control
	JLGR_INPUT_PRESS_STOP, // User just released the control.
}JLGR_INPUT_PRESS_T;

typedef enum{
	JLGR_INPUT_DIR_NO=0, // Center
	JLGR_INPUT_DIR_UP=1, // Up
	JLGR_INPUT_DIR_DN=2, // Down
	JLGR_INPUT_DIR_RT=3, // Right
	JLGR_INPUT_DIR_LT=4, // Left
	JLGR_INPUT_DIR_UL=5, // Up Left
	JLGR_INPUT_DIR_UR=6, // Up Right
	JLGR_INPUT_DIR_DL=7, // Down Left
	JLGR_INPUT_DIR_DR=8, // Down Right
}JLGR_INPUT_DIRECTION_T;

//
// Computer ( Linux / Windows / Mac ):
//	JLGR_INPUT_DEVICE_PRESS
//	JLGR_INPUT_DEVICE_COMPUTER
//	JLGR_INPUT_DEVICE_MENU
//	* JLGR_INPUT_DEVICE_GAME1 ( if joystick found )
//	* JLGR_INPUT_DEVICE_GAME2 ( if joystick found )
// Phone ( Android / IPad / IPhone ):
//	JLGR_INPUT_DEVICE_PRESS
//	JLGR_INPUT_DEVICE_GAME1
//	JLGR_INPUT_DEVICE_JOY1
//	JLGR_INPUT_DEVICE_JOY2
//	* JLGR_INPUT_DEVICE_MENU ( Android only )
// Game ( 3DS / Wii U / XBox ):
//	* JLGR_INPUT_DEVICE_PRESS ( Wii U / 3DS only )
//	JLGR_INPUT_DEVICE_GAME1
//	JLGR_INPUT_DEVICE_GAME2
//	JLGR_INPUT_DEVICE_JOY1
//	* JLGR_INPUT_DEVICE_JOY2 ( Wii U only )
//	* JLGR_INPUT_DEVICE_MENU ( Wii U / 3DS only )
typedef enum{
	JLGR_INPUT_DEVICE_PRESS,	// Mouse or Touch Screen Available
	JLGR_INPUT_DEVICE_GAME1,	// ABXY Buttons Available
	JLGR_INPUT_DEVICE_GAME2,	// L, R, and Start Buttons Available
	JLGR_INPUT_DEVICE_JOY1,		// Left (1st) Joystick Available
	JLGR_INPUT_DEVICE_JOY2,		// Right (2nd) Joystick Available
	JLGR_INPUT_DEVICE_COMPUTER, 	// Physical Keyboard / Mouse Available.
	JLGR_INPUT_DEVICE_MENU,		// Menu Key or Select Button
	JLGR_INPUT_DEVICE_MAX,		// # of device types
}JLGR_INPUT_DEVICE_T;

typedef enum{
	// PRESS
	JLGR_INPUT_PRESS,	// Touch Screen Press / Left Click
	JLGR_INPUT_SWIPE,	// Swipe
	JLGR_INPUT_PRESS_SDIR,	// Simple Directional Press
	JLGR_INPUT_PRESS_FDIR,	// Far Directional Press
	JLGR_INPUT_PRESS_NDIR,	// Near Directional Press
	// GAME1
	JLGR_INPUT_A,		// A button: Right
	JLGR_INPUT_B,		// B button: Down
	JLGR_INPUT_X,		// X button: Up
	JLGR_INPUT_Y,		// Y button: Left
	JLGR_INPUT_ABXY,	// ABXY Buttons
	JLGR_INPUT_DPAD,	// D-Pad
	// GAME2
	JLGR_INPUT_L,		// L button: Up/Left
	JLGR_INPUT_R,		// R button: Down/Right
	JLGR_INPUT_LR,		// L & R Buttons
	JLGR_INPUT_START,	// Start button
	// JOY1
	JLGR_INPUT_LJOY,	// Left Joystick ( Circle Pad )
	// JOY2
	JLGR_INPUT_RJOY,	// Right Josytick
	// COMPUTER
	JLGR_INPUT_ARROW,	// Arrow Keys
	JLGR_INPUT_WASD,	// WASD
	JLGR_INPUT_SPACE,	// Space Key
	JLGR_INPUT_RETURN,	// Return Key
	JLGR_INPUT_SHIFT,	// Shift Key
	JLGR_INPUT_TAB,		// Tab Key
	JLGR_INPUT_KEY,		// Any Other Key
	JLGR_INPUT_SCROLL,	// Scroll ( SHIFT to scroll right/left)
	JLGR_INPUT_CLICK,	// Click Right(Ctrl-Click)/Left/Middle(Shift-Click)
	JLGR_INPUT_LOOK,	// Mouse Movement
	// MENU
	JLGR_INPUT_MENU,	// Menu Key or Select Button
	//
	JLGR_INPUT_NONE,
}JLGR_INPUT_T;

typedef struct{
	uint8_t computer;
	uint8_t computer_backup;
	uint8_t phone;
	uint8_t phone_backup;
	uint8_t game;
	uint8_t game_backup;
}jlgr_control_t;

#if JL_PLAT == JL_PLAT_COMPUTER
	#define JL_CT_ALLP(nintendo, computer, android) computer
#elif JL_PLAT == JL_PLAT_PHONE
	#define JL_CT_ALLP(nintendo, computer, android) android
#else //3DS/WiiU
	#define JL_CT_ALLP(nintendo, computer, android) nintendo
#endif

// Press.
#define JL_INPUT_PRESS (jlgr_control_t) { \
	JLGR_INPUT_PRESS, JLGR_INPUT_NONE,	/*Computer*/ \
	JLGR_INPUT_PRESS, JLGR_INPUT_NONE,	/*Phone*/ \
	JLGR_INPUT_PRESS, JLGR_INPUT_NONE }	/*Game*/
// Joystick ( Input Design A ).
#define JL_INPUT_JOYA1 (jlgr_control_t) { \
	JLGR_INPUT_LJOY, JLGR_INPUT_WASD,	/*Computer*/ \
	JLGR_INPUT_LJOY, JLGR_INPUT_NONE,	/*Phone*/ \
	JLGR_INPUT_LJOY, JLGR_INPUT_NONE}	/*Game*/
#define JL_INPUT_JOYA2 (jlgr_control_t) { \
	JLGR_INPUT_RJOY, JLGR_INPUT_ARROW,	/*Computer*/ \
	JLGR_INPUT_RJOY, JLGR_INPUT_NONE,	/*Phone*/ \
	JLGR_INPUT_RJOY, JLGR_INPUT_DPAD}	/*Game*/
// Joystick ( Input Design B ).
#define JL_INPUT_JOYB1 (jlgr_control_t) { \
	JLGR_INPUT_LJOY, JLGR_INPUT_WASD,	/*Computer*/ \
	JLGR_INPUT_PRESS_FDIR, JLGR_INPUT_NONE,	/*Phone*/ \
	JLGR_INPUT_PRESS_FDIR, JLGR_INPUT_NONE}	/*Game*/
#define JL_INPUT_JOYB2 (jlgr_control_t) { \
	JLGR_INPUT_RJOY, JLGR_INPUT_ARROW,	/*Computer*/ \
	JLGR_INPUT_PRESS_NDIR, JLGR_INPUT_NONE,	/*Phone*/ \
	JLGR_INPUT_PRESS_NDIR, JLGR_INPUT_NONE}	/*Game*/
// Joystick ( Input Design C ).
#define JL_INPUT_JOYC (jlgr_control_t) { \
	JLGR_INPUT_ARROW, JLGR_INPUT_NONE,	/*Computer*/ \
	JLGR_INPUT_RJOY, JLGR_INPUT_NONE,	/*Phone*/ \
	JLGR_INPUT_LJOY, JLGR_INPUT_NONE}	/*Game*/
// Main Button
#define JL_INPUT_SELECT (jlgr_control_t) { \
	JLGR_INPUT_RETURN, JLGR_INPUT_NONE,	/*Computer*/ \
	JLGR_INPUT_A, JLGR_INPUT_NONE,		/*Phone*/ \
	JLGR_INPUT_A, JLGR_INPUT_NONE}		/*Game*/
#define JL_INPUT_SELECT2 (jlgr_control_t) { \
	JLGR_INPUT_SHIFT, JLGR_INPUT_NONE,	/*Computer*/ \
	JLGR_INPUT_B, JLGR_INPUT_NONE,		/*Phone*/ \
	JLGR_INPUT_B, JLGR_INPUT_NONE}		/*Game*/
#define JL_INPUT_SELECT3 (jlgr_control_t) { \
	JLGR_INPUT_SPACE, JLGR_INPUT_NONE,	/*Computer*/ \
	JLGR_INPUT_X, JLGR_INPUT_NONE,		/*Phone*/ \
	JLGR_INPUT_X, JLGR_INPUT_NONE}		/*Game*/
#define JL_INPUT_SELECT4 (jlgr_control_t) { \
	JLGR_INPUT_TAB, JLGR_INPUT_NONE,	/*Computer*/ \
	JLGR_INPUT_Y, JLGR_INPUT_NONE,		/*Phone*/ \
	JLGR_INPUT_Y, JLGR_INPUT_NONE}		/*Game*/
// Menu Button
#define JL_INPUT_MENU (jlgr_control_t) { \
	JLGR_INPUT_MENU, JLGR_INPUT_NONE,	/*Computer*/ \
	JLGR_INPUT_MENU, JLGR_INPUT_NONE,	/*Phone*/ \
	JLGR_INPUT_START, JLGR_INPUT_NONE}	/*Game*/

typedef enum {
	JL_SCR_UP,
	JL_SCR_DN,
	JL_SCR_SS,
}jlgr_which_screen_t;

// Types:

// Coordinate Structures
typedef struct{
	float x, y, w, h;
}jl_rect_t;

// Collision Box / Line / Etc.
typedef struct{
	jl_vec3_t pos; // Position ( X/Y/Z )
	jl_vec3_t ofs; // Position Offset ( W/H/D )
}jl_area_t;

// Graphical stuff

typedef struct {
	uint32_t gl_texture;
	uint16_t w, h;
	void* pixels; // BGRA
}jl_tex_t;

// Pre-renderer
typedef struct {
	// What to render
	uint32_t tx;	// ID to texture.
	uint32_t fb;	// ID to Frame Buffer
	uint16_t w, h;	// Width and hieght of texture
	// Render Area
	uint32_t gl;	// GL Vertex Buffer Object [ 0 = Not Enabled ]
	float ar;	// Aspect Ratio: h:w
	float cv[4*3];	// Converted Vertices
	jl_area_t cb;	// 2D/3D collision box.
	jl_vec3_t scl;	// Scaling vector.
}jl_pr_t;

typedef struct{
	struct {
		int32_t position;
		int32_t texpos_color;
	}attributes;

	struct {
		int32_t texture;
		int32_t newcolor_malpha;
		int32_t translate;
		int32_t transform;
	}uniforms;

	uint32_t program;
}jlgr_glsl_t;

//Vertex Object
typedef struct{
	// Basic:
	uint8_t rs;	// Rendering Style 0=GL_TRIANGLE_FAN 1=GL_TRIANGLES
	uint32_t gl;	// GL Vertex Buffer Object [ 0 = Not Enabled ]
	uint32_t vc;	// # of Vertices
	float* cv;	// Converted Vertices
	uint32_t bt;	// Buffer for Texture coordinates or Color Vertices.
	// Coloring:
	float* cc;	// Colors
	// Texturing:
	uint32_t tx;	// ID to texture. [ 0 = Colors Instead ]
}jl_vo_t;

/**
 * Font type.
**/
typedef struct {
	/** The texture ID of the font. */
	int32_t tex;
	/** Whether to allow multiple colors ( gradient ). */
	uint8_t multicolor;
	/** The color value(s). */
	float* colors;
	/** The size to draw the text ( 0. - 1. ). */
	float size;
}jl_font_t;

typedef struct{
	SDL_mutex* mutex;	// The mutex for writing/reading ctx_draw.
	float rh, rw;		// Real Height & Width
	void* ctx_main;		// The sprite's context.
	void* ctx_draw;		// Information required for drawing.
	uint32_t ctx_draw_size;	// Size of "ctx_draw"
	void* loop;		// (jlgr_sprite_loop_fnt) Loop function
	void* kill;		// (jlgr_sprite_loop_fnt) Kill function
	void* draw;		// (jlgr_sprite_draw_fnt) Draw function
	uint8_t update;		// Whether sprite should redraw or not.
	jl_pr_t pr;		// Pre-renderer / collision box.
}jl_sprite_t;

typedef void (*jlgr_sprite_draw_fnt)(jl_t* jl, uint8_t resize, void* ctx_draw);
typedef void (*jlgr_sprite_loop_fnt)(jl_t* jl, jl_sprite_t* spr);

typedef struct{
	char *opt;
	jl_fnct run;
}jl_popup_button_t;

typedef struct{
	void* single;
	void* upper;
	void* lower;
	void* resize;
}jlgr_redraw_t;

typedef struct{
	uint8_t id;		// Packet ID
	uint16_t x, y;		// X(w), Y(h)
	jl_fnct fn;		// Function
}jlgr_thread_packet_t;

typedef struct{
	float x; // X Location
	float y; // Y Location
	float r; // Rotational Value in "pi radians" 2=full circle
	float p; // Pressure 0-1
	int8_t h; // How long held down.
	uint8_t k; // Which key [ a-z, 0-9 , left/right click ]
	void* data; // Parameter
}jlgr_input_t;

typedef struct{
	jl_t* jl;

	// For Programer's Use
	float fontcolor[4];
	jl_font_t font;
	jl_sprite_t mouse; // Sprite to represent mouse pointer

	uint8_t thread; // Graphical Thread ID.
	SDL_mutex* mutex; // Mutex to lock wshare structure.
	jl_comm_t* comm2draw; // thread communication variable.
	jl_wait_t wait;
	struct {
		SDL_mutex *usr_ctx;
	}mutexs;

	struct{
		jlgr_input_t input;
		int8_t states[JLGR_INPUT_NONE];
		uint8_t used[JLGR_INPUT_NONE];
	}input;

	struct {
		//Input Information
		struct {
			void* getEvents[JLGR_INPUT_NONE];

			float msx, msy;
			int msxi, msyi;

			SDL_Event event;
		
			const Uint8 *keys;

			struct {
				#if JL_PLAT == JL_PLAT_PHONE
					uint8_t back;
				#elif JL_PLAT == JL_PLAT_COMPUTER
					uint8_t click_right; // Or Ctrl-Click
					uint8_t click_middle; // Or Shift-Click
				#endif
				//Multi-Platform
				uint8_t click; // Or Click Left
				uint8_t scroll_right;
				uint8_t scroll_left;
				uint8_t scroll_up;
				uint8_t scroll_down;
			}input;

			uint8_t back; //Back Key, Escape Key, Start Button
			int8_t keyDown[255];
			uint32_t sd; //NYI: stylus delete
		
			uint8_t sc;
			uint8_t text_input[32];
			uint8_t read_cursor;

			uint8_t current_event;
		}ct;
	} main;

	struct {
		uint8_t rtn;
		jl_fnct fn;
		jlgr_redraw_t redraw;
	} draw;

	// Window Info
	struct {
		uint32_t taskbar[5];
		uint32_t init_image_location;

		// If Matching FPS
		uint8_t on_time;
		uint8_t changed;
		
		// Each screen is a sprite.
		struct {
			jl_sprite_t up;
			jl_sprite_t dn;
		}bg;

		void* loop; // ( jlgr_fnct ) For upper or lower screen.
		uint8_t cs; // The current screen "jlgr_which_screen_t"
	}sg;

	struct{
		jlgr_glsl_t alpha;
		jlgr_glsl_t hue;
		float colors[4];
	}effects;
	
	//Opengl Data
	struct {
		struct {
			jlgr_glsl_t texture;
			jlgr_glsl_t color;
		}prg;

		jl_vo_t temp_vo;
		// Default texture coordinates.
		uint32_t default_tc;
		
		jl_pr_t* cp; // Renderer currently being drawn on.
	}gl;

	struct {
		jl_sprite_t menubar;
	}menubar;

	//Graphics
	struct {
		struct {
			double timeTilVanish;
			char message[256];
		} notification;
		struct {
			char* window_name;
			char* message;
			jl_popup_button_t* btns;
		}popup;
		struct {
			jl_vo_t whole_screen;
		}vos;
		struct {
			char message[256];
			uint16_t t;
			uint8_t c;
		}msge;
		data_t* textbox_string;
	}gr;

	// Window Management
	struct {
	#if JL_PLAT == JL_PLAT_COMPUTER
		uint8_t fullscreen;
	#endif

		char windowTitle[2][16];
		SDL_Window* window;
		SDL_GLContext* glcontext;
		// The full width and height of the window.
		int32_t w, h;
		// Aspect Ratio of the window
		float ar;
	}wm;

	// File Manager
	struct {
		struct cl_list *filelist; //List of all files in working dir.
		int8_t cursor;
		uint8_t cpage;
		char *dirname;
		char *selecteditem;
		uint8_t returnit;
		uint8_t drawupto;
		uint8_t inloop;
		jl_sprite_t btns[2];
		void *newfiledata;
		uint64_t newfilesize;
		uint8_t prompt;
		data_t* promptstring;
	}fl;

	struct {
		uint32_t font; // JL_Lib font
		uint32_t logo; // JL_Lib Loading Logo
		uint32_t game; // Game Graphics
		uint32_t icon; // Icons
	} textures;

	double timer;
	double psec;
}jlgr_t;

typedef void(*jlgr_fnct)(jlgr_t* jlgr);
typedef void(*jlgr_input_fnct)(jlgr_t* jlgr, jlgr_input_t input);

// JLGR.c:
jlgr_t* jlgr_init(jl_t* jl, uint8_t fullscreen, jl_fnct fn_);
void jlgr_loop_set(jlgr_t* jlgr, jl_fnct onescreen, jl_fnct upscreen,
	jl_fnct downscreen, jl_fnct resize);

// JLGRsprite.c
void jlgr_sprite_dont(jl_t* jl, jl_sprite_t* sprite);
void jlgr_sprite_redraw(jlgr_t* jlgr, jl_sprite_t *spr, void* ctx);
void jlgr_sprite_resize(jlgr_t* jlgr, jl_sprite_t *spr, jl_rect_t* rc);
void jlgr_sprite_loop(jlgr_t* jlgr, jl_sprite_t *spr);
void jlgr_sprite_draw(jlgr_t* jlgr, jl_sprite_t *spr);
void jlgr_sprite_init(jlgr_t* jlgr, jl_sprite_t* sprite, jl_rect_t rc,
	jlgr_sprite_loop_fnt loopfn, jlgr_sprite_draw_fnt drawfn,
	void* main_ctx, uint32_t main_ctx_size,
	void* draw_ctx, uint32_t draw_ctx_size);
void jlgr_sprite_free(jlgr_t* jlgr, jl_sprite_t* sprite);
uint8_t jlgr_sprite_collide(jlgr_t* jlgr, jl_sprite_t *spr1, jl_sprite_t *spr2);
void* jlgr_sprite_getcontext(jl_sprite_t *sprite);
void* jlgr_sprite_getdrawctx(jl_sprite_t *sprite);

// JLGRmenu.c
void jlgr_menu_toggle(jlgr_t* jlgr);
void jlgr_menu_draw_icon(jlgr_t* jlgr, uint32_t tex, uint8_t c);
void jlgr_menu_addicon(jlgr_t* jlgr, jlgr_input_fnct inputfn, jlgr_fnct rdr);
void jlgr_menu_addicon_flip(jlgr_t* jlgr);
void jlgr_menu_addicon_slow(jlgr_t* jlgr);
void jlgr_menu_addicon_name(jlgr_t* jlgr);

// JLGRgraphics.c:
void jlgr_dont(jlgr_t* jlgr);
void jlgr_fill_image_set(jlgr_t* jlgr, uint32_t tex, uint8_t w, uint8_t h, 
	int16_t c);
void jlgr_fill_image_draw(jlgr_t* jlgr);
void jlgr_draw_bg(jlgr_t* jlgr, uint32_t tex, uint8_t w, uint8_t h, int16_t c);

// JLGRtext.c:
void jlgr_text_draw(jlgr_t* jlgr, const char* str, jl_vec3_t loc, jl_font_t f);
void jlgr_draw_int(jlgr_t* jlgr, int64_t num, jl_vec3_t loc, jl_font_t f);
void jlgr_draw_dec(jlgr_t* jlgr, double num, uint8_t dec, jl_vec3_t loc,
	jl_font_t f);
void jlgr_text_draw_area(jlgr_t* jlgr, jl_sprite_t * spr, const char* txt);
void jlgr_draw_text_sprite(jlgr_t* jlgr, jl_sprite_t* spr, const char* txt);
void jlgr_draw_ctxt(jlgr_t* jlgr, char *str, float yy, float* color);
void jlgr_draw_loadscreen(jlgr_t* jlgr, jl_fnct draw_routine);
void jlgr_draw_msge(jlgr_t* jlgr, uint32_t tex, uint8_t c, char* format, ...);
void jlgr_term_msge(jlgr_t* jlgr, char* message);
void jlgr_slidebtn_rsz(jlgr_t* jlgr, jl_sprite_t * spr, const char* txt);
void jlgr_slidebtn_loop(jlgr_t* jlgr, jl_sprite_t * spr, float defaultx,
	float slidex, jlgr_input_fnct prun);
void jlgr_glow_button_draw(jlgr_t* jlgr, jl_sprite_t * spr,
	char *txt, jlgr_input_fnct prun);
uint8_t jlgr_draw_textbox(jlgr_t* jlgr, float x, float y, float w,
	float h, data_t* string);
void jlgr_gui_slider(jlgr_t* jlgr, jl_sprite_t* sprite, jl_rect_t rectangle,
	uint8_t isdouble, float* x1, float* x2);
void jlgr_notify(jlgr_t* jlgr, const char* notification);

// JLGRvo.c
void jlgr_vo_init(jlgr_t* jlgr, jl_vo_t* vo);
void jlgr_vo_set_vg(jlgr_t* jlgr, jl_vo_t *vo, uint16_t tricount,
	float* triangles, float* colors, uint8_t multicolor);
void jlgr_vo_set_rect(jlgr_t* jlgr, jl_vo_t *vo, jl_rect_t rc, float* colors,
	uint8_t multicolor);
void jlgr_vo_set_image(jlgr_t* jlgr, jl_vo_t *vo, jl_rect_t rc, uint32_t tex);
void jlgr_vo_txmap(jlgr_t* jlgr,jl_vo_t* vo,uint8_t w,uint8_t h,int16_t map);
void jlgr_vo_color_gradient(jlgr_t* jlgr, jl_vo_t* vo, float* rgba);
void jlgr_vo_color_solid(jlgr_t* jlgr, jl_vo_t* vo, float* rgba);
void jlgr_vo_draw2(jlgr_t* jlgr, jl_vo_t* vo, jlgr_glsl_t* sh);
void jlgr_vo_draw(jlgr_t* jlgr, jl_vo_t* vo, jl_vec3_t* vec);
void jlgr_vo_free(jlgr_t* jlgr, jl_vo_t *vo);

// JLGRpr.c
void jlgr_pr_off(jlgr_t* jlgr);
void jlgr_pr_resize(jlgr_t* jlgr, jl_pr_t* pr, float w, float h, uint16_t w_px);
void jlgr_pr_init(jlgr_t* jlgr, jl_pr_t* pr, float w, float h, uint16_t w_px);
void jlgr_pr_draw(jlgr_t* jlgr, jl_pr_t* pr, jl_vec3_t* vec, jl_vec3_t* scl);
void jlgr_pr(jlgr_t* jlgr, jl_pr_t * pr, jl_fnct par__redraw);

// OpenGL
void jl_gl_pbo_new(jlgr_t* jlgr, jl_tex_t* texture, uint8_t* pixels,
	uint16_t w, uint16_t h, uint8_t bpp);
void jl_gl_pbo_set(jlgr_t* jlgr, jl_tex_t* texture, uint8_t* pixels,
	uint16_t w, uint16_t h, uint8_t bpp);
uint32_t jl_gl_maketexture(jlgr_t* jlgr, void* pixels,
	uint32_t width, uint32_t height, uint8_t bytepp);
float jl_gl_ar(jlgr_t* jlgr);
void jl_gl_clear(jlgr_t* jlgr, float r, float g, float b, float a);
void jlgr_gl_shader_init(jlgr_t* jlgr, jlgr_glsl_t* glsl, const char* vert,
	const char* frag, const char* effectName);

// JLGRopengl.c
void jlgr_opengl_draw1(jlgr_t* jlgr, jlgr_glsl_t* sh);

// JLGReffects.c
void jlgr_effects_uniform1(jlgr_t* jlgr, jlgr_glsl_t* sh, float x);
void jlgr_effects_uniform3(jlgr_t* jlgr, jlgr_glsl_t* sh, float x, float y,
	float z);
void jlgr_effects_uniform4(jlgr_t* jlgr, jlgr_glsl_t* sh, float x, float y,
	float z, float w);
void jlgr_effects_vo_hue(jlgr_t* jlgr, jl_vo_t* vo, jl_vec3_t offs, float c[]);
void jlgr_effects_hue(jlgr_t* jlgr, float c[]);

// video
void jl_vi_make_jpeg(jl_t* jl, data_t* rtn, uint8_t quality, uint8_t* pxdata,
	uint16_t w, uint16_t h);
uint8_t* jlgr_load_image(jl_t* jl, data_t* data, uint16_t* w, uint16_t* h);

// SG
uint32_t jl_sg_add_image(jlgr_t* jlgr, data_t* zipdata, const char* filename);

// JLGRinput.c
int8_t jlgr_input_do(jlgr_t *jlgr, jlgr_control_t events, jlgr_input_fnct fn,
	void* data);
void jlgr_input_dont(jlgr_t* jlgr, jlgr_input_t input);
void jl_ct_quickloop_(jlgr_t* jlgr);
uint8_t jl_ct_typing_get(jlgr_t* pusr);
void jl_ct_typing_disable(void);

// JLGRfiles.c
uint8_t jlgr_openfile_init(jlgr_t* jlgr, const char* program_name,
	void *newfiledata, uint64_t newfilesize);
void jlgr_openfile_loop(jlgr_t* jlgr);
const char* jlgr_openfile_kill(jlgr_t* jlgr);

// JLGRwm.c
void jlgr_wm_setfullscreen(jlgr_t* jlgr, uint8_t is);
void jlgr_wm_togglefullscreen(jlgr_t* jlgr);
uint16_t jlgr_wm_getw(jlgr_t* jlgr);
uint16_t jlgr_wm_geth(jlgr_t* jlgr);
void jlgr_wm_setwindowname(jlgr_t* jlgr, const char* window_name);

#endif
