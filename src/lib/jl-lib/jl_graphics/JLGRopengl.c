#include "JLGRinternal.h"

// Shader Code

const char *JL_SHADER_CLR_FRAG = 
	GLSL_HEAD
	"varying vec4 vcolor;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = vec4(vcolor.rgba);\n"
	"}";

const char *JL_SHADER_CLR_VERT = 
	GLSL_HEAD
	"uniform vec3 translate;\n"
	"uniform vec4 transform;\n"
	"\n"
	"attribute vec3 position;\n"
	"attribute vec4 acolor;\n"
	"\n"
	"varying vec4 vcolor;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = transform * vec4(position + translate, 1.0);\n"
	"	vcolor = acolor;\n"
	"}";

const char *JL_SHADER_TEX_FRAG = 
	GLSL_HEAD
	"uniform sampler2D texture;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = texture2D(texture, texcoord);\n"
	"}";

const char *JL_SHADER_TEX_VERT = 
	GLSL_HEAD
	"uniform vec3 translate;\n"
	"uniform vec4 transform;\n"
	"\n"
	"attribute vec3 position;\n"
	"attribute vec2 texpos;\n"
	"\n"
	"varying vec2 texcoord;\n"
	"\n"
	"void main() {\n"
	"	texcoord = texpos;\n"
	"	gl_Position = transform * vec4(position + translate, 1.0);\n"
	"}";

// Full texture
const float DEFAULT_TC[] = {
	0., 1.,
	0., 0.,
	1., 0.,
	1., 1.
};

// Prototypes:

//static void jl_gl_depthbuffer_set__(jlgr_t* jlgr, uint16_t w, uint16_t h);
//static void _jl_gl_depthbuffer_bind(jlgr_t* jlgr, uint32_t db);
//static void _jl_gl_depthbuffer_make(jlgr_t* jlgr, uint32_t *db);
//static void jl_gl_depthbuffer_off__(jlgr_t* jlgr);
static void jl_gl_framebuffer_addtx__(jlgr_t* jlgr, uint32_t tx);
static void jl_gl_framebuffer_adddb__(jlgr_t* jlgr, uint32_t db);
static void jl_gl_framebuffer_status__(jlgr_t* jlgr);
static void jl_gl_framebuffer_use__(jlgr_t* jlgr, jl_pr_t* pr);

// Definitions:
#ifdef JL_DEBUG_LIB
	#define JL_GL_ERROR(jlgr, x, fname) jl_gl_get_error___(jlgr, x, fname)
	#define JL_EGL_ERROR(jlgr, x, fname) jl_gl_egl_geterror__(jlgr, x, fname)
#else
	#define JL_GL_ERROR(jlgr, x, fname) ;
	#define JL_EGL_ERROR(jlgr, x, fname) ;
#endif

// Functions:

#ifdef JL_DEBUG_LIB
	static void jl_gl_get_error___(jlgr_t* jlgr, int width, str_t fname) {
		uint8_t thread = jl_thread_current(jlgr->jl);
		if(thread != 1) {
			jl_print(jlgr->jl, "\"%s\" is on the Wrong Thread: %d",
				fname, thread);
			jl_print(jlgr->jl, "Must be on thread 1!");
			jl_print_stacktrace(jlgr->jl);
			exit(-1);
		}

		GLenum err= glGetError();
		if(err == GL_NO_ERROR) return;
		char *fstrerr;
		if(err == GL_INVALID_ENUM) {
			fstrerr = "opengl: invalid enum";
		}else if(err == GL_INVALID_VALUE) {
			fstrerr = "opengl: invalid value!!\n";
		}else if(err == GL_INVALID_OPERATION) {
			fstrerr = "opengl: invalid operation!!\n";
		}else if(err == GL_OUT_OF_MEMORY) {
			fstrerr = "opengl: out of memory ): "
				"!! (Texture too big?)\n";
			GLint a;
			glGetIntegerv(GL_MAX_TEXTURE_SIZE, &a);
			JL_PRINT("Max texture size: %d/%d\n", width, a);
		}else{
			fstrerr = "opengl: unknown error!\n";
		}
		jl_print(jlgr->jl, "error: %s:%s (%d)",fname,fstrerr,width);
		exit(-1);
	}
#endif

static void jl_gl_buffer_use__(jlgr_t* jlgr, GLuint *buffer) {
	// Make buffer if not initialized.
	if(*buffer == 0) {
		glGenBuffers(1, buffer);
#ifdef JL_DEBUG_LIB
		JL_GL_ERROR(jlgr, 0,"buffer gen");
		if(*buffer == 0) {
			jl_print(jlgr->jl,
				"buffer is made wrongly on thread #%d!",
				jl_thread_current(jlgr->jl));
			exit(-1);
		}
#endif
	}
	// Bind the buffer
	glBindBuffer(GL_ARRAY_BUFFER, *buffer);
	JL_GL_ERROR(jlgr, *buffer, "bind buffer");
}

// Set the Data for VBO "buffer" to "buffer_data" with "buffer_size"
static void jl_gl_buffer_set__(jlgr_t* jlgr, GLuint *buffer,
	const void *buffer_data, uint16_t buffer_size)
{
	//Bind Buffer "buffer"
	jl_gl_buffer_use__(jlgr, buffer);
	//Set the data
	glBufferData(GL_ARRAY_BUFFER, buffer_size * sizeof(float), buffer_data,
		GL_DYNAMIC_DRAW);
	JL_GL_ERROR(jlgr, buffer_size, "buffer data");
}

static void jl_gl_buffer_old__(jlgr_t* jlgr, uint32_t *buffer) {
	glDeleteBuffers(1, buffer);
	JL_GL_ERROR(jlgr, 0,"buffer free");
	*buffer = 0;
}

GLuint jl_gl_load_shader(jlgr_t* jlgr, GLenum shaderType, const char* pSource) {
	GLuint shader = glCreateShader(shaderType);
	JL_GL_ERROR(jlgr, 0,"couldn't create shader");
	if (shader) {
		GLint compiled = 0;

		glShaderSource(shader, 1, &pSource, NULL);
		JL_GL_ERROR(jlgr, 0,"glShaderSource");
		glCompileShader(shader);
		JL_GL_ERROR(jlgr, 0,"glCompileShader");
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		JL_GL_ERROR(jlgr, 0,"glGetShaderiv");
		if (!compiled) {
			GLint infoLen = 0;
			char* buf;

			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			JL_GL_ERROR(jlgr, 1,"glGetShaderiv");
			if (infoLen) {
				buf = (char*) malloc(infoLen);
				if (buf) {
					glGetShaderInfoLog(shader, infoLen, NULL, buf);
					jl_print(jlgr->jl,
						"Could not compile shader:%s",buf);
					exit(-1);
				}
				glDeleteShader(shader);
				shader = 0;
			}
		}
	}
	return shader;
}

