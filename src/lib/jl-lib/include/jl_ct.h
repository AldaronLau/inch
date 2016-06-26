/*
 * (c) Jeron A. Lau
 * library for making input compatible between different
 * devices.
*/

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
//
