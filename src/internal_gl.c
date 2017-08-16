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

#include"rendering.h"
#include<stdio.h>
#include<unistd.h>
#include<GL/gl.h>
#include<GL/glext.h>
#include<SDL2/SDL.h>

SDL_Window* glslview_gl_init(void){
	SDL_Window* win;
	SDL_DisplayMode displaymode;
	int glatt;
	int x;

	/*	*/
	glslview_verbose_printf("Initialize OpenGL rendering interface.\n");

	/*	*/
	SDL_GetCurrentDisplayMode(0, &displaymode);
	displaymode.w /= 2;
	displaymode.h /= 2;
	win = SDL_CreateWindow("", displaymode.w/ 2, displaymode.h / 2, displaymode.w, displaymode.h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);



	if(win == NULL){
		return NULL;
	}

	/*	*/
	glc = SDL_GL_CreateContext(win);
	if(glc == NULL){
		fprintf(stderr, "Failed to create OpenGL context, %s.\n", SDL_GetError());
		assert(glc);
	}
	if(SDL_GL_MakeCurrent(win, glc) < 0){
		fprintf(stderr, "Failed to make current OpenGL context, %s.\n", SDL_GetError());
		return NULL;
	}

	/*	*/
	drawable = SDL_GL_GetCurrentWindow();
	if(drawable == NULL){
		fprintf(stderr, "Failed to current OpenGL Window, %s.\n", SDL_GetError());
		return NULL;
	}


	/*	Display OpenGL information.	*/
	glslview_verbose_printf("-------------- OpenGL Information ------------------\n");
	glslview_verbose_printf("OpenGL vendor string: %s.\n", glGetString(GL_VENDOR));
	glslview_verbose_printf("OpenGL version string: %s.\n", glGetString(GL_VERSION));
	glslview_verbose_printf("OpenGL shading language version string %s.\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	glslview_verbose_printf("OpenGL renderer string: %s.\n\n", glGetString(GL_RENDERER));


	/*	Set OpenGL default state.	*/
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);
	glDisable(GL_STENCIL_TEST);


	/*	Check if alpha is used.	*/
	SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &glatt);
	if(glatt > 0){
		/*	Enable alpha buffer bits.	*/
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glEnable(GL_ALPHA_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	else{
		/*	Disable alpha buffer bits.	*/
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
		glDisable(GL_ALPHA_TEST);
	}



	/*	Generate vertex array for quad.	*/
	glslview_verbose_printf("----------- constructing rendering quad. ----------\n");
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);

	/*	*/
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, (const void*)0);

	/*	*/
	glslview_gl_set_viewport(displaymode.w, displaymode.h);

	/*	*/
	if(glIsTexture(fbackbuffertex.texture) == GL_TRUE){
		glActiveTexture(GL_TEXTURE0 + numTextures);
		glBindTexture(fbackbuffertex.target, fbackbuffertex.texture);
	}

	/*	Bind all textures once.	*/
	for(x = 0; x < numTextures; x++){
		if(glIsTexture(textures[x].texture) == GL_TRUE){
			glslview_verbose_printf("Binding texture %d.\n", x);
			glActiveTexture(GL_TEXTURE0 + x);
			glBindTexture(textures[x].target, textures[x].texture);
		}
	}


	return win;
}

void glslview_gl_release(void){

	int x;

	glslview_verbose_printf("Releasing OpenGL resources.\n");

	/*	Release OpenGL resources.	*/
	if(g_glc != NULL){

		/*	Release each shader program.	*/
		for(x = 0; x < numShaderPass; x++){
			if(glIsProgram(g_shaders[x].shader.program) == GL_TRUE){
				glDeleteProgram(1, &g_shaders[x].shader.program);
			}
		}
		if(glIsVertexArray(vao) == GL_TRUE){
			glDeleteVertexArrays(1, &vao);
		}
		if(glIsBuffer(vbo) == GL_TRUE ){
			glDeleteBuffers(1, &vbo);
		}

		/*	Release each textures.	*/
		for(x = 0; x < numTextures; x++){
			if(glIsTexture(textures[x].texture) == GL_TRUE){
				glDeleteTextures(1, &textures[x].texture);
			}
		}

		if(glIsTexture(fbackbuffertex.texture) == GL_TRUE){
			glDeleteTextures(1, &fbackbuffertex.texture);
		}

		/*	Unbind glcontext and delete.	*/
		SDL_GL_MakeCurrent(g_window, NULL);
		SDL_GL_DeleteContext(g_glc);
	}
}