GLuint jl_gl_glsl_prg_create(jlgr_t* jlgr, const char* pVertexSource,
	const char* pFragmentSource)
{
	GLuint vertexShader = jl_gl_load_shader(jlgr, GL_VERTEX_SHADER, pVertexSource);
	if (!vertexShader) {
		jl_print(jlgr->jl, "couldn't load vertex shader");
		exit(-1);
	}

	GLuint pixelShader = jl_gl_load_shader(jlgr, GL_FRAGMENT_SHADER, pFragmentSource);
	if (!pixelShader) {
		jl_print(jlgr->jl, "couldn't load fragment shader");
		exit(-1);
	}

	GLuint program = glCreateProgram();
	JL_GL_ERROR(jlgr, 0,"glCreateProgram");
	if (program) {
		GLint linkStatus = GL_FALSE;

		glAttachShader(program, vertexShader);
		JL_GL_ERROR(jlgr, 0,"glAttachShader (vertex)");
		glAttachShader(program, pixelShader);
		JL_GL_ERROR(jlgr, 0,"glAttachShader (fragment)");
		glLinkProgram(program);
		JL_GL_ERROR(jlgr, 0,"glLinkProgram");
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		JL_GL_ERROR(jlgr, 0,"glGetProgramiv");
		glValidateProgram(program);
		JL_GL_ERROR(jlgr, 1,"glValidateProgram");
		if (linkStatus != GL_TRUE) {
			GLint bufLength = 0;
			char* buf;

			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
			JL_GL_ERROR(jlgr, 1,"glGetProgramiv");
			if (bufLength) {
				buf = (char*) malloc(bufLength);
				if (buf) {
					glGetProgramInfoLog(program, bufLength, NULL, buf);
					jl_print(jlgr->jl,
						"Could not link program:%s",buf);
					exit(-1);
				}else{
					jl_print(jlgr->jl, "failed malloc");
					exit(-1);
				}
			}else{
				glDeleteProgram(program);
				jl_print(jlgr->jl, "no info log");
				exit(-1);
			}
		}
	}else{
		jl_print(jlgr->jl, "Failed to load program");
		exit(-1);
	}
	return program;
}

static void jl_gl_texture_make__(jlgr_t* jlgr, uint32_t *tex) {
	glGenTextures(1, tex);
#ifdef JL_DEBUG_LIB
	if(!(*tex)) {
		JL_GL_ERROR(jlgr, 0, "jl_gl_texture_make__: glGenTextures");
		jl_print(jlgr->jl, "jl_gl_texture_make__: GL tex = 0");
		exit(-1);
	}
	JL_GL_ERROR(jlgr, 0, "jl_gl_texture_make__: glGenTextures");
#endif
}

// Set the bound texture.  pm is the pixels 0 - blank texture.
static void jl_gl_texture_set__(jlgr_t* jlgr, uint8_t* px, uint16_t w, uint16_t h,
	uint8_t bytepp)
{
	GLenum format = (bytepp == 3) ? GL_RGB : GL_RGBA;
	glTexImage2D(
		GL_TEXTURE_2D, 0,		/* target, level */
		format,				/* internal format */
		w, h, 0,			/* width, height, border */
		format, GL_UNSIGNED_BYTE,	/* external format, type */
		px				/* pixels */
	);
	JL_GL_ERROR(jlgr, w, "texture image 2D");
}

static void jl_gl_texpar_set__(jlgr_t* jlgr) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	JL_GL_ERROR(jlgr, 0,"glTexParameteri");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	JL_GL_ERROR(jlgr, 1,"glTexParameteri");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	JL_GL_ERROR(jlgr, 2,"glTexParameteri");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	JL_GL_ERROR(jlgr, 3,"glTexParameteri");
}

static void jl_gl_texture__bind__(jlgr_t* jlgr, uint32_t tex) {
	glBindTexture(GL_TEXTURE_2D, tex);
	JL_GL_ERROR(jlgr, tex,"jl_gl_texture_bind__: glBindTexture");
}

// Bind a texture.
#ifdef JL_DEBUG_LIB
static void jl_gl_texture_bind__(jlgr_t* jlgr, uint32_t tex) {
	if(tex == 0) {
		jl_print(jlgr->jl, "jl_gl_texture_bind__: GL tex = 0");
		exit(-1);
	}
	jl_gl_texture__bind__(jlgr, tex);
}
#else
	#define jl_gl_texture_bind__(jlgr, tex) jl_gl_texture__bind__(jlgr, tex)
#endif

// Unbind a texture
static void jl_gl_texture_off__(jlgr_t* jlgr) {
	jl_gl_texture__bind__(jlgr, 0);
}

// Make & Bind a new texture.
static void jl_gl_texture_new__(jlgr_t* jlgr, uint32_t *tex, uint8_t* px,
	uint16_t w, uint16_t h, uint8_t bytepp)
{
	jl_print_function(jlgr->jl, "jl_gl_texture_new__");
	// Make the texture
	jl_gl_texture_make__(jlgr, tex);
	// Bind the texture
	jl_gl_texture_bind__(jlgr, *tex);
	// Set texture
	jl_gl_texture_set__(jlgr, px, w, h, bytepp);
	// Set the texture parametrs.
	jl_gl_texpar_set__(jlgr);
	jl_print_return(jlgr->jl, "jl_gl_texture_new__");
}

/*
// Make & Bind a new depth buffer.
static void jl_gl_depthbuffer_new__(jlgr_t* jlgr,uint32_t*db ,uint16_t w,uint16_t h) {
	// Make the depth buffer.
	_jl_gl_depthbuffer_make(jlgr, db);
	// Bind the depth buffer
	_jl_gl_depthbuffer_bind(jlgr, *db);
	// Set the depth buffer
	jl_gl_depthbuffer_set__(jlgr, w, h);
}
*/
// Make a texture - doesn't free "pixels"
uint32_t jl_gl_maketexture(jlgr_t* jlgr, void* pixels,
	uint32_t width, uint32_t height, uint8_t bytepp)
{
	uint32_t texture;

	jl_print_function(jlgr->jl, "GL_MkTex");
	if (!pixels) {
		jl_print(jlgr->jl, "null pixels");
		exit(-1);
	}
	JL_PRINT_DEBUG(jlgr->jl, "generating texture (%d,%d)",width,height);
	// Make the texture.
	jl_gl_texture_new__(jlgr, &texture, pixels, width, height, bytepp);
	jl_print_return(jlgr->jl, "GL_MkTex");
	return texture;
}

