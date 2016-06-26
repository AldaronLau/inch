// Version Definitions
#define JL_VERSION "6.0.0 beta"
/*
 * Version System:
 * 	major version "." minor version "." debug version "."
 *
 *	A new major version is made every time your code will break.
 *	A new minor version is made every time new features are added.
 *	A new debug version is made for every debug.
 *	A new -x version is made for every commit with alpha/beta extension.
 */

// Uncommented for debugging ( GL/SDL error check && verbose printing. )
#define JL_DEBUG_LIB

#ifdef JL_DEBUG_LIB
	#define DEBUG
#endif

//Platform Declarations
#define JL_PLAT_COMPUTER 0 //PC/MAC
#define JL_PLAT_PHONE 1 //ANDROID/IPHONE
#define JL_PLAT_GAME 2 // 3DS
#if defined(__IPHONEOS__) || defined(__ANDROID__)
        #define JL_PLAT JL_PLAT_PHONE
#else
        #define JL_PLAT JL_PLAT_COMPUTER
#endif

//Determine Which OpenGL to use.

#define JL_GLTYPE_NO_SPRT 0 // No Support for OpenGL
// GLES version 2
#define JL_GLTYPE_SDL_GL2 1 // Include OpenGL with SDL
#define JL_GLTYPE_OPENGL2 2 // Include OpenGL with glut.
#define JL_GLTYPE_SDL_ES2 3 // Include OpenGLES 2 with SDL
#define JL_GLTYPE_OPENES2 4 // Include OpenGLES 2 standardly.
// Newer versions...

#define JL_GLTYPE JL_GLTYPE_NO_SPRT

// Platform Capabilities.
#if JL_PLAT == JL_PLAT_COMPUTER
	// All Linux Platforms
	#undef JL_GLTYPE
	#define JL_GLTYPE JL_GLTYPE_SDL_ES2
	// Windows
	// #define JL_GLTYPE JL_GLTYPE_SDL_GL2
#elif JL_PLAT == JL_PLAT_PHONE
	#undef JL_GLTYPE
	#define JL_GLTYPE JL_GLTYPE_SDL_ES2
#else
	#error "NO OpenGL support for this platform!"
#endif
