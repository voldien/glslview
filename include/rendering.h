#ifndef ___H_
#define ___H_ 1
#include"def.h"



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
 *	TODO add enum for texture target.
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
 *
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