//Lower Level Stuff
static inline void jl_gl_usep__(jlgr_t* jlgr, GLuint prg) {
#ifdef JL_DEBUG_LIB
	if(!prg) {
		jl_print(jlgr->jl, "shader program uninit'd!");
		exit(-1);
	}
#endif
	glUseProgram(prg);
	JL_GL_ERROR(jlgr, prg, "glUseProgram");
}

static void jl_gl_setalpha__(jlgr_t* jlgr, float a) {
	glUniform1f(jlgr->effects.alpha.uniforms.newcolor_malpha, a);
	JL_GL_ERROR(jlgr, 0,"glUniform1f");
}

static void jl_gl_uniform3f__(jlgr_t* jlgr, GLint uv, float x, float y, float z)
{
	glUniform3f(uv, x, y, z);
	JL_GL_ERROR(jlgr, 0,"glUniform3f");
}

static void jl_gl_uniform4f__(jlgr_t* jlgr, GLint uv, float x, float y, float z,
	float w)
{
	glUniform4f(uv, x, y, z, w);
	JL_GL_ERROR(jlgr, 0,"glUniform4f");
}

//This pushes VBO "buff" up to the shader's vertex attribute "vertexAttrib"
//Set xyzw to 2 if 2D coordinates 3 if 3D. etc.
void _jl_gl_setv(jlgr_t* jlgr, uint32_t* buff, uint32_t vertexAttrib,
	uint8_t xyzw)
{
	// Bind Buffer
	jl_gl_buffer_use__(jlgr, buff);
	// Set Vertex Attrib Pointer
	glEnableVertexAttribArray(vertexAttrib);
	JL_GL_ERROR(jlgr, vertexAttrib,"glEnableVertexAttribArray");
	glVertexAttribPointer(
		vertexAttrib,	// attribute
		xyzw,		// x+y+z = 3
		GL_FLOAT,	// type
		GL_FALSE,	// normalized?
		0,		// stride
		0		// array buffer offset
	);
	JL_GL_ERROR(jlgr, 0,"glVertexAttribPointer");
}

static void _jl_gl_draw_arrays(jlgr_t* jlgr, GLenum mode, uint8_t count) {
	glDrawArrays(mode, 0, count);
	JL_GL_ERROR(jlgr, 0,"glDrawArrays");
}

static inline void _jl_gl_init_disable_extras(jlgr_t* jlgr) {
	glDisable(GL_DEPTH_TEST);
	JL_GL_ERROR(jlgr, 0, "glDisable(GL_DEPTH_TEST)");
	glDisable(GL_DITHER);
	JL_GL_ERROR(jlgr, 0, "glDisable(GL_DITHER)");
}

static inline void _jl_gl_init_enable_alpha(jlgr_t* jlgr) {
	glEnable(GL_BLEND);
	JL_GL_ERROR(jlgr, 0,"glEnable( GL_BLEND )");
//	glEnable(GL_CULL_FACE);
//	JL_GL_ERROR(jlgr, 0,"glEnable( GL_CULL_FACE )");
	glBlendColor(0.f,0.f,0.f,0.f);
	JL_GL_ERROR(jlgr, 0,"glBlendColor");
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	JL_GL_ERROR(jlgr, 0,"glBlendFunc");
}

// Copy & Push vertices to a VBO.
static void jl_gl_vertices__(jlgr_t* jlgr, const float *xyzw, uint8_t vertices,
	float* cv, uint32_t* gl)
{
	uint16_t items = (vertices*3);

	// Copy Vertices
	jl_mem_copyto(xyzw, cv, items * sizeof(float));
	// Copy Buffer Data "cv" to Buffer "gl"
	jl_gl_buffer_set__(jlgr, gl, cv, items);
}

static void jl_gl_vo_vertices(jlgr_t* jlgr, jl_vo_t* pv, const float *xyzw,
	uint32_t vertices)
{
	pv->vc = vertices;
	if(vertices) {
		// Re-Allocate pv->cc
		pv->cc = jl_mem(jlgr->jl, pv->cc, vertices * sizeof(float) * 4);
		// Re-Allocate pv->cv
		pv->cv = jl_mem(jlgr->jl, pv->cv, vertices * sizeof(float) * 3);
		// Set pv->cv & pv->gl
		jl_gl_vertices__(jlgr, xyzw, vertices, pv->cv, &pv->gl);
	}
}

void jl_gl_vo_free(jlgr_t* jlgr, jl_vo_t *pv) {
	// Free GL VBO
	jl_gl_buffer_old__(jlgr, &pv->gl);
	// Free GL Texture Buffer
	jl_gl_buffer_old__(jlgr, &pv->bt);
	// Free Converted Vertices & Colors
	if(pv->cv) pv->cv = jl_mem(jlgr->jl, pv->cv, 0);
	if(pv->cc) pv->cc = jl_mem(jlgr->jl, pv->cc, 0);
	// Free main structure
	pv = jl_mem(jlgr->jl, (void**)&pv, 0);
}

// TODO: MOVE
void jl_gl_pbo_new(jlgr_t* jlgr, jl_tex_t* texture, uint8_t* pixels,
	uint16_t w, uint16_t h, uint8_t bpp)
{
	jl_print_function(jlgr->jl, "GL_PBO_NEW");
	jl_gl_texture_make__(jlgr, &(texture->gl_texture));
	jl_gl_texture__bind__(jlgr, texture->gl_texture);
	jl_gl_texpar_set__(jlgr);
	jl_gl_texture_set__(jlgr, pixels, w, h, bpp);
	jl_print_return(jlgr->jl, "GL_PBO_NEW");
}

// TODO: MOVE
void jl_gl_pbo_set(jlgr_t* jlgr, jl_tex_t* texture, uint8_t* pixels,
	uint16_t w, uint16_t h, uint8_t bpp)
{
	GLenum format = GL_RGBA;

	if(bpp == 3) format = GL_RGB;

	// Bind Texture
	jl_gl_texture__bind__(jlgr, texture->gl_texture);
	// Copy to texture.
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, format, GL_UNSIGNED_BYTE,
		pixels);
	JL_GL_ERROR(jlgr, 0, "jl_gl_pbo_set__: glTexSubImage2D");
}

static inline void jl_gl_viewport__(jlgr_t* jlgr, uint16_t w, uint16_t h) {
	glViewport(0, 0, w, h);
	JL_GL_ERROR(jlgr, w * h, "glViewport");
}

static void jl_gl_framebuffer_free__(jlgr_t* jlgr, uint32_t *fb) {
	glDeleteFramebuffers(1, fb);
	JL_GL_ERROR(jlgr, *fb, "glDeleteFramebuffers");
	*fb = 0;
}

