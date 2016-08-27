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

/**
 *
 */
typedef struct uniform_location_t{
	unsigned int time;			/*	time in seconds as float.	*/
	unsigned int resolution;	/*	resolution. */
	unsigned int deltatime;		/*	delta time.	*/
	unsigned int mouse;			/*	mouse.	*/
	unsigned int offset;		/*	offset.	*/
	unsigned int backbuffer;	/*	previous buffer.	*/
	unsigned int stdin;			/*	stdin data.	*/
	unsigned int tex0;			/*	texture 0.	*/
	unsigned int tex1;			/*	texture 1.	*/
	unsigned int tex2;			/*	texture 2.	*/
	unsigned int tex3;			/*	texture 3.	*/
	unsigned int tex4;			/*	texture 4.	*/
	unsigned int tex5;			/*	texture 5.	*/
	unsigned int tex6;			/*	texture 6.	*/
	unsigned int tex7;			/*	texture 7.	*/

}UniformLocation;

/**
 *
 */
void resize_screen_gl(ExEvent* event, struct uniform_location_t* uniform, ExShader* shader, ExTexture* ftexture);
void resize_screen_vk(ExEvent* event, struct uniform_location_t* uniform, ExShader* shader, ExTexture* ftexture);

/**
 *
 */
void update_shader_uniform_gl(struct uniform_location_t* uniform, ExShader* shader, int width, int height);
void update_shader_uniform_vk(struct uniform_location_t* uniform, ExShader* shader, int width, int height);


#endif
