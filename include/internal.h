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

#include<ELT/elt.h>
#include<ELT/graphic.h>



#define COMPILED_VERSION(major, minor, revision) EX_STR(major)EX_TEXT(".")EX_STR(minor)EX_TEXT(".")EX_STR(revision)


/**
 *
 */
typedef struct uniform_location_t{
	int time;			/*	time in seconds as float.	*/
	int resolution;	/*	resolution. */
	int deltatime;		/*	delta time.	*/
	int mouse;			/*	mouse.	*/
	int offset;		/*	offset.	*/
	int backbuffer;	/*	previous buffer.	*/
	int stdin;			/*	stdin data.	*/
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























typedef void (*pswapbufferfunctype)(ExWin window);	/*	Function pointer data type.	*/
typedef void (*presize_screen)(ExEvent* event, struct uniform_location_t* uniform, ExShader* shader, ExTexture* ftexture);
typedef void (*pupdate_shader_uniform)(struct uniform_location_t* uniform, ExShader* shader, int width, int height);
typedef void (*pupdate_update_uniforms)(UniformLocation* uniform, ExShader* shader, float ttime, long int deltatime);
typedef void (*pset_viewport)(unsigned int width, unsigned int height);
typedef void (*pdisplaygraphic)(ExWin drawable);

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

/**/
extern int glslview_init(int argc, const char** argv);

/**/
extern const char* glslview_getVersion(void);

/**/
extern void glslview_catchSig(int signal);

/**/
extern int glslview_readargument(int argc, const char** argv, int pass);




/**
 *
 */
void glslview_resize_screen_gl(ExEvent* event, struct uniform_location_t* uniform, ExShader* shader, ExTexture* ftexture);
void glslview_resize_screen_vk(ExEvent* event, struct uniform_location_t* uniform, ExShader* shader, ExTexture* ftexture);

/**
 *
 */
void glslview_update_shader_uniform_gl(struct uniform_location_t* uniform, ExShader* shader, int width, int height);
void glslview_update_shader_uniform_vk(struct uniform_location_t* uniform, ExShader* shader, int width, int height);

/**
 *
 */
void glslview_update_uniforms_gl(UniformLocation* uniform, ExShader* shader, float ttime, long int deltatime);
void glslview_update_uniforms_vk(UniformLocation* uniform, ExShader* shader, float ttime, long int deltatime);

/**
 *
 */
void glslview_displaygraphic_gl(ExWin drawable);
void glslview_displaygraphic_vk(ExWin drawable);

/**
 *
 */
void glslview_set_viewport_gl(unsigned int width, unsigned int height);
void glslview_set_viewport_vk(unsigned int width, unsigned int height);



void glslview_rendergraphic(ExWin drawable, ExShader* shader, UniformLocation* location, float ttime, float deltatime);






extern int privatefprintf(const char* format,...);
extern int debugprintf(const char* format,...);


typedef struct glslview_shader_t{
	UniformLocation uniform;
	ExShader shader;
}glslviewShader;

typedef struct glslview_texture_t{
	ExTexture texture;
}glslviewTexture;


/**/
extern unsigned int vao;						/*	*/
extern unsigned int vbo;						/*	*/
extern const char* vertex;

extern ExWin window;								/*	Window.	*/
extern ExBoolean fullscreen ;						/*	Set window fullscreen.	*/
extern ExBoolean verbose;							/*	enable verbose.	*/
extern ExBoolean debug;								/*	enable debugging.	*/
extern ExBoolean compression;						/*	Use compression.	*/
extern unsigned int rendererapi;					/*	Rendering API.	*/
extern unsigned int isAlive;						/*	*/
extern int ifd;										/*	inotify file descriptor.*/
extern int wd;										/*	inotify watch directory.	*/
extern char* inotifybuf;							/*	*/
extern unsigned int numFragPaths;					/*	*/
extern unsigned int numShaderPass;
extern char* fragPath[32];							/*	Path of fragment shader.	*/
extern UniformLocation uniform[32];				/*	uniform.	*/
extern ExShader shader[32];						/*	*/
extern glslviewShader* shaders;
extern unsigned int fbo;							/*	*/
extern unsigned int ftextype;
extern unsigned int ftexinternalformat;
extern unsigned int ftexformat;
extern ExTexture fbackbuffertex;					/*	framebuffer texture for backbuffer uniform variable.	*/
extern ExTexture textures[8];						/*	*/
extern const int numTextures;
extern unsigned int nextTex;						/*	*/
extern unsigned int use_stdin_as_buffer;			/*	*/
extern int stdin_buffer_size;						/*	*/


#endif