static void jl_gl_framebuffer_make__(jlgr_t* jlgr, uint32_t *fb) {
	glGenFramebuffers(1, fb);
	if(!(*fb)) {
		jl_print(jlgr->jl, "jl_gl_framebuffer_make__: GL FB = 0");
		exit(-1);
	}
	JL_GL_ERROR(jlgr, *fb,"glGenFramebuffers");
}

static void jl_gl_pr_obj_make_tx__(jlgr_t* jlgr, jl_pr_t *pr) {
	// Make a new texture for pre-renderering.  The "NULL" sets it blank.
	jl_gl_texture_new__(jlgr, &(pr->tx), NULL, pr->w, pr->h, 0);
	jl_gl_texture_off__(jlgr);
}

static void jl_gl_framebuffer_off__(jlgr_t* jlgr) {
	// Unbind the texture.
	jl_gl_texture_off__(jlgr);
	// Unbind the depthbuffer.
	// jl_gl_depthbuffer_off__(jlgr);
	// Unbind the framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	JL_GL_ERROR(jlgr, 0,"jl_gl_framebuffer_off__: glBindFramebuffer");
	// Render to the whole screen.
	jl_gl_viewport_screen(jlgr);
}

static void jl_gl_framebuffer_init__(jlgr_t* jlgr, jl_pr_t* pr) {
	if(pr->fb == 0 || pr->tx == 0/* || pr->db == 0*/) {
		// Make frame buffer
		jl_gl_framebuffer_make__(jlgr, &pr->fb);

		// Make a new Depthbuffer.
		//jl_gl_depthbuffer_new__(jlgr, &(pr->db), pr->w, pr->h);
		//jl_gl_depthbuffer_off__(jlgr);

		// Make the texture.
		jl_gl_pr_obj_make_tx__(jlgr, pr);

		// Make & Bind a new Framebuffer.
		// Recursively bind the framebuffer.
		jl_gl_framebuffer_use__(jlgr, pr);
		// Attach depth and texture buffer.
		jl_gl_framebuffer_addtx__(jlgr, pr->tx);
		jl_gl_framebuffer_adddb__(jlgr, pr->db);
		jl_gl_framebuffer_status__(jlgr);

		// Set Viewport to image and clear.
		jl_gl_viewport__(jlgr, pr->w, pr->h);
		// Clear the pre-renderer.
		jl_gl_clear(jlgr, 0.f, 0.f, 0.f, 0.f);
	}
}

static void jl_gl_framebuffer_use__(jlgr_t* jlgr, jl_pr_t* pr) {
	jl_print_function(jlgr->jl, "jl_gl_framebuffer_use__");
	jl_gl_framebuffer_init__(jlgr, pr);
	if(pr->w == 0) {
		jl_print(jlgr->jl,
		 "jl_gl_framebuffer_use__ failed: 'w' must be more than 0");
		exit(-1);
	}else if(pr->h == 0) {
		jl_print(jlgr->jl,
		 "jl_gl_framebuffer_use__ failed: 'h' must be more than 0");
		exit(-1);
	}else if((pr->w > GL_MAX_TEXTURE_SIZE)||(pr->h > GL_MAX_TEXTURE_SIZE)) {
		jl_print(jlgr->jl, "_jl_gl_pr_obj_make() failed:");
		jl_print(jlgr->jl, "w = %d,h = %d", pr->w, pr->h);
		jl_print(jlgr->jl, "texture is too big for graphics card.");
		exit(-1);
	}
	// Bind the texture.
	jl_gl_texture_bind__(jlgr, pr->tx);
	// Bind the depthbuffer.
	//_jl_gl_depthbuffer_bind(jlgr, pr->db);
	// Bind the framebuffer.
	glBindFramebuffer(GL_FRAMEBUFFER, pr->fb);
	JL_GL_ERROR(jlgr, pr->fb,"glBindFramebuffer");
	// Render on the whole framebuffer [ lower left -> upper right ]
	jl_gl_viewport__(jlgr, pr->w, pr->h);
	jl_print_return(jlgr->jl, "jl_gl_framebuffer_use__");
}

// add a texture to a framebuffer object.
static void jl_gl_framebuffer_addtx__(jlgr_t* jlgr, uint32_t tx) {
	// Set "*tex" as color attachment #0.
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, tx, 0);
	JL_GL_ERROR(jlgr, tx,"jl_gl_framebuffer_addtx: glFramebufferTexture2D");
}

// add a depthbuffer to a framebuffer object.
static void jl_gl_framebuffer_adddb__(jlgr_t* jlgr, uint32_t db) {
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, db);
	JL_GL_ERROR(jlgr, db,"make pr: glFramebufferRenderbuffer");
}

static void jl_gl_framebuffer_status__(jlgr_t* jlgr) {
	// Check to see if framebuffer was made properly.
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
		jl_print(jlgr->jl, "Frame buffer not complete!");
		exit(-1);
	}
}

static void _jl_gl_texture_free(jlgr_t* jlgr, uint32_t *tex) {
	glDeleteTextures(1, tex);
	JL_GL_ERROR(jlgr, 0, "_jl_gl_texture_free: glDeleteTextures");
	*tex = 0;
}

/*
static void _jl_gl_depthbuffer_free(jlgr_t* jlgr, uint32_t *db) {
	glDeleteRenderbuffers(1, db);
	JL_GL_ERROR(jlgr,*db,"_jl_gl_depthbuffer_free: glDeleteRenderbuffers");
	*db = 0;
}

static void _jl_gl_depthbuffer_make(jlgr_t* jlgr, uint32_t *db) {
	glGenRenderbuffers(1, db);
	if(!(*db)) {
		jl_print(jlgr->jl, "_jl_gl_depthbuffer_make: GL buff=0");
		exit(-1);
	}
	JL_GL_ERROR(jlgr,*db,"make pr: glGenRenderbuffers");
}

static void jl_gl_depthbuffer_set__(jlgr_t* jlgr, uint16_t w, uint16_t h) {
	if(!w) {
		jl_print(jlgr->jl, "jl_gl_depthbuffer_set: w = 0");
		exit(-1);
	}
	if(!h) {
		jl_print(jlgr->jl, "jl_gl_depthbuffer_set: h = 0");
		exit(-1);
	}
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);
	JL_GL_ERROR(jlgr, w, "make pr: glRenderbufferStorage");
}

static void _jl_gl_depthbuffer_bind(jlgr_t* jlgr, uint32_t db) {
	if(db == 0) {
		jl_print(jlgr->jl, "_jl_gl_depthbuffer_bind: GL db = 0");
		exit(-1);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, db);
	JL_GL_ERROR(jlgr, db,"_jl_gl_depthbuffer_bind: glBindRenderbuffer");
}

static void jl_gl_depthbuffer_off__(jlgr_t* jlgr) {
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	JL_GL_ERROR(jlgr, 0,"jl_gl_depthbuffer_off__: glBindRenderbuffer");
}
*/

