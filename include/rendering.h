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
#ifndef _GLSLVIEW_RENDERING_H_
#define _GLSLVIEW_RENDERING_H_ 1
#include"glslview.h"
#include<SDL2/SDL.h>


/**
 *	Initialize	rendering API and create window
 *	associate with the rendering API.
 *
 *	@Return None null pointer if successfully.
 */
extern SDL_Window* glslview_gl_init(void);

/**
 *	Release rendering API resource.
 */
extern void glslview_gl_release(void);

/**
 *	Create texture. Uses similar function paramter call as
 *	glTexImage2D.
 *
 *	@Return
 */
extern glslviewTexture* glslview_gl_create_texture(glslviewTexture* texture, unsigned int target, int level,
		int internalFormat, int width, int height, int border, unsigned int format, unsigned int type,
		const void *pixels);

/**
 *	Create shader.
 *
 *	@Return status.
 */
extern int glslview_gl_create_shader(glslviewShader* texture, const char* cvertexSource,
		const char* cfragmentSource, const char* cgeometry_source, const char* ctess_c_source,
		const char* ctess_e_source);

/**
 *	Resize screen size.
 */
extern void glslview_gl_resize_screen(int* __restrict__ res,
		UniformLocation* __restrict__ uniform,
		glslviewShader* __restrict__ shader,
		glslviewTexture* __restrict__ ftexture);

/**
 *	Update shader uniforms.
 */
extern void glslview_gl_update_shader_uniform(
		UniformLocation* __restrict__ uniform,
		glslviewShader* __restrict__ shader, int width, int height);

/**
 *
 */
extern void glslview_gl_update_uniforms(UniformLocation* uniform,
		glslviewShader* shader, float ttime, long int deltatime);

/**
 *
 */
extern void glslview_gl_displaygraphic(SDL_Window* drawable);

/**
 *	Set viewport.
 */
extern void glslview_gl_set_viewport(unsigned int width, unsigned int height);

/**
 *
 */
extern void glslview_gl_rendergraphic(SDL_Window* drawable,
		glslviewShaderCollection* shader, float ttime, float deltatime);


#endif
