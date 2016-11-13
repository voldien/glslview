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
#include<SDL2/SDL.h>

/*	TODO remove glsl function later.	*/
#include<GL/gl.h>
#include<GL/glext.h>

#include<regex.h>

#include<getopt.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<signal.h>

#include<sys/inotify.h>	/*	TODO fix such that it uses a portable solution.	*/


#include<FreeImage.h>




#ifndef GLSLVIEW_MAJOR_VERSION
	#define GLSLVIEW_MAJOR_VERSION	0
#endif	/*	GLSLVIEW_MAJOR_VERSION	*/
#ifndef GLSLVIEW_MINOR_VERSION
	#define GLSLVIEW_MINOR_VERSION	5
#endif	/*	GLSLVIEW_MINOR_VERSION	*/
#ifndef GLSLVIEW_REVISION_VERSION
	#define GLSLVIEW_REVISION_VERSION 0
#endif	/*	GLSLVIEW_REVISION_VERSION	*/




void glslview_default_init(void){
	glslview_resize_screen = glslview_resize_screen_gl;
	glslview_displaygraphic = glslview_displaygraphic_gl;
	glslview_update_shader_uniform = glslview_update_shader_uniform_gl;
	glslview_update_uniforms = glslview_update_uniforms_gl;
	glslview_set_viewport = glslview_set_viewport_gl;
	glslview_swapbuffer = SDL_GL_SwapWindow;
}



int glslview_init(int argc, const char** argv){
	int status = EXIT_SUCCESS;					/*	*/
	long int srclen;							/*	*/

	SDL_DisplayMode displaymode;
	char title[512];							/*	*/
	int glatt;
	char* fragData = NULL;						/*	*/
	int x;										/*	*/


	/*	Init values that has to been set in order for the software to work.	*/
	glslview_default_init();


	/*	*/
	if(glslview_readargument(argc, argv, 0) == 2){
		return EXIT_SUCCESS;
	}
	numShaderPass = numFragPaths;

	/**/
	printf("\n");
	printf("glslview v%d.%d.%d\n", GLSLVIEW_MAJOR_VERSION, GLSLVIEW_MINOR_VERSION, GLSLVIEW_REVISION_VERSION);
	printf("==================\n\n");

	/*	Initialize SDL.	*/
	//privatefprintf("ELT version %s\n", ExGetVersion());
	if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0){
		status = EXIT_FAILURE;
		return status;
	}

	/*	*/
	signal(SIGILL, glslview_catchSig);
	signal(SIGINT, glslview_catchSig);
	signal(SIGQUIT, glslview_catchSig);
	signal(SIGABRT, glslview_catchSig);
	signal(SIGPIPE, glslview_catchSig);



	/*	Create window. */
	SDL_GetCurrentDisplayMode(0, &displaymode);
	displaymode.w /= 2;
	displaymode.h /= 2;
	sprintf(title, "glslview %s", glslview_getVersion());
	window = SDL_CreateWindow(title, displaymode.w/ 2, displaymode.h / 2, displaymode.w, displaymode.h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if(!window){
		status = EXIT_FAILURE;
		return status;
	}
	glc = SDL_GL_CreateContext(window);
	SDL_GL_MakeCurrent(window, glc);

	/*	*/
	SDL_ShowWindow(window);
	SDL_SetWindowTitle(window, title);
	SDL_SetWindowPosition(window, displaymode.w / 2, displaymode.h / 2);
	SDL_SetWindowSize(window, displaymode.w, displaymode.h );


	/*	Display OpenGL information.	*/
	privatefprintf("-------------- OpenGL Information ------------------\n");
	privatefprintf("OpenGL vendor string: %s.\n", glGetString(GL_VENDOR));
	privatefprintf("OpenGL version string: %s.\n", glGetString(GL_VERSION));
	privatefprintf("OpenGL shading language version string %s.\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	privatefprintf("OpenGL renderer string: %s.\n\n", glGetString(GL_RENDERER));

	/*	*/
	if(glslview_readargument(argc, argv, 1) == 2){
		status = EXIT_FAILURE;
		return status;
	}


	/*	Load shader fragment source code.	*/
	privatefprintf("----------- fetching source code ----------\n");
	shaders = malloc(sizeof(glslviewShaderCollection) * numFragPaths);
	memset(shaders, 0, sizeof(glslviewShaderCollection) * numFragPaths);
	/**/
	if(isPipe && !use_stdin_as_buffer){
		char buf[4096];
		unsigned int len;
		unsigned int totallen = 0;
		while( ( len = read(STDIN_FILENO, buf, sizeof(buf) ) ) > 0 ){
			fragData = realloc(fragData, totallen + len);
			memcpy(fragData + totallen, buf, len);
			totallen += len;
		}

	}
	else{
		for(x = 0; x < numFragPaths; x++){
			const char* cvertex;
			if(usepolygone)
				cvertex = vertexpolygone;
			else
				cvertex = vertex;
			srclen = glslview_loadfile((const char*)fragPath[x], (void**)&fragData);
			debugprintf("Loaded shader file %s, with size of %d bytes.\n", fragPath[x], srclen);

			/*	compile shader.	*/
			privatefprintf("----------- compiling source code ----------\n");
			if(glslview_create_shader(&shaders[x].shader, cvertex, fragData, NULL, NULL, NULL) == 0){
				fprintf(stderr, "Invalid shader.\n");
				status = EXIT_FAILURE;
				return status;
			}else{
				glslview_update_shader_uniform(&shaders[x].uniform, &shaders[x].shader, displaymode.w, displaymode.h);
			}

			if( srclen < 0 ){
				status = EXIT_FAILURE;
				return status;
			}

			/*	free fragment source.	*/
			free(fragData);
			fragData = NULL;
		}
	}









	/*	generate vertex array for quad.	*/
	privatefprintf("----------- constructing rendering quad. ----------\n");
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);

	/*	*/
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, (const void*)0);

	/*	*/
	glBindVertexArray(0);

	/*	*/
	if(usepolygone){
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
		glDisable(GL_STENCIL_TEST);
	}else{
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glDisable(GL_STENCIL_TEST);
	}

	glViewport(0, 0, displaymode.w, displaymode.h);

	/*	*/
	glBindVertexArray(vao);



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

	return status;
}


const char* glslview_getVersion(void){
	return COMPILED_VERSION(GLSLVIEW_MAJOR_VERSION, GLSLVIEW_MINOR_VERSION, GLSLVIEW_REVISION_VERSION);
}




long int glslview_loadfile(const char* cfilename, void** bufferptr){
	FILE*f;
	void* buffer;
	long length;
	long int pos;
	*bufferptr = NULL;

	if(!cfilename || strlen(cfilename) == 0){
		return -1;
	}

	f = fopen(cfilename, "rb");
	if(!f){
		return -1;
	}

	/**/
    pos = ftell(f);
    fseek(f, 0,SEEK_END);
    length = ftell(f);
    fseek(f, pos, SEEK_SET);

    /**/
	buffer = (char*)malloc(length + 1);
	((char*)buffer)[length] = 0;
	length = fread(buffer, 1, length,f);
	fclose(f);
	*bufferptr = buffer;
	return length;
}


/**/
int privatefprintf(const char* format,...){
	va_list larg;
	int status;

	if(verbose == 0){
		return 0;
	}

	va_start(larg, format);
	status = vprintf(format, larg);
	va_end(larg);

	return status;
}

int debugprintf(const char* format,...){
	va_list larg;
	int status;

	if(debug == 0){
		return 0;
	}

	va_start(larg, format);
	status = vprintf(format, larg);
	va_end(larg);


	return status;
}