static void _jl_gl_txtr(jlgr_t* jlgr, jl_vo_t** pv, float a, uint8_t is_rt) {
	if((*pv) == NULL) (*pv) = &jlgr->gl.temp_vo;
	// Set Simple Variabes
	(*pv)->a = a;
	// Make sure non-textured colors aren't attempted
	(*pv)->tx = 0;
}

static inline uint8_t _jl_gl_set_shader(jlgr_t* jlgr, jl_vo_t* pv) {
	uint8_t isTextured = (!!pv->tx);
	jl_gl_usep__(jlgr, isTextured ? jlgr->effects.alpha.program : jlgr->gl.prg.color.program);
	return isTextured;
}

// Prepare to draw a solid color
static inline void _jl_gl_draw_colr(jlgr_t* jlgr, jl_vo_t* pv) {
	// Bind Colors to shader
	_jl_gl_setv(jlgr, &pv->bt, jlgr->gl.prg.color.attributes.texpos_color, 4);
}

// Prepare to draw a texture with texture coordinates "tc". 
static void _jl_gl_draw_txtr(jlgr_t* jlgr, float a, uint32_t tx, uint32_t* tc) {
	jl_print_function(jlgr->jl, "OPENGL/Draw Texture");
	// Bind Texture Coordinates to shader
	_jl_gl_setv(jlgr, tc, jlgr->effects.alpha.attributes.texpos_color, 2);
	// Set Alpha Value In Shader
	jl_gl_setalpha__(jlgr, a);
	// Bind the texture
	jl_gl_texture_bind__(jlgr, tx);
	jl_print_return(jlgr->jl, "OPENGL/Draw Texture");
}

static void jl_gl_draw_vertices(jlgr_t* jlgr, uint32_t* gl, int32_t attr) {
	_jl_gl_setv(jlgr, gl, attr, 3);
}

static void jl_gl_draw_final__(jlgr_t* jlgr, uint8_t rs, uint32_t vc) {
	_jl_gl_draw_arrays(jlgr, rs ? GL_TRIANGLES : GL_TRIANGLE_FAN, vc);
}

static void _jl_gl_draw_onts(jlgr_t* jlgr, uint32_t* gl, uint8_t isTextured,
	uint8_t rs, uint32_t vc)
{
	// Update the position variable in shader.
	jl_gl_draw_vertices(jlgr, gl, isTextured ?
		jlgr->effects.alpha.attributes.position :
		jlgr->gl.prg.color.attributes.position);
	// Draw the image on the screen!
	jl_gl_draw_final__(jlgr, rs, vc);
}

/************************/
/***  ETOM Functions  ***/
/************************/

// Set the viewport to the screen size.
void jl_gl_viewport_screen(jlgr_t* jlgr) {
	jl_gl_viewport__(jlgr, jlgr->wm.w, jlgr->wm.h);
}

void jl_gl_pr_use_(jlgr_t* jlgr, jl_pr_t* pr) {
	// Render to the framebuffer.
	if(pr) jl_gl_framebuffer_use__(jlgr, pr);
	else jl_gl_framebuffer_off__(jlgr);
	// Reset the aspect ratio.
	jlgr->gl.cp = pr;
}

// Stop drawing to a pre-renderer.
void jl_gl_pr_off(jlgr_t* jlgr) {
	// Render to the screen
	jl_gl_framebuffer_off__(jlgr);
	// Reset the aspect ratio.
	jlgr->gl.cp = NULL;
}

// Set vertices for a polygon.
void jl_gl_poly(jlgr_t* jlgr, jl_vo_t* pv, uint32_t vertices, const float *xyzw) {
	const float FS_RECT[] = {
		0.,jl_gl_ar(jlgr),0.,
		0.,0.,0.,
		1.,0.,0.,
		1.,jl_gl_ar(jlgr),0.
	};

	if(pv == NULL) pv = &jlgr->gl.temp_vo;
	if(xyzw == NULL) xyzw = FS_RECT;
	// Rendering Style = polygon
	pv->rs = 0;
	// Set the vertices of vertex object "pv"
	jl_gl_vo_vertices(jlgr, pv, xyzw, vertices);
}

// Set vertices for vector triangles.
void jl_gl_vect(jlgr_t* jlgr, jl_vo_t* pv, uint32_t vertices, const float *xyzw) {
	if(pv == NULL) pv = &jlgr->gl.temp_vo;
	// Rendering Style = triangles
	pv->rs = 1;
	// Set the vertices of vertex object "pv"
	jl_gl_vo_vertices(jlgr, pv, xyzw, vertices);
}

// Set colors to "cc" in vertex oject "pv"
static void jl_gl_clrc(jlgr_t* jlgr, jl_vo_t* pv, float* cc) {
	//
	pv->tx = 0;
	// 
	jl_mem_copyto(cc, pv->cc, pv->vc * 4 * sizeof(float));
	// Set Color Buffer "pv->bt" to "pv->cc"
	jl_gl_buffer_set__(jlgr, &pv->bt, pv->cc, pv->vc * 4);
}

/**
 * Change the coloring scheme for a vertex object to a gradient.
 * @param jl: The library context.
 * @param vo: The Vertex Object
 * @param rgba: { (4 * vertex count) values }
**/
void jlgr_vo_color_gradient(jlgr_t* jlgr, jl_vo_t* vo, float* rgba) {
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	jl_gl_clrc(jlgr, vo, rgba);
}

/**
 * Change the coloring scheme for a vertex object to a solid.
 * @param jl: The library context.
 * @param vo: The Vertex Object
 * @param rgba: { 4 values }
**/
void jlgr_vo_color_solid(jlgr_t* jlgr, jl_vo_t* vo, float* rgba) {
	if(vo == NULL) vo = &jlgr->gl.temp_vo;
	float rgbav[4 * vo->vc];
	uint32_t i;

	for(i = 0; i < vo->vc; i++) {
		jl_mem_copyto(rgba, &(rgbav[i * 4]), 4 * sizeof(float));
	}
	jl_gl_clrc(jlgr, vo, rgbav);
}

// Set texturing to a bitmap
void jl_gl_txtr_(jlgr_t* jlgr, jl_vo_t* vo, float a, uint32_t tx) {
	_jl_gl_txtr(jlgr, &vo, a, 0);
	vo->tx = tx;
#ifdef JL_DEBUG_LIB
	if(!vo->tx) {
		jl_print(jlgr->jl, "Error: Texture=0!");
		jl_print_stacktrace(jlgr->jl);
		exit(-1);
	}
#endif
	jl_gl_vo_txmap(jlgr, vo, 0, 0, -1);
}

