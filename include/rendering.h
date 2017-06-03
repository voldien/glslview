#ifndef ___H_
#define ___H_ 1
#include"def.h"



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


#endif
