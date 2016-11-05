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
#include<hpm/hpm.h>

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
	int mvp;			/*	model view projection matrix.	*/
	int model;			/*	world space matrix.	*/
	int perspective;	/*	perspective matrix.	*/
	int view;			/*	camera space matrix.	*/
}UniformLocation;

typedef struct mesh_object_t{
	unsigned int vbo;
	unsigned int ibo;
	unsigned int vao;
	unsigned int indicescount;
	unsigned int verticescount;
	union{
		struct{
			hpmvec3f center;
			hpmvec3f size;
		};
		hpmvec3f aabb[2];
	};
}Mesh;






















typedef void (*pswapbufferfunctype)(ExWin window);	/*	Function pointer data type.	*/
typedef void (*presize_screen)(ExEvent* event, struct uniform_location_t* uniform, ExShader* shader, ExTexture* ftexture);
typedef void (*pupdate_shader_uniform)(struct uniform_location_t* uniform, ExShader* shader, int width, int height);
typedef void (*pupdate_update_uniforms)(UniformLocation* uniform, ExShader* shader, float ttime, long int deltatime);
typedef void (*pdisplaygraphic)(ExWin drawable);

/**/
extern presize_screen glslview_resize_screen;
extern pupdate_shader_uniform glslview_update_shader_uniform;
extern pdisplaygraphic glslview_displaygraphic;
extern pupdate_update_uniforms glslview_update_uniforms;
extern pswapbufferfunctype glslview_swapbuffer;					/*	Function pointer for swap default framebuffer.	*/


presize_screen glslview_resize_screen;
pupdate_shader_uniform glslview_update_shader_uniform;
pdisplaygraphic glslview_displaygraphic;
pupdate_update_uniforms glslview_update_uniforms;
pswapbufferfunctype glslview_swapbuffer;

extern const float quad[4][3];


void glslview_default_init(void);
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


void glslview_rendergraphic(ExWin drawable, ExShader* shader, UniformLocation* location, float ttime, float deltatime);






extern int privatefprintf(const char* format,...);
extern int debugprintf(const char* format,...);


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
/*	polygone	*/
extern unsigned int usepolygone;					/**/
extern Mesh mesh;									/**/




#endif