//TODO:MOVE
// Shader true if texturing, false if coloring
// X,Y,Z are all [0. -> 1.]
// X,Y are turned into [-.5 -> .5] - center around zero.
static void jl_gl_translate_transform__(jlgr_t* jlgr, int32_t translate,
	int32_t transform, uint32_t which, float xe, float ye, float ze,
	float xm, float ym, float zm, float ar)
{
	// Determine which shader to use
	jl_gl_usep__(jlgr, which);
	// Set the uniforms
	glUniform3f(translate, xe - (1./2.), ye - (ar/2.), ze);
	JL_GL_ERROR(jlgr, 0,"glUniform3f - translate");
	glUniform4f(transform, 2. * xm, 2. * (ym / ar), 2. * zm, 1.f);
	JL_GL_ERROR(jlgr, 0,"glUniform3f - transform");
}

void jl_gl_transform_pr_(jlgr_t* jlgr, jl_pr_t* pr, float x, float y, float z,
	float xm, float ym, float zm)
{
	jl_print_function(jlgr->jl, "OPENGL TRANSFORM!");
	float ar = jl_gl_ar(jlgr);

	jl_gl_translate_transform__(jlgr, jlgr->gl.prg.texture.uniforms.translate,
		jlgr->gl.prg.texture.uniforms.transform,
		jlgr->gl.prg.texture.program, x, y, z, xm, ym, zm, ar);
	jl_print_return(jlgr->jl, "OPENGL TRANSFORM!");
}

void jl_gl_transform_vo_(jlgr_t* jlgr, jl_vo_t* vo, float x, float y, float z,
	float xm, float ym, float zm)
{
	float ar = jl_gl_ar(jlgr);
	if(vo == NULL) vo = &jlgr->gl.temp_vo;

	jl_gl_translate_transform__(jlgr,
		vo->tx ?
			jlgr->effects.alpha.uniforms.translate :
			jlgr->gl.prg.color.uniforms.translate,
		vo->tx ?
			jlgr->effects.alpha.uniforms.transform :
			jlgr->gl.prg.color.uniforms.transform,
		vo->tx ?
			jlgr->effects.alpha.program :
			jlgr->gl.prg.color.program,
		x, y, z, xm, ym, zm, ar);
}

void jl_gl_transform_chr_(jlgr_t* jlgr, float x, float y, float z,
	float xm, float ym, float zm)
{
	float ar = jl_gl_ar(jlgr);

	jl_gl_translate_transform__(jlgr, jlgr->effects.hue.uniforms.translate,
		jlgr->effects.hue.uniforms.transform, 
		jlgr->effects.hue.program, x, y, z, xm, ym, zm, ar);
}

//Draw object with "vertices" vertices.  The vertex data is in "x","y" and "z".
//"map" refers to the charecter map.  0 means don't zoom in to one character.
//Otherwise it will zoom in x16 to a single charecter
/**
 * If "pv" is NULL then draw what's on the temporary buffer
 * Else render vertex object "pv" on the screen.
*/
void jl_gl_draw(jlgr_t* jlgr, jl_vo_t* pv) {
	jl_print_function(jlgr->jl, "OPENGL/Draw");
	// Use Temporary Vertex Object If no vertex object.
	if(pv == NULL) pv = &jlgr->gl.temp_vo;
	// Determine which shader to use: texturing or coloring?
	uint8_t isTextured = _jl_gl_set_shader(jlgr, pv);
	// Set texture and transparency if texturing.  If colors: bind colors
	if(pv->tx) _jl_gl_draw_txtr(jlgr, pv->a, pv->tx, &pv->bt);
	else _jl_gl_draw_colr(jlgr, pv);
	// Draw onto the screen.
	_jl_gl_draw_onts(jlgr, &pv->gl, isTextured, pv->rs, pv->vc);
	jl_print_return(jlgr->jl, "OPENGL/Draw");
}

/**
 * If "pv" is NULL then draw what's on the temporary buffer
 * Else render vertex object "pv" on the screen.
*/
void jl_gl_draw_chr(jlgr_t* jlgr, jl_vo_t* pv,
	float r, float g, float b, float a)
{
	// Use Temporary Vertex Object If no vertex object.
	if(pv == NULL) pv = &jlgr->gl.temp_vo;
	// Set Shader
	jl_gl_usep__(jlgr, jlgr->effects.hue.program);
	// Bind Texture Coordinates to shader
	_jl_gl_setv(jlgr, &pv->bt, jlgr->effects.hue.attributes.texpos_color, 2);
	// Set New Color In Shader
	jl_gl_uniform4f__(jlgr, jlgr->effects.hue.uniforms.newcolor_malpha, r, g, b, a);
	// Bind the texture
	glBindTexture(GL_TEXTURE_2D, pv->tx);
	JL_GL_ERROR(jlgr, pv->tx, "jl_gl_draw_chr: glBindTexture");
	// Update the position variable in shader.
	jl_gl_draw_vertices(jlgr, &pv->gl, jlgr->effects.hue.attributes.position);
	// Draw the image on the screen!
	jl_gl_draw_final__(jlgr, pv->rs, pv->vc);
}

// Draw the pre-rendered texture.
void jl_gl_draw_pr_(jl_t* jl, jl_pr_t* pr) {
	jlgr_t* jlgr = jl->jlgr;

	jl_print_function(jlgr->jl, "DRAW PR!");
	// Initialize Framebuffer, if not already init'd
	jl_gl_framebuffer_init__(jlgr, pr);
	// Use pre-mixed texturing shader.
	jl_gl_usep__(jlgr, jlgr->gl.prg.texture.program);
	// Bind Texture Coordinates to shader
	_jl_gl_setv(jlgr, &jlgr->gl.default_tc, jlgr->gl.prg.texture.attributes.texpos_color, 2);
	// Bind the texture
	jl_gl_texture_bind__(jlgr, pr->tx);
	// Update the position variable in shader.
	_jl_gl_setv(jlgr, &pr->gl, jlgr->gl.prg.texture.attributes.position, 3);
	// Draw the image on the screen!
	_jl_gl_draw_arrays(jlgr, GL_TRIANGLE_FAN, 4);
	jl_print_return(jlgr->jl, "DRAW PR!");
}

static int32_t _jl_gl_getu(jlgr_t* jlgr, GLuint prg, const char *var) {
	int32_t a;
	if((a = glGetUniformLocation(prg, var)) == -1) {
		jl_print(jlgr->jl, ":opengl: bad name; is: %s", var);
		exit(-1);
	}
	JL_GL_ERROR(jlgr, a,"glGetUniformLocation");
	return a;
}

