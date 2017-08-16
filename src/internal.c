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
#include"glslview.h"
#include"rendering.h"
#include<SDL2/SDL.h>
#include<GL/gl.h>
#include<FreeImage.h>
/*	TODO remove glsl function later.	*/
#include<getopt.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<signal.h>
#include<sys/inotify.h>	/*	TODO fix such that it uses a portable solution.	*/

/*	Default vertex shader.	*/
const char* vertex = ""
//"#version 330 core\n"
"layout(location = 0) in vec3 vertex;\n"
"void main(void){\n"
"gl_Position = vec4(vertex,1.0);\n"
"}\n";


/*	Display quad.	*/
const float quad[4][3] = {
		{-1.0f, -1.0f, 0.0f},
		{-1.0f, 1.0f, 0.0f},
		{ 1.0f, -1.0f, 0.0f},
		{ 1.0f,  1.0f, 0.0f},
};

/*	quad buffer.	*/
unsigned int vao = 0;                           /*	*/
unsigned int vbo = 0;                           /*	*/

/**/
SDL_GLContext g_glc;                            /*	*/
SDL_Window* g_window = NULL;                    /*	Window.	*/
SDL_Window* drawable = NULL;                    /*	Window.	*/
unsigned int renderingapi = 0;                  /*	rendering api.	*/
int g_fullscreen = 0;                           /*	Set window fullscreen.	*/
int g_verbose = GLSLVIEW_QUITE;                 /*	enable verbose.	*/
int g_debug = 0;                                /*	enable debugging.	*/
int g_compression = 0;                          /*	Use compression.	*/
unsigned int g_isAlive = 1;                     /*	*/
int ifd = -1;                                   /*	inotify file descriptor.*/
int wd = -1;                                    /*	inotify watch directory.	*/
char* inotifybuf = NULL;                        /*	*/
unsigned int numFragPaths = 0;                  /*	*/
unsigned int numShaderPass = 0;                 /*	*/
char* fragPath[32] = {NULL};                    /*	Path of fragment shader.	*/

/*	Rendering global variable.	*/
glslviewShaderCollection* g_shaders = NULL;     /*	*/
unsigned int g_fbo = 0;                         /*	*/
unsigned int g_ftextype = GL_FLOAT;             /*	*/
unsigned int g_ftexinternalformat = GL_RGBA;    /*	*/
unsigned int g_ftexformat = GL_RGBA;            /*	*/
glslviewTexture fbackbuffertex = {0};           /*	framebuffer texture for backbuffer uniform variable.	*/
glslviewTexture textures[8] = {{0}};            /*	*/
const int numTextures = sizeof(textures) / sizeof(textures[0]);
unsigned int nextTex = 0;                       /*	*/
unsigned int g_isPipe;                          /*	*/
unsigned int use_stdin_as_buffer = 0;           /*	*/
int stdin_buffer_size = 1;                      /*	*/


int needsUpdate(glslviewShaderCollection* shader){
	int i;
	int needupdate = 0;

	for(i = 0; i < numShaderPass; i++){
		if(shader[i].uniform.time > 0 || shader[i].uniform.deltatime > 0){
			needupdate = 1;
			break;
		}
	}

	return needupdate;
}

void glslview_catchSig(int signal){

	SDL_Event event = { 0 };

	switch(signal){
	case SIGINT:
	case SIGQUIT:
		g_isAlive = SDL_FALSE;
		event.type = SDL_QUIT;
		SDL_PushEvent(&event);
		break;
	case SIGTERM:
	case SIGABRT:
		exit(EXIT_FAILURE);
		break;
	case SIGPIPE:
		if(use_stdin_as_buffer){
			exit(EXIT_FAILURE);
		}
		break;
	case SIGILL:
		exit(EXIT_FAILURE);
		break;
	}
}

#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

