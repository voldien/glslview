/**
	glslview
    Copyright (C) 2016  Valdemar Lindberg

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
#ifndef _INTERNAL_H_
#define _INTERNAL_H_ 1
#include<SDL2/SDL.h>
#include<assert.h>

/*
 *	Unicode macro for converting constant string to
 */
#ifdef UNICODE  /*  UTF-16*/
	#define GLSLVIEW_TEXT(quote) L##quote
	#define GLSLVIEW_TEXT(quote)  _EX_TEXT(quote)
#else           /*	ASCII / UTF-8	*/
    #define GLSLVIEW_TEXT(quote) quote
#endif
#define GLSLVIEW_STR_HELPER(x) #x
#define GLSLVIEW_STR(x) GLSLVIEW_STR_HELPER(x)
#define COMPILED_VERSION(major, minor, revision) GLSLVIEW_STR(major)GLSLVIEW_TEXT(".")GLSLVIEW_STR(minor)GLSLVIEW_TEXT(".")GLSLVIEW_STR(revision)


/**
 *	Texture flags.
 */
#define TEXTURE_RGB 0x1
#define TEXTURE_RGBA 0x2
#define TEXTURE_COMPRESSION_RGB 0x3
#define TEXTURE_COMPRESSION_RGBA 0x4
#define TEXTURE_BGR 0x5
#define TEXTURE_BGRA 0x6
#define TEXTURE_UNSIGNED_BYTE 0x1

/**
 *
 */
typedef struct uniform_location_t{
	int time;			/*	time in seconds as float.	*/
	int resolution;		/*	resolution. */
	int deltatime;		/*	delta time.	*/
	int mouse;			/*	mouse.	*/
	int offset;			/*	offset.	*/
	int backbuffer;		/*	previous buffer.	*/
	int stdin;			/*	stdin data.	*/
	int wheel;			/*				*/
	int tex0;			/*	texture 0.	*/
	int tex1;			/*	texture 1.	*/
	int tex2;			/*	texture 2.	*/
	int tex3;			/*	texture 3.	*/
	int tex4;			/*	texture 4.	*/
	int tex5;			/*	texture 5.	*/
	int tex6;			/*	texture 6.	*/
	int tex7;			/*	texture 7.	*/
	int tex8;			/*	texture 8.	*/
	int tex9;			/*	texture 9.	*/
	int tex10;			/*	texture 10.	*/
	int tex11;			/*	texture 11.	*/
	int tex12;			/*	texture 12.	*/
	int tex13;			/*	texture 13.	*/
	int tex14;			/*	texture 14.	*/
	int tex15;			/*	texture 15.	*/
}UniformLocation;

typedef struct glslview_texture_t{
	unsigned int target;			/**/
	unsigned int texture;			/**/
	unsigned int width;				/**/
	unsigned int height;			/**/
	unsigned int internalformat;	/**/
	unsigned int type;				/**/
}glslviewTexture;

typedef struct glslview_shader_t{
	unsigned int ver;		/*	vertex shader.	*/
	unsigned int fra;		/*	fragment shader.	*/
	unsigned int geo;		/*	geometry shader.	*/
	unsigned int tesc;		/*	tessellation control shader.	*/
	unsigned int tese;		/*	tessellation evaluation shader.	*/
	unsigned int program;	/*	shader program.	*/
	unsigned int flag;		/**/
}glslviewShader;


typedef struct glslview_shader_collection_t{
	UniformLocation uniform;
	glslviewShader shader;
}glslviewShaderCollection;

typedef struct glslview_texture_collection_t{
	glslviewTexture texture;
}glslviewTextureCollection;



















