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
#include"def.h"
#include"glslview_log.h"
#include<SDL2/SDL.h>
#include<assert.h>


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

/**
 *
 */
typedef struct glslview_texture_t{
	unsigned int target;			/*	Texture target.	*/
	unsigned int texture;			/*	Texture unique identifier.	*/
	unsigned int width;				/*	Texture width in pixels.	*/
	unsigned int height;			/*	Texture height in pixels.	*/
	unsigned int internalformat;	/*	Internal format enumerator.	*/
	unsigned int type;				/*	Internal storage type enumerator.	*/
}glslviewTexture;

/**
 *
 */
typedef struct glslview_shader_t{
	unsigned int ver;		/*	vertex shader.	*/
	unsigned int fra;		/*	fragment shader.	*/
	unsigned int geo;		/*	geometry shader.	*/
	unsigned int tesc;		/*	tessellation control shader.	*/
	unsigned int tese;		/*	tessellation evaluation shader.	*/
	unsigned int program;	/*	shader program.	*/
	unsigned int flag;		/*	Shader flag.	*/
}glslviewShader;

/**
 *
 */
typedef struct glslview_shader_collection_t{
	UniformLocation uniform;	/*	Uniform of shader.	*/
	glslviewShader shader;		/*	Shader.	*/
}glslviewShaderCollection;

/**
 *
 */
typedef struct glslview_texture_collection_t{
	glslviewTexture texture;	/*	Texture.	*/
}glslviewTextureCollection;



















/**
 *	Initialize glslsview program.
 *
 *	@Return
 *
 */
extern void glslview_terminate(void);

/**
 *
 */
extern int glslview_display(void);


/**
 *	Signal catch function.
 */
extern void glslview_catchSig(int signal);




/**
 *
 */
extern int needsUpdate(glslviewShaderCollection* shader);

/**
 *	Load file from filepath.
 *
 *	@Return if sucesfully the number of bytes loaded, -1 otherwise.
 */
extern long int glslview_loadfile(const char* cfilename, void** bufferptr);

/**
 *	Load string from filepath.
 *
 *	@Return if sucesfully the number of bytes loaded, -1 otherwise.
 */
extern long int glslview_loadString(const char* cfilename, void** bufferptr);


#endif