void _jl_gl_geta(jlgr_t* jlgr, GLuint prg, int32_t *attrib, const char *title) {
	if((*attrib = glGetAttribLocation(prg, title)) == -1) {
		jl_print(jlgr->jl, 
			"attribute name is either reserved or non-existant");
		exit(-1);
	}
}

/***	  @cond	   ***/
/************************/
/*** Static Functions ***/
/************************/

static inline void _jl_gl_init_setup_gl(jlgr_t* jlgr) {
	JL_PRINT_DEBUG(jlgr->jl, "setting properties....");
	//Disallow Dither & Depth Test
	_jl_gl_init_disable_extras(jlgr);
	//Set alpha=0 to transparent
	_jl_gl_init_enable_alpha(jlgr);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	JL_PRINT_DEBUG(jlgr->jl, "set glproperties.");
}

/**
 * Create a GLSL shader program.
 * @param jlgr: The library context.
 * @param glsl: The jlgr_glsl_t object to initialize.
 * @param vert: The vertex shader - NULL for texture support.
 * @param frag: The fragment shader.
 * @param effectName: Name of effect to use ( must be a uniform variable ).
**/
void jlgr_gl_shader_init(jlgr_t* jlgr, jlgr_glsl_t* glsl, const char* vert,
	const char* frag, const char* effectName)
{
	const char* vertShader = vert ? vert : JL_SHADER_TEX_VERT;
	// Program
	glsl->program = jl_gl_glsl_prg_create(jlgr, vertShader, frag);
	// Translate Vector
	glsl->uniforms.translate = _jl_gl_getu(jlgr,glsl->program,"translate");
	// Transform Vector
	glsl->uniforms.transform = _jl_gl_getu(jlgr,glsl->program, "transform");
	// Position Attribute
	_jl_gl_geta(jlgr, glsl->program, &glsl->attributes.position, "position");
	// If custom vertex shader, don't assume texture is defined
	if(vert) {
		// Color Attribute
		_jl_gl_geta(jlgr, glsl->program, &glsl->attributes.texpos_color,
			"acolor");
	}else{
		// Texture
		glsl->uniforms.texture =
			_jl_gl_getu(jlgr, glsl->program, "texture");
		// Texture Position Attribute
		_jl_gl_geta(jlgr, glsl->program, &glsl->attributes.texpos_color,
			 "texpos");
	}
	// Effects
	if(effectName) {
		glsl->uniforms.newcolor_malpha =
			_jl_gl_getu(jlgr, glsl->program, effectName);
	}
}

static inline void _jl_gl_init_shaders(jlgr_t* jlgr) {
	JL_PRINT_DEBUG(jlgr->jl, "Making GLSL program: texture");
	jlgr_gl_shader_init(jlgr, &jlgr->gl.prg.texture, NULL,
		JL_SHADER_TEX_FRAG, NULL);
	JL_PRINT_DEBUG(jlgr->jl, "Making GLSL program: color");
	jlgr_gl_shader_init(jlgr, &jlgr->gl.prg.color, JL_SHADER_CLR_VERT,
		JL_SHADER_CLR_FRAG, NULL);
	JL_PRINT_DEBUG(jlgr->jl, "Set up shaders!");
}

//Load and create all resources
static inline void _jl_gl_make_res(jlgr_t* jlgr) {
	jl_print_function(jlgr->jl, "GL_Init");
	// Setup opengl properties
	_jl_gl_init_setup_gl(jlgr);
	// Create shaders and set up attribute/uniform variable communication
	_jl_gl_init_shaders(jlgr);
	//
	JL_PRINT_DEBUG(jlgr->jl, "making temporary vertex object....");
	jl_gl_vo_init(jlgr, &jlgr->gl.temp_vo);
	JL_PRINT_DEBUG(jlgr->jl, "making default texc buff!");
	// Default GL Texture Coordinate Buffer
	jl_gl_buffer_set__(jlgr, &jlgr->gl.default_tc, DEFAULT_TC, 8);
	JL_PRINT_DEBUG(jlgr->jl, "made temp vo & default tex. c. buff!");
	jl_print_return(jlgr->jl, "GL_Init");
}

static void jl_gl_pr_set__(jl_pr_t *pr, float w, float h, uint16_t w_px) {
	const float ar = h / w; // Aspect Ratio.
	const float h_px = w_px * ar; // Height in pixels.

	// Set width and height.
	pr->w = w_px;
	pr->h = h_px;
	// Set aspect ratio.
	pr->ar = ar;
}

/**	  @endcond	  **/
/************************/
/*** Global Functions ***/
/************************/

/**
 * Create an empty vertex object.
 * @param jl: The library context.
 * @param vo: A uninitialized vertex object - to initailize with 0 vertices.
**/
void jl_gl_vo_init(jlgr_t* jlgr, jl_vo_t* vo) {
	// GL VBO
	vo->gl = 0;
	// GL Texture Coordinate Buffer
	vo->bt = 0;
	// Converted Vertices
	vo->cv = NULL;
	// Vertex Count
	vo->vc = 0;
	// Converted Colors
	vo->cc = NULL;
	// Rendering Style = Polygon
	vo->rs = 0;
	// Texture
	vo->tx = 0;
}

/**
 * Change the character map for a texture.
 * @param jl: The library context.
 * @param vo: The vertext object to change.
 * @param w: How many characters wide the texture is.
 * @param h: How many characters high the texture is.
 * @param map: The character value to map.  -1 for full texture.
**/
void jl_gl_vo_txmap(jlgr_t* jlgr,jl_vo_t* vo,uint8_t w,uint8_t h,int16_t map) {
	if(map != -1) {
		float ww = (float)w;
		float hh = (float)h;
		float CX = ((float)(map%h))/ww;
		float CY = ((float)(map/h))/hh;
		float tex1[] = {
			(DEFAULT_TC[0]/ww) + CX, (DEFAULT_TC[1]/hh) + CY,
			(DEFAULT_TC[2]/ww) + CX, (DEFAULT_TC[3]/hh) + CY,
			(DEFAULT_TC[4]/ww) + CX, (DEFAULT_TC[5]/hh) + CY,
			(DEFAULT_TC[6]/ww) + CX, (DEFAULT_TC[7]/hh) + CY
		};
		jl_gl_buffer_set__(jlgr, &vo->bt, tex1, 8);
	}else{
		jl_gl_buffer_set__(jlgr, &vo->bt, DEFAULT_TC, 8);
	}
}

/**
 * Get the Aspect Ratio of the pre-renderer in use.
 * @param jl: The library context.
**/
float jl_gl_ar(jlgr_t* jlgr) {
	uint8_t thread = jl_thread_current(jlgr->jl);

	if(thread)
		return jlgr->gl.cp ? jlgr->gl.cp->ar : jlgr->wm.ar;
	else
		return jlgr->wm.ar;
}