int glslview_display(void){

	SDL_Event event = {0};                  /*	*/
	float elapse;                           /*	Time elapse since start in seconds.	*/
	SDL_Point size;                         /*	*/
	char* fragData = NULL;                  /*	*/
	int x;                                  /*	Iterator.	*/
	float mouse[2];                         /*	*/

	/**/
	long int private_start;                 /*	Timestamp start.	*/
	long int pretime;                       /*	Previous timestamp.	*/
	long int deltatime;                     /*	Delta timestamp.	*/

	volatile int visable = 1;               /*	View visibility.	*/
	volatile int renderInBackground = 0;    /*	whether being rendered in the background or not.	*/
	unsigned int needsFrameUpdate = 0;      /*	If time is enabled.	*/
	unsigned int needmouseupdate = 0;       /*	If mouse input is enabled.	*/
	int eventtimeout = INT32_MAX;           /*	*/

	/*	*/
	struct timeval timeval = { 0, 1000 };   /*	Timeout for the inotify.	*/

	/*	*/
	private_start = SDL_GetPerformanceCounter();
	pretime = SDL_GetPerformanceCounter();

	/*	*/
	if(ifd < 0 ){
		timeval.tv_sec = 0;
		timeval.tv_usec = 0;
	}

	/*	TODO improve later, because mouse input and other has to be taking into consideration.*/
	if(needsUpdate(g_shaders) ){
		eventtimeout = 0;
	}

	/*	*/
	while(g_isAlive){

		/*	*/
		while(SDL_WaitEventTimeout(&event, eventtimeout)){

			switch(event.type){
			case SDL_QUIT:
				g_isAlive = SDL_FALSE;
				eventtimeout = 0;
			break;
			case SDL_KEYDOWN:

				glslview_debug_printf("%d\n", event.key.keysym.sym);
				if(event.key.keysym.sym == SDLK_RETURN && (event.key.keysym.mod & KMOD_CTRL )){
					g_fullscreen = ~g_fullscreen & SDL_TRUE;
					SDL_SetWindowFullscreen( g_window, g_fullscreen == SDL_TRUE ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
				}
			break;
			case SDL_MOUSEMOTION:
				for(x = 0; x < numShaderPass; x++){
					glUniform2fv(g_shaders[x].uniform.mouse, 1, &mouse[0]);
				}
			break;
			case SDL_MOUSEWHEEL:
			break;
			case SDL_WINDOWEVENT:
				switch(event.window.event){
				case SDL_WINDOWEVENT_RESIZED:
					glslview_gl_set_viewport(event.window.data1, event.window.data2);
					for(x = 0; x < numShaderPass; x++){
						glslview_gl_resize_screen(&event.window.data1, &g_shaders[x].uniform, &g_shaders[x], &fbackbuffertex);
					}
					glslview_gl_rendergraphic(drawable, g_shaders, elapse, deltatime);
					break;
				case SDL_WINDOWEVENT_MOVED:
					for(x = 0; x < numShaderPass; x++){
						if(g_shaders[x].uniform.offset != -1){
							glUniform2i(g_shaders[x].uniform.offset, 0, 0);
						}
					}
					break;
				case SDL_WINDOWEVENT_SHOWN:
				case SDL_WINDOWEVENT_EXPOSED:
					visable = SDL_TRUE;
					SDL_GetWindowSize(g_window, &size.x, &size.y);
					glslview_gl_set_viewport(size.x, size.y);
					for(x = 0; x < numShaderPass; x++){
						glslview_gl_resize_screen(&size.x, &g_shaders[x].uniform, &g_shaders[x], &fbackbuffertex);
					}
					glslview_gl_rendergraphic(drawable, g_shaders, elapse, deltatime);
					break;
				case SDL_WINDOWEVENT_MINIMIZED:
				case SDL_WINDOWEVENT_HIDDEN:
					visable = SDL_FALSE;
					break;
				default:
					break;
				}
				break;
				case SDL_LASTEVENT:
					break;
				default:
					break;
			}
		}


		/*	time.	*/
		elapse = (float)(( SDL_GetPerformanceCounter() - private_start) / 1E9F);
		deltatime = SDL_GetPerformanceCounter() - pretime;
		pretime = SDL_GetPerformanceCounter();

		/*	TODO fix such that its not needed to redefine some code twice for the rendering code section.	*/
		if(ifd != -1){
			fd_set readfd;
			int ret;
			FD_ZERO(&readfd);
			FD_SET(ifd, &readfd);
			ret = select(FD_SETSIZE, &readfd, NULL, NULL, &timeval);

			if(ret < 0){
				glslview_verbose_printf("Select failed: %s\n", strerror(errno));
			}
			else if(ret == 0){
				if(visable || renderInBackground){
					glslview_gl_rendergraphic(drawable, g_shaders, elapse, deltatime);
				}
			}else{
				struct inotify_event ionevent;

				char buffer[EVENT_BUF_LEN];
				/**/
				int nbytes;
				/**/
				while( (nbytes = read(ifd, &ionevent, EVENT_BUF_LEN)) > 0){
					char tmppath[4096];
					glslview_verbose_printf("inotify event fetching.\n");
					read(ifd, &buffer, ionevent.len);

					printf("%s\n",ionevent.name);
					if(ionevent.mask & IN_MODIFY){

						for(x = 0; x < numFragPaths; x++){
							char* ptmp;
							memcpy(tmppath, fragPath[x], strlen(fragPath[x]) + 1);
							ptmp = basename(tmppath);
							printf(tmppath);
							if(strcmp(ionevent.name, ptmp ) == 0){
								glslview_verbose_printf("Updating %s\n", fragPath[x]);

								glDeleteProgram(g_shaders[x].shader.program);
								memset(&g_shaders[x].shader, 0, sizeof(glslviewShader));

								glslview_loadfile((const char*)fragPath[x], (void**)&fragData);
								if(glslview_gl_create_shader(&g_shaders[x].shader, vertex, (const char*)fragData, NULL, NULL, NULL)){
									/**/
								}

								free(fragData);

								SDL_GetWindowSize(g_window, &size.x, &size.y);
								glUseProgram(g_shaders[x].shader.program);
								glslview_gl_update_shader_uniform(&g_shaders[x].uniform, &g_shaders[x].uniform, size.x, size.y);
								break;
							}
						}


					}
					if(ionevent.mask & IN_DELETE){
						glslview_verbose_printf("File deleted.\n");
					}
				}

			}
		}/*	if fd != -1*/
		else{

			if(visable || renderInBackground){
				glslview_gl_rendergraphic(drawable, g_shaders, elapse, deltatime);
			}else{/*	render passes	*/
				sleep(1);
			}

		}/*	render condition.	*/

	}/*	End of main while loop.	*/

}