typedef SDL_Window* (*pglslview_init_renderingapi)(void);
typedef void (*pglslview_release_vulkan)(void);
typedef void (*pswapbufferfunctype)(SDL_Window* window);	/*	Function pointer data type.	*/
typedef void (*presize_screen)(int* event, struct uniform_location_t* uniform, glslviewShader* shader, glslviewTexture* ftexture);
typedef void (*pupdate_shader_uniform)(struct uniform_location_t* uniform, glslviewShader* shader, int width, int height);
typedef void (*pupdate_update_uniforms)(UniformLocation* uniform, glslviewShader* shader, float ttime, long int deltatime);
typedef void (*pset_viewport)(unsigned int width, unsigned int height);
typedef void (*pdisplaygraphic)(SDL_Window* drawable);
typedef glslviewTexture* (*pglslview_create_texture)(glslviewTexture* texture, unsigned int target, int level, int internalFormat, int width, int height, int border, unsigned int format, unsigned int type, const void *pixels);
typedef int (*pglslview_create_shader)(glslviewShader* texture, const char* cvertexSource, const char* cfragmentSource, const char* cgeometry_source, const char* ctess_c_source, const char* ctess_e_source);
typedef void (*pglslview_rendergraphic)(SDL_Window* drawable, glslviewShaderCollection* shader, float ttime, float deltatime);


/**/
extern pglslview_init_renderingapi glslview_init_renderingapi;
extern pglslview_release_vulkan glslview_release_renderingapi;
extern presize_screen glslview_resize_screen;
extern pupdate_shader_uniform glslview_update_shader_uniform;
extern pdisplaygraphic glslview_displaygraphic;
extern pupdate_update_uniforms glslview_update_uniforms;
extern pset_viewport glslview_set_viewport;
extern pswapbufferfunctype glslview_swapbuffer;					/*	Function pointer for swap default framebuffer.	*/
extern pglslview_create_texture glslview_create_texture;
extern pglslview_create_shader glslview_create_shader;
extern pglslview_rendergraphic glslview_rendergraphic;


/**
 *	Initialize default function pointer.
 *	It will set all the rendering function pointer
 *	to the OpenGL version.
 */
extern void glslview_default_init(void);

/**
 *	Initialize glslsview.
 *
 *	@Return
 *
 */
extern int glslview_init(int argc, const char** argv);

/**
 *	Get glslview version.
 *
 *	@Return non NULL string.
 */
extern const char* glslview_getVersion(void);

/**
 *	Signal catch function.
 */
extern void glslview_catchSig(int signal);


/**
 *	Read user input argument.
 *
 *	@Return status.
 */
extern int glslview_readargument(int argc, const char** argv, int pass);

/**
 *
 */
extern int needsUpdate(glslviewShaderCollection* shader);

/**
 *	Load file.
 *
 *	@Return number of bytes loaded.
 */
extern long int glslview_loadfile(const char* cfilename, void** bufferptr);




/**
 *	Initialize	rendering API and create window
 *	associate with the rendering API.
 *
 *	@Return None null pointer if successfully.
 */
extern SDL_Window* glslview_init_vulkan(void);
extern SDL_Window* glslview_init_opengl(void);

/**
 *	Release rendering API resource.
 */
extern void glslview_release_vulkan(void);
extern void glslview_release_opengl(void);

/**
 *	TODO add enum for texture target.
 *
 *	@Return
 */
extern glslviewTexture* glslview_create_texture_gl(glslviewTexture* texture, unsigned int target, int level,
		int internalFormat, int width, int height, int border, unsigned int format, unsigned int type,
		const void *pixels);
extern glslviewTexture* glslview_create_texture_vk(glslviewTexture* texture, unsigned int target, int level,
		int internalFormat, int width, int height, int border, unsigned int format, unsigned int type,
		const void *pixels);


/**
 *	Create shader.
 *
 *	@Return status.
 */
extern int glslview_create_shader_gl(glslviewShader* texture, const char* cvertexSource,
		const char* cfragmentSource, const char* cgeometry_source, const char* ctess_c_source,
		const char* ctess_e_source);