GLuint getTextureFormat(unsigned int format){

	/*	*/
	switch(format){
	case TEXTURE_RGB:
		return GL_RGB;
	case TEXTURE_RGBA:
		return GL_RGBA;
	case TEXTURE_COMPRESSION_RGB:
		return GL_COMPRESSED_RGB;
	case TEXTURE_COMPRESSION_RGBA:
		return GL_COMPRESSED_RGBA;
	case TEXTURE_BGR:
		return GL_BGR;
	case TEXTURE_BGRA:
		return GL_BGRA;
	default:
		return 0;
	}
}

glslviewTexture* glslview_gl_create_texture(glslviewTexture* texture,
		unsigned int target, int level, int internalFormat, int width,
		int height, int border, unsigned int format, unsigned int type,
		const void *pixels) {

	assert(texture);

	/*	*/
	internalFormat = getTextureFormat(internalFormat);
	format = getTextureFormat(format);

	/*	*/
	texture->target = target;
	texture->internalformat = internalFormat;
	texture->width = width;
	texture->height = height;
	texture->internalformat = format;
	texture->type = type;

	/*	*/
	glGenTextures(1, &texture->texture);
	glBindTexture(target, texture->texture);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	/*	*/
	switch(target){
	case GL_TEXTURE_2D:
		glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
		break;
	default:
		break;
	}

	/*	*/
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_R,GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 5);

	/*	*/
	glGenerateMipmap(target);

	/*	*/
	glIsTexture(texture->texture);

	return texture;
}


int glslview_compile_shader(const char** source, unsigned int num, unsigned int type){

	int shader;
	int status;
	int error;

	/*	*/
	assert(source || *source);

	/*	Compile shaders.	*/
	shader = glCreateShader(type);
	glShaderSource(shader, num, source, NULL);
	glCompileShader(shader);

	/*	Check for error.	*/
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status );
	if( status == GL_FALSE){
		char infolog[4096];
		glGetShaderInfoLog(shader, sizeof(infolog), NULL, &infolog[0]);
		fprintf(stderr, "%s\n", infolog);
	}

	return shader;
}

