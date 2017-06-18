/**
	glslview
    Copyright (C) 2017  Valdemar Lindberg

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
#ifndef _GLSL_VIEW_H_
#define _GLSL_VIEW_H_ 1
#include"def.h"
#include"internal.h"

typedef struct glslview_process_unit_t{

}GlslviewProcessUnit;

/*	Read only.	*/
extern const float quad[4][3];						/*	Display quad.	*/
extern const char* vertex;							/*	Display vertex shader.	*/

extern unsigned int vao;							/*	Display vertex array object.	*/
extern unsigned int vbo;							/*	Display vertex buffer object.	*/

/*	*/
extern SDL_GLContext glc;							/*	OpenGL Context.	*/
extern SDL_Window* window;							/*	Window.	*/
extern SDL_Window* drawable;
extern int fullscreen;								/*	Set window fullscreen.	*/
extern int g_verbose;									/*	enable verbose.	*/
extern int g_debug;									/*	enable debugging.	*/
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


/**
 *	Get glslview version.
 *
 *	@Return non NULL string.
 */
extern const char* glslview_getVersion(void);

/**
 *	Initialize glslsview program.
 *
 *	@Return
 *
 */
extern int glslview_init(int argc, const char** argv);

/**
 *	Read user input argument.
 *
 *	@Return status.
 */
extern int glslview_readargument(int argc, const char** argv, int pass);



#endif