extern int glslview_create_shader_vk(glslviewShader* texture, const char* cvertexSource,
		const char* cfragmentSource, const char* cgeometry_source, const char* ctess_c_source,
		const char* ctess_e_source);

/**
 *
 */
extern void glslview_resize_screen_gl(int* res, struct uniform_location_t* uniform, glslviewShader* shader, glslviewTexture* ftexture);
extern void glslview_resize_screen_vk(int* res, struct uniform_location_t* uniform, glslviewShader* shader, glslviewTexture* ftexture);

/**
 *
 */
extern void glslview_update_shader_uniform_gl(struct uniform_location_t* uniform, glslviewShader* shader, int width, int height);
extern void glslview_update_shader_uniform_vk(struct uniform_location_t* uniform, glslviewShader* shader, int width, int height);

/**
 *
 */
extern void glslview_update_uniforms_gl(UniformLocation* uniform, glslviewShader* shader, float ttime, long int deltatime);
extern void glslview_update_uniforms_vk(UniformLocation* uniform, glslviewShader* shader, float ttime, long int deltatime);

/**
 *
 */
extern void glslview_displaygraphic_gl(SDL_Window* drawable);
extern void glslview_displaygraphic_vk(SDL_Window* drawable);

/**
 *
 */
extern void glslview_set_viewport_gl(unsigned int width, unsigned int height);
extern void glslview_set_viewport_vk(unsigned int width, unsigned int height);

/**
 *
 */
extern void glslview_rendergraphic_gl(SDL_Window* drawable, glslviewShaderCollection* shader, float ttime, float deltatime);
extern void glslview_rendergraphic_vk(SDL_Window* drawable, glslviewShaderCollection* shader, float ttime, float deltatime);




/**
 *	Print verbose.
 *	The function will print depending on if the
 *	verbose variable is true or not.
 *
 *	@Return
 */
extern int glslview_verbose_printf(const char* format,...);

/**
 *	Print debug.
 *
 *	@Return
 */
extern int glslview_debug_printf(const char* format,...);





/*	Read only.	*/
extern const float quad[4][3];					/*	Display quad.	*/
extern const char* vertex;						/*	Display vertex shader.	*/

extern unsigned int vao;						/*	Display vertex array object.	*/
extern unsigned int vbo;						/*	Display vertex buffer object.	*/

/*	*/
extern SDL_GLContext glc;							/*	OpenGL Context.	*/
extern SDL_Window* window;							/*	Window.	*/
extern SDL_Window* drawable;
extern int fullscreen;								/*	Set window fullscreen.	*/
extern int verbose;									/*	enable verbose.	*/
extern int debug;									/*	enable debugging.	*/
extern int compression;								/*	Use compression.	*/
extern unsigned int rendererapi;					/*	Rendering API.	*/
extern unsigned int isAlive;						/*	*/
extern int ifd;										/*	inotify file descriptor.*/
extern int wd;										/*	inotify watch directory.	*/
extern char* inotifybuf;							/*	*/
extern unsigned int numFragPaths;					/*	*/
extern unsigned int numShaderPass;					/*	*/
extern char* fragPath[32];							/*	Path of fragment shader.	*/

extern glslviewShaderCollection* shaders;			/*	Shaders.	*/
extern unsigned int fbo;							/*	*/
extern unsigned int ftextype;						/**/
extern unsigned int ftexinternalformat;				/**/
extern unsigned int ftexformat;						/**/
extern glslviewTexture fbackbuffertex;				/*	framebuffer texture for backbuffer uniform variable.	*/
extern glslviewTexture textures[8];					/*	*/
extern glslviewTextureCollection* texturess;		/*	TODO replace textures variable.	*/
extern const int numTextures;						/*	*/
extern unsigned int nextTex;						/*	*/
extern unsigned int isPipe;							/*	Is STDIN pipe used.	*/
extern unsigned int use_stdin_as_buffer;			/*	*/
extern int stdin_buffer_size;						/*	*/


#endif