static unsigned int glslview_get_GLSL_version(void){

	unsigned int version;
	char glstring[128] = {0};
	char* wspac;

	/*	Extract version number.	*/
	strcpy(glstring, (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	wspac = strstr(glstring, " ");
	if(wspac){
		*wspac = '\0';
	}
	version = strtof(glstring, NULL) * 100;

	return version;
}

int glslview_gl_create_shader(glslviewShader* shader, const char* cvertexSource,
		const char* cfragmentSource, const char* cgeometry_source,
		const char* ctess_c_source, const char* ctess_e_source) {

	int error = 1;
	GLuint lstatus;
	char glversion[64];
	char** vsources[2] = {""};
	char** fsources[2] = {""};

	/**/
	sprintf(glversion, "#version %d\n", glslview_get_GLSL_version());

	/*	Assign version header unless explicity defined.	*/
	if(strstr(cvertexSource, "#version") == NULL)
		vsources[0] = &glversion[0];
	if(strstr(cfragmentSource, "#version") == NULL)
		fsources[0] = &glversion[0];


	vsources[1] = (char*)cvertexSource;
	fsources[1] = (char*)cfragmentSource;

	/**/
	shader->program = glCreateProgram();

	/*	*/
	if(cvertexSource){
		shader->ver = glslview_compile_shader(vsources, 2, GL_VERTEX_SHADER);
		glAttachShader(shader->program, shader->ver);
	}
	if(cfragmentSource){
		shader->fra = glslview_compile_shader(fsources, 2, GL_FRAGMENT_SHADER);
		glAttachShader(shader->program, shader->fra);
	}
	/*
#if defined(__gl_h_)
	if(cgeometrySource){
		shader->geo = ExCompileShaderSourcev(&cgeometrySource, GL_GEOMETRY_SHADER);
		glAttachShader(shader->program, shader->geo);
	}
	if(ctessCSource){
		shad->tesc = ExCompileShaderSourcev(&ctessCSource, GL_TESS_CONTROL_SHADER);
		glAttachShader(shad->program,shad->tesc);
	}
	if(ctessESource){
		shad->tese = ExCompileShaderSourcev(&ctessESource, GL_TESS_EVALUATION_SHADER);
		glAttachShader(shad->program,shad->tese);
	}
#endif
	*/

	/*	Link togheter shader program.	*/
	glLinkProgram(shader->program);

	/*	Check for error.	*/
	glGetProgramiv(shader->program, GL_LINK_STATUS, &lstatus);
	if(lstatus == GL_FALSE){
		char infolog[4096];
		glGetProgramInfoLog(shader->program, sizeof(infolog), NULL, &infolog[0] );
		fprintf(stderr, "%s\n", infolog);
	}


#if defined(__gl_h_)
	if(!error){
		glDeleteProgram(shader->program);
	}
#endif
	glValidateProgram(shader->program);

	/*	Setach shader objects and release their resources.	*/
	glDetachShader(shader->program, shader->ver);
	glDetachShader(shader->program, shader->fra);
	glDetachShader(shader->program, shader->geo);
	glDetachShader(shader->program, shader->tesc);
	glDetachShader(shader->program, shader->tese);
	glDeleteShader(shader->ver);
	glDeleteShader(shader->fra);
	glDeleteShader(shader->geo);
	glDeleteShader(shader->tesc);
	glDeleteShader(shader->tese);

	return error;
}


void glslview_gl_resize_screen(int* res, UniformLocation* uniform,
		glslviewShader* shader, glslviewTexture* ftexture) {

	float resolution[2] = {res[0], res[1]};
	glUniform2fv(uniform->resolution, 1, &resolution[0]);
	glslview_debug_printf("gl window resize, %dx%d.\n", res[0], res[1]);

	/**/
	if(ftexture){
		if(glIsTexture(fbackbuffertex.texture) == GL_TRUE){
			glDeleteTextures(1, &fbackbuffertex.texture);
			glslview_gl_create_texture(&fbackbuffertex, GL_TEXTURE_2D, 0,
					g_ftexinternalformat, res[0], res[1], 0, g_ftexformat,
					g_ftextype, NULL);
		}
	}
}


void glslview_gl_update_shader_uniform(struct uniform_location_t* uniform,
		glslviewShader* shader, int width, int height) {

	float res[2];

	glslview_verbose_printf("----------- fetching uniforms index location ----------\n");
	uniform->time = glGetUniformLocation(shader->program, "time");
	uniform->deltatime = glGetUniformLocation(shader->program, "deltatime");
	uniform->resolution = glGetUniformLocation(shader->program, "resolution");
	uniform->mouse = glGetUniformLocation(shader->program, "mouse");
	uniform->offset = glGetUniformLocation(shader->program, "offset");
	uniform->stdin = glGetUniformLocation(shader->program, "stdin");
	uniform->tex0 = glGetUniformLocation(shader->program, "tex0");
	uniform->tex1 = glGetUniformLocation(shader->program, "tex1");
	uniform->tex2 = glGetUniformLocation(shader->program, "tex2");
	uniform->tex3 = glGetUniformLocation(shader->program, "tex3");
	uniform->tex4 = glGetUniformLocation(shader->program, "tex4");
	uniform->tex5 = glGetUniformLocation(shader->program, "tex5");
	uniform->tex6 = glGetUniformLocation(shader->program, "tex6");
	uniform->tex7 = glGetUniformLocation(shader->program, "tex7");
	uniform->tex8 = glGetUniformLocation(shader->program, "tex8");
	uniform->tex9 = glGetUniformLocation(shader->program, "tex9");
	uniform->tex10 = glGetUniformLocation(shader->program, "tex10");
	uniform->tex11 = glGetUniformLocation(shader->program, "tex11");
	uniform->tex12 = glGetUniformLocation(shader->program, "tex12");
	uniform->tex13 = glGetUniformLocation(shader->program, "tex13");
	uniform->tex14 = glGetUniformLocation(shader->program, "tex14");
	uniform->tex15 = glGetUniformLocation(shader->program, "tex15");
	uniform->backbuffer = glGetUniformLocation(shader->program, "backbuffer");

	/*	Debug.	*/
	glslview_debug_printf("time %d\n", uniform->time);
	glslview_debug_printf("deltatime %d\n", uniform->deltatime);
	glslview_debug_printf("resolution %d\n", uniform->resolution);
	glslview_debug_printf("mouse %d\n", uniform->mouse);
	glslview_debug_printf("offset %d\n", uniform->offset);
	glslview_debug_printf("stdin %d\n", uniform->stdin);
	glslview_debug_printf("tex0 %d\n", uniform->tex0);
	glslview_debug_printf("tex1 %d\n", uniform->tex1);
	glslview_debug_printf("tex2 %d\n", uniform->tex2);
	glslview_debug_printf("tex3 %d\n", uniform->tex3);
	glslview_debug_printf("tex4 %d\n", uniform->tex4);
	glslview_debug_printf("tex5 %d\n", uniform->tex5);
	glslview_debug_printf("tex6 %d\n", uniform->tex6);
	glslview_debug_printf("tex7 %d\n", uniform->tex7);
	glslview_debug_printf("tex8 %d\n", uniform->tex8);
	glslview_debug_printf("tex9 %d\n", uniform->tex9);
	glslview_debug_printf("tex10 %d\n", uniform->tex10);
	glslview_debug_printf("tex11 %d\n", uniform->tex11);
	glslview_debug_printf("tex12 %d\n", uniform->tex12);
	glslview_debug_printf("tex13 %d\n", uniform->tex13);
	glslview_debug_printf("tex14 %d\n", uniform->tex14);
	glslview_debug_printf("tex15 %d\n", uniform->tex15);
	glslview_debug_printf("backbuffer %d\n", uniform->backbuffer);

	glUseProgram(shader->program);

	res[0] = width;
	res[1] = height;
	glUniform2fv(uniform->resolution, 1, &res[0]);

	/*	*/
	glslview_verbose_printf("----------- Assigning texture sampler index ----------\n");
	glUniform1i(uniform->tex0, 0);
	glUniform1i(uniform->tex1, 1);
	glUniform1i(uniform->tex2, 2);
	glUniform1i(uniform->tex3, 3);
	glUniform1i(uniform->tex4, 4);
	glUniform1i(uniform->tex5, 5);
	glUniform1i(uniform->tex6, 6);
	glUniform1i(uniform->tex7, 7);
	glUniform1i(uniform->tex8, 8);
	glUniform1i(uniform->tex9, 9);
	glUniform1i(uniform->tex10, 10);
	glUniform1i(uniform->tex11, 11);
	glUniform1i(uniform->tex12, 12);
	glUniform1i(uniform->tex13, 13);
	glUniform1i(uniform->tex14, 14);
	glUniform1i(uniform->tex15, 15);
	glUniform1i(uniform->backbuffer, numTextures);


	/*	Create backbuffer.	*/
	if(uniform->backbuffer != -1){
		if(glIsTexture(fbackbuffertex.texture) == GL_TRUE){
			glDeleteTextures(1, &fbackbuffertex.texture);
		}
		glslview_gl_create_texture(&fbackbuffertex, GL_TEXTURE_2D,  0, g_ftexinternalformat, width, height, 0, g_ftexformat, g_ftextype, NULL);
		glslview_verbose_printf("Created backbuffer.	\n");
	}else{
		if(glIsTexture(fbackbuffertex.texture) == GL_TRUE){
			glDeleteTextures(1, &fbackbuffertex.texture);
		}
	}
}

void glslview_gl_displaygraphic(SDL_Window* drawable){

	/*	draw quad.	*/
	glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(quad) / sizeof(quad[0]));

	/*	*/
	SDL_GL_SwapWindow(drawable);
}





void glslview_gl_update_uniforms(UniformLocation* uniform,
		glslviewShader* shader, float ttime, long int deltatime) {

	if(uniform->time != -1){
		glUniform1fv(uniform->time, 1, &ttime);
	}
	if(uniform->deltatime != -1){
		glUniform1f(uniform->deltatime, (float)((float)deltatime / (float)1E9));
	}

	/*	*/
	if(use_stdin_as_buffer){
		int buffer;
		if(read(STDIN_FILENO, (void*)&buffer, stdin_buffer_size) > 0){
			glUniform1iv(uniform->stdin, sizeof(GLint), (const GLint*)&buffer);
		}
	}

}

void glslview_gl_set_viewport(unsigned int width, unsigned int height){
	glViewport(0, 0, width, height);
}



void glslview_gl_rendergraphic(SDL_Window* drawable,
		glslviewShaderCollection* shaders, float ttime, float deltatime) {

	int x;

	for(x = 0; x < numShaderPass; x++){

		/**/
		glUseProgram(shaders[x].shader.program);

		/*	*/
		glslview_gl_update_uniforms(&shaders[x].uniform, &shaders[x].shader, ttime, deltatime);
		glslview_gl_displaygraphic(drawable);

		/*	*/
		if(shaders[x].uniform.backbuffer != -1){
			glActiveTexture(GL_TEXTURE0 + numTextures);
			glBindTexture(fbackbuffertex.target, fbackbuffertex.texture);
			glCopyTexImage2D(fbackbuffertex.target, 0, GL_RGBA, 0, 0, fbackbuffertex.width, fbackbuffertex.height, 0);
		}

	}

	glClear(GL_COLOR_BUFFER_BIT);

}