/**
 * Get the Width for the pre-renderer in use.
**/
uint32_t jl_gl_w(jlgr_t* jlgr) {
	uint32_t w;

	w = jlgr->gl.cp ? jlgr->gl.cp->w : jlgr->wm.w;
	return w;
}

/**
 * Clear the screen with a color
 * @param jl: The library context.
 * @param r: The amount of red [ 0.f - 1.f ]
 * @param g: The amount of green [ 0.f - 1.f ]
 * @param b: The amount of blue [ 0.f - 1.f ]
 * @param a: The translucency [ 0.f - 1.f ]
**/
void jl_gl_clear(jlgr_t* jlgr, float r, float g, float b, float a) {
	glClearColor(r, g, b, a);
	JL_GL_ERROR(jlgr, a, "jl_gl_clear(): glClearColor");
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	JL_GL_ERROR(jlgr, a, "jl_gl_clear(): glClear");
}

/**
 * THREAD: Draw thread only.
 * Resize a prerenderer.
 * @param jl: The library context.
 * @param pr: The pre-renderer to resize.
 * @param w: The display width. [ 0. - 1. ]
 * @param h: The display height. [ 0. - 1. ]
 * @param w_px: The resolution in pixels along the x axis [ 1- ]
**/
void jl_gl_pr_rsz(jlgr_t* jlgr, jl_pr_t* pr, float w, float h, uint16_t w_px) {
	const float xyzw[] = {
		0.f,	h,	0.f,
		0.f,	0.f,	0.f,
		w,	0.f,	0.f,
		w,	h,	0.f
	};

	// If pre-renderer is initialized, reset.
	if(pr->fb) {
		_jl_gl_texture_free(jlgr, &(pr->tx));
//		_jl_gl_depthbuffer_free(jlgr, &(pr->db));
		jl_gl_framebuffer_free__(jlgr, &(pr->fb));
		jl_gl_buffer_old__(jlgr, &(pr->gl));
	}
	// Create the VBO.
	jl_gl_vertices__(jlgr, xyzw, 4, pr->cv, &pr->gl);
	// Set width, height and aspect ratio.
	jl_gl_pr_set__(pr, w, h, w_px);
}

/**
 * THREAD: any
 * Make a new pre-renderer.  Call to jl_gl_pr_rsz(); is necessary after this.
 * @param jlgr: The library context.
 * @param w: The display width. [ 0. - 1. ]
 * @param h: The display height. [ 0. - 1. ]
 * @param w_px: The resolution in pixels along the x axis [ 1- ]
**/
void jl_gl_pr_new(jlgr_t* jlgr, jl_pr_t* pr, float w, float h, uint16_t w_px) {
	jl_print_function(jlgr->jl, "GL_PR_NEW");
	// Set the initial pr structure values - Nothings made yet.
	pr->tx = 0;
	pr->db = 0;
	pr->fb = 0;
	pr->gl = 0;
	// Clear pr->cv
	jl_mem_clr(pr->cv, 4*sizeof(float)*3);
	// Set width, height and aspect ratio.
	jl_gl_pr_set__(pr, w, h, w_px);
	// Set transformation
	pr->scl = (jl_vec3_t) { 1., 1., 1. };
	jl_print_return(jlgr->jl, "GL_PR_NEW");
}

/**
 * Draw a pre-rendered texture.
 * @param jlgr: The library context.
 * @param pr: The pre-rendered texture.
 * @param vec: The vector of offset/translation.
 * @param scl: The scale factor.
**/
void jl_gl_pr_draw(jlgr_t* jlgr, jl_pr_t* pr, jl_vec3_t* vec, jl_vec3_t* scl) {
	if(vec == NULL) {
		jl_gl_transform_pr_(jlgr, pr,
			0.f, 0.f, 0.f, 1., 1., 1.);
	}else if(scl == NULL) {
		jl_gl_transform_pr_(jlgr, pr,
			vec->x, vec->y, vec->z, 1., 1., 1.);
	}else{
		jl_gl_transform_pr_(jlgr, pr,
			vec->x, vec->y, vec->z, scl->x, scl->y, scl->z);	
	}
	jl_gl_draw_pr_(jlgr->jl, pr);
}

void jl_gl_pr(jlgr_t* jlgr, jl_pr_t * pr, jl_fnct par__redraw) {
	jl_pr_t* oldpr = jlgr->gl.cp;

	if(!pr) {
		jl_print(jlgr->jl, "Drawing on lost pre-renderer.");
		exit(-1);
	}
	// Use the vo's pr
	jl_gl_pr_use_(jlgr, pr);
	// Render to the pr.
	par__redraw(jlgr->jl);
	// Go back to the previous pre-renderer.
	jl_gl_pr_use_(jlgr, oldpr);
}

/***	  @cond	   ***/
/************************/
/***  ETOM Functions  ***/
/************************/

void jl_gl_init__(jlgr_t* jlgr) {
#ifdef JL_GLTYPE_HAS_GLEW
	if(glewInit()!=GLEW_OK) {
		jl_print(jlgr->jl, "glew fail!(no sticky)");
		exit(-1);
	}
#endif
	jlgr->gl.cp = NULL;
	_jl_gl_make_res(jlgr);
	//Set uniform values
	JL_PRINT_DEBUG(jlgr->jl, "Setting color uniforms....");
	jl_gl_usep__(jlgr, jlgr->gl.prg.color.program);
	jl_gl_uniform3f__(jlgr, jlgr->gl.prg.color.uniforms.translate, 0.f, 0.f, 0.f);
	jl_gl_uniform4f__(jlgr, jlgr->gl.prg.color.uniforms.transform, 1.f, 1.f, 1.f,
		1.f);
	JL_PRINT_DEBUG(jlgr->jl, "Setting texture uniforms....");
	jl_gl_usep__(jlgr, jlgr->gl.prg.texture.program);
	jl_gl_uniform3f__(jlgr, jlgr->gl.prg.texture.uniforms.translate, 0.f, 0.f, 0.f);
	jl_gl_uniform4f__(jlgr, jlgr->gl.prg.texture.uniforms.transform, 1.f, 1.f, 1.f,
		1.f);
/*	jl_gl_usep__(jlgr, jlgr->effects.alpha.program);
	jl_gl_uniform3f__(jlgr, jlgr->effects.alpha.uniforms.translate, 0.f, 0.f, 0.f);
	jl_gl_uniform4f__(jlgr, jlgr->effects.alpha.uniforms.transform, 1.f, 1.f, 1.f,
		1.f);*/
	// Make sure no pre-renderer is activated.
	jl_gl_pr_off(jlgr);
}

/**	  @endcond	  **/
/***   #End of File   ***/
