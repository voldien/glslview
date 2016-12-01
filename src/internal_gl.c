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

#include"internal.h"
#include<stdio.h>
#include<unistd.h>
#include<GL/gl.h>
#include<GL/glext.h>
#include<SDL2/SDL.h>

SDL_Window* glslview_init_opengl(void){
	SDL_Window* win;
	SDL_DisplayMode displaymode;
	int glatt;

	SDL_GetCurrentDisplayMode(0, &displaymode);
	displaymode.w /= 2;
	displaymode.h /= 2;
	win = SDL_CreateWindow("", displaymode.w/ 2, displaymode.h / 2, displaymode.w, displaymode.h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if(win == NULL){
		return NULL;
	}

	glc = SDL_GL_CreateContext(win);
	SDL_GL_MakeCurrent(win, glc);
	assert(glc);


	/*	Display OpenGL information.	*/
	glslview_verbose_printf("-------------- OpenGL Information ------------------\n");
	glslview_verbose_printf("OpenGL vendor string: %s.\n", glGetString(GL_VENDOR));
	glslview_verbose_printf("OpenGL version string: %s.\n", glGetString(GL_VERSION));
	glslview_verbose_printf("OpenGL shading language version string %s.\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	glslview_verbose_printf("OpenGL renderer string: %s.\n\n", glGetString(GL_RENDERER));


	/*	*/
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glDisable(GL_STENCIL_TEST);

	/*	*/
	SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE, &glatt);
	if(glatt > 0){
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glEnable(GL_ALPHA_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	}
	else{
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

	/**/
	glslview_set_viewport_gl(displaymode.w, displaymode.h);

	return win;
}



glslviewTexture* glslview_create_texture_gl(glslviewTexture* texture, unsigned int target, int level, int internalFormat, int width, int height, int border, unsigned int format, unsigned int type, const void *pixels){

	if(!texture){
		return NULL;
	}

	texture->target = target;
	texture->internalformat = internalFormat;
	texture->width = width;
	texture->height = height;
	texture->internalformat = format;
	texture->type = type;


	glGenTextures(1, &texture->texture);
	glBindTexture(target, texture->texture);
	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	switch(target){
	case GL_TEXTURE_2D:
		glTexImage2D(target, level, internalFormat, width, height, border, format, type, pixels);
		break;
	default:
		break;
	}


	glTexParameteri(target, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_R,GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 5);

	glGenerateMipmap(target);


	return texture;
}


int compile_shader(const char** source, unsigned int type){
	int shader;
	int status;
	int error;
	if(!source || !source[0]){
		return -1;
	}
	if(!strlen(source[0])){
		return -1;
	}

	shader = glCreateShader(type);
	glShaderSource(shader, 1, source, NULL);
	glCompileShader(shader);

	//error = ExShaderCompileLog(shader, type);

	return shader;
}

int glslview_create_shader_gl(glslviewShader* shader, const char* cvertexSource, const char* cfragmentSource, const char* cgeometry_source, const char* ctess_c_source, const char* ctess_e_source){
	int error = 1;

	/**/
	shader->program = glCreateProgram();

	if(cvertexSource){
		shader->ver = compile_shader(&cvertexSource, GL_VERTEX_SHADER);
		glAttachShader(shader->program, shader->ver);
	}
	if(cfragmentSource){
		shader->fra = compile_shader(&cfragmentSource, GL_FRAGMENT_SHADER);
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

	/*	*/
	glLinkProgram(shader->program);

#if defined(__gl_h_)
	//error = ExshadererCompileLog(shader->program, GL_PROGRAM);
	/*	if shaderer failed. clean up resources.	*/
	if(!error){
		glDeleteProgram(shader->program);
	}

#endif
	glValidateProgram(shader->program);

	/*	detach shaderer object and release their resources.	*/
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


void glslview_resize_screen_gl(int* res, UniformLocation* uniform, glslviewShader* shader, glslviewTexture* ftexture){

	float resolution[2] = {res[0], res[1]};
	glUniform2fv(uniform->resolution, 1, &resolution[0]);
	glslview_verbose_printf("%dx%d.\n", res[0], res[1]);
	/**/


	if(ftexture){
		if(glIsTexture(fbackbuffertex.texture) == GL_TRUE){
			glDeleteTextures(1, &fbackbuffertex.texture);
			glslview_create_texture(&fbackbuffertex, GL_TEXTURE_2D,  0, ftexinternalformat, res[0], res[1], 0, ftexformat, ftextype, NULL);
		}
	}
}


void glslview_update_shader_uniform_gl(struct uniform_location_t* uniform, glslviewShader* shader, int width, int height){
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
	uniform->mvp = glGetUniformLocation(shader->program, "mvp");
	uniform->model = glGetUniformLocation(shader->program, "model");
	uniform->perspective = glGetUniformLocation(shader->program, "pers");
	uniform->view = glGetUniformLocation(shader->program, "view");


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
	glslview_debug_printf("mvp %d\n", uniform->mvp);
	glslview_debug_printf("model%d\n", uniform->model);
	glslview_debug_printf("perspective %d\n", uniform->perspective);
	glslview_debug_printf("view %d\n", uniform->view);


	glUseProgram(shader->program);

	res[0] = width;
	res[1] = height;
	glUniform2fv(uniform->resolution, 1, &res[0]);

	/**/
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
		glslview_create_texture(&fbackbuffertex, GL_TEXTURE_2D,  0, ftexinternalformat, width, height, 0, ftexformat, ftextype, NULL);
		glslview_verbose_printf("Created backbuffer.	\n");
	}else{
		if(glIsTexture(fbackbuffertex.texture) == GL_TRUE){
			glDeleteTextures(1, &fbackbuffertex.texture);
		}
	}
}

void glslview_displaygraphic_gl(SDL_Window* drawable){
	/*	draw quad.	*/
	glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(quad) / sizeof(quad[0]));

	if(usepolygone){
		glBindVertexArray(mesh.vao);
		glDrawElements(GL_TRIANGLES, mesh.indicescount, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);
	}
	else{
		/*	draw quad.	*/
		glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(quad) / sizeof(quad[0]));
	}
	/**/
	glslview_swapbuffer(drawable);
}





void glslview_update_uniforms_gl(UniformLocation* uniform, glslviewShader* shader, float ttime, long int deltatime){

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

void glslview_set_viewport_gl(unsigned int width, unsigned int height){
	glViewport(0, 0, width, height);
}




void glslview_rendergraphic_gl(SDL_Window* drawable, glslviewShaderCollection* shaders, float ttime, float deltatime){
	int x;

	for(x = 0; x < numShaderPass; x++){

		glUseProgram(shaders[x].shader.program);


		if(shaders[x].uniform.mvp != -1){
			glUniformMatrix4fv(shaders[x].uniform.mvp, 1, GL_FALSE, mvp);
		}
		if(shaders[x].uniform.model != -1){
			glUniformMatrix4fv(shaders[x].uniform.model, 1, GL_FALSE, model);
		}


		glslview_update_uniforms(&shaders[x].uniform, &shaders[x].shader, ttime, deltatime);
		glslview_displaygraphic(drawable);

		if(shaders[x].uniform.backbuffer != -1){
			glActiveTexture(GL_TEXTURE0 + numTextures);
			glBindTexture(fbackbuffertex.target, fbackbuffertex.texture);
			glCopyTexImage2D(fbackbuffertex.target, 0, GL_RGBA, 0, 0, fbackbuffertex.width, fbackbuffertex.height, 0);
		}
	}
	glClear(usepolygone > 0 ? GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT : GL_COLOR_BUFFER_BIT);
	//glClear(GL_COLOR_BUFFER_BIT |  ( ( (usepolygone & 0x1) << 32 ) & GL_DEPTH_BUFFER_BIT) );
}

