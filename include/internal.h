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
#include<CL/cl.h>

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
	union{
		struct{
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
		};
		int tex[16];
	};

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




















typedef void (*pswapbufferfunctype)(SDL_Window* window);	/*	Function pointer data type.	*/
typedef void (*presize_screen)(int* event, struct uniform_location_t* uniform, glslviewShader* shader, glslviewTexture* ftexture);
typedef void (*pupdate_shader_uniform)(struct uniform_location_t* uniform, glslviewShader* shader, int width, int height);
typedef void (*pupdate_update_uniforms)(UniformLocation* uniform, glslviewShader* shader, float ttime, long int deltatime);
typedef void (*pset_viewport)(unsigned int width, unsigned int height);
typedef void (*pdisplaygraphic)(SDL_Window* drawable);

/**/
extern presize_screen glslview_resize_screen;
extern pupdate_shader_uniform glslview_update_shader_uniform;
extern pdisplaygraphic glslview_displaygraphic;
extern pupdate_update_uniforms glslview_update_uniforms;
extern pset_viewport glslview_set_viewport;
extern pswapbufferfunctype glslview_swapbuffer;					/*	Function pointer for swap default framebuffer.	*/

/**/
extern const float quad[4][3];




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
 *	@Return
 */
extern const char* glslview_getVersion(void);

/**/
extern void glslview_catchSig(int signal);

/**/
extern int glslview_readargument(int argc, const char** argv, int pass);
/**/
extern long int glslview_loadfile(const char* cfilename, void** bufferptr);


/*	TODO create function pointer.	*/
glslviewTexture* glslview_create_texture(glslviewTexture* texture, unsigned int target, int level, int internalFormat, int width, int height, int border, unsigned int format, unsigned int type, const void *pixels);
/*	TODO create function pointer.	*/
int glslview_create_shader(glslviewShader* texture, const char* cvertexSource, const char* cfragmentSource, const char* cgeometry_source, const char* ctess_c_source, const char* ctess_e_source);

/**
 *
 */
void glslview_resize_screen_gl(int* res, struct uniform_location_t* uniform, glslviewShader* shader, glslviewTexture* ftexture);
void glslview_resize_screen_vk(int* res, struct uniform_location_t* uniform, glslviewShader* shader, glslviewTexture* ftexture);

/**
 *
 */
void glslview_update_shader_uniform_gl(struct uniform_location_t* uniform, glslviewShader* shader, int width, int height);
void glslview_update_shader_uniform_vk(struct uniform_location_t* uniform, glslviewShader* shader, int width, int height);

/**
 *
 */
void glslview_update_uniforms_gl(UniformLocation* uniform, glslviewShader* shader, float ttime, long int deltatime);
void glslview_update_uniforms_vk(UniformLocation* uniform, glslviewShader* shader, float ttime, long int deltatime);

/**
 *
 */
void glslview_displaygraphic_gl(SDL_Window* drawable);
void glslview_displaygraphic_vk(SDL_Window* drawable);

/**
 *
 */
void glslview_set_viewport_gl(unsigned int width, unsigned int height);
void glslview_set_viewport_vk(unsigned int width, unsigned int height);



void glslview_rendergraphic(SDL_Window* drawable, glslviewShaderCollection* shader, float ttime, float deltatime);






extern int privatefprintf(const char* format,...);
extern int debugprintf(const char* format,...);




/**
 *
 *	@Return
 */
extern cl_context glslview_createclcontext(void* shared, unsigned int* numDevices, cl_device_id** device);
/**
 *	Release OpenCL release.
 */
extern void glslview_clrelease(void);

/**
 * 	Create OpenCL program.
 */
extern cl_program glslview_createProgram(cl_context context, unsigned int nDevices, cl_device_id* device, const char* cfilename);

/**
 *	@Return
 */
extern cl_command_queue glslview_createcommandqueue(cl_context context, cl_device_id device);
extern cl_context glslview_createCLContext(void* shared, unsigned int* ncldevices, cl_device_id** devices);
extern cl_program glslview_createCLProgram(cl_context context, unsigned int nNumDevices, cl_device_id* id, const char* cfilename, UniformLocation* uniform);
extern void glslview_acquirecltextures(cl_context context, cl_command_queue queue, cl_kernel kernel);
extern void glslview_cl_resize(unsigned int width, unsigned int height);
extern void glslview_cl_createframebuffer(unsigned int width, unsigned int height);
extern void glslview_renderclframe(cl_command_queue queue, cl_kernel kernel);



/**/
extern unsigned int vao;						/*	*/
extern unsigned int vbo;						/*	*/
extern const char* vertex;
extern const char* quadfrag;

extern SDL_GLContext glc;
extern SDL_Window* window;								/*	Window.	*/
extern int fullscreen;						/*	Set window fullscreen.	*/
extern int verbose;							/*	enable verbose.	*/
extern int debug;								/*	enable debugging.	*/
extern int compression;						/*	Use compression.	*/
extern unsigned int rendererapi;					/*	Rendering API.	*/
extern unsigned int isAlive;						/*	*/
extern int ifd;										/*	inotify file descriptor.*/
extern int wd;										/*	inotify watch directory.	*/
extern char* inotifybuf;							/*	*/
extern unsigned int numFragPaths;					/*	*/
extern unsigned int numShaderPass;
extern char* fragPath[32];							/*	Path of fragment shader.	*/
//extern UniformLocation uniform[32];				/*	uniform.	*/
//extern glslviewShader shader[32];						/*	*/
extern glslviewShaderCollection* shaders;
extern unsigned int fbo;							/*	*/
extern unsigned int ftextype;
extern unsigned int ftexinternalformat;
extern unsigned int ftexformat;
extern glslviewTexture fbackbuffertex;					/*	framebuffer texture for backbuffer uniform variable.	*/
extern glslviewTexture textures[8];						/*	*/
extern const int numTextures;
extern unsigned int nextTex;						/*	*/
extern unsigned int isPipe;
extern unsigned int use_stdin_as_buffer;			/*	*/
extern int stdin_buffer_size;

/*	Opencl.	*/
extern unsigned int usingopencl;						/*	*/
extern cl_context clcontext;							/*	*/
extern unsigned int ncldevices;							/*	*/
extern cl_device_id* cldevice;							/*	*/
extern cl_command_queue clqueue;						/*	*/
extern cl_program clprogram;							/*	*/
extern cl_kernel clkernel;								/*	*/
extern unsigned int numclframebuffer;					/*	*/
extern cl_mem clmemframetexture[2];						/*	*/
extern unsigned int clcurrent;							/*	*/
extern glslviewTexture clframetexture[2];						/*	*/
extern unsigned int numcltextures;						/*	*/
extern cl_mem cltextures[16];							/*	*/
extern UniformLocation cluniform;						/*	*/
/*	*/
extern pswapbufferfunctype glslview_swapbuffer;					/*	Function pointer for swap default framebuffer.	*/


#endif
