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

#include <errno.h>
#include <FreeImage.h>
#include <getopt.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <internal.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>	/*	TODO fix such that it uses a portable solution.	*/
#include <sys/select.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <unistd.h>


/*	for version 2.0, 3D objects with hpm for high performance matrices operators.
 *	and assimp for extracting geometrices;
 */
//#include<hpm/hpm.h>
//#include<assimp/cimport.h>




#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

/*	Default vertex shader.	*/
const char* vertex = ""
"#version 330 core\n"
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
unsigned int vao = 0;						/*	*/
unsigned int vbo = 0;						/*	*/

/**/
SDL_GLContext glc;
SDL_Window* window = NULL;						/*	Window.	*/
SDL_Window* drawable = NULL;					/*	Window.	*/
unsigned int renderingapi = 0;					/*	rendering api.	*/
int fullscreen = 0;								/*	Set window fullscreen.	*/
int verbose = 0;								/*	enable verbose.	*/
int debug = 0;									/*	enable debugging.	*/
int compression = 0;							/*	Use compression.	*/
unsigned int isAlive = 1;						/*	*/
int ifd = -1;									/*	inotify file descriptor.*/
int wd = -1;									/*	inotify watch directory.	*/
char* inotifybuf = NULL;						/*	*/
unsigned int numFragPaths = 0;					/*	*/
unsigned int numShaderPass = 0;
char* fragPath[32] = {NULL};					/*	Path of fragment shader.	*/
/**/
glslviewShaderCollection* shaders = NULL;
unsigned int fbo = 0;							/*	*/
unsigned int ftextype = GL_FLOAT;
unsigned int ftexinternalformat = GL_RGBA;
unsigned int ftexformat = GL_RGBA;
glslviewTexture fbackbuffertex = {0};					/*	framebuffer texture for backbuffer uniform variable.	*/
glslviewTexture textures[8] = {{0}};					/*	*/
const int numTextures = sizeof(textures) / sizeof(textures[0]);
unsigned int nextTex = 0;						/*	*/
unsigned int isPipe;							/*	*/
unsigned int use_stdin_as_buffer = 0;			/*	*/
int stdin_buffer_size = 1;						/*	*/


/*	function pointers.	*/
pglslview_init_renderingapi glslview_init_renderingapi = NULL;
pglslview_release_vulkan glslview_release_renderingapi = NULL;
presize_screen glslview_resize_screen = NULL;
pupdate_shader_uniform glslview_update_shader_uniform = NULL;
pdisplaygraphic glslview_displaygraphic = NULL;
pupdate_update_uniforms glslview_update_uniforms = NULL;
pset_viewport glslview_set_viewport = NULL;
pswapbufferfunctype glslview_swapbuffer	= NULL;					/*	Function pointer for swap default framebuffer.	*/
pglslview_create_texture glslview_create_texture = NULL;
pglslview_create_shader glslview_create_shader = NULL;
pglslview_rendergraphic glslview_rendergraphic = NULL;



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
		isAlive = SDL_FALSE;
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



void glslview_terminate(void){

	if(window != NULL){
		SDL_DestroyWindow(window);
	}

	/*	*/
	if(ifd != -1){
		inotify_rm_watch(ifd, wd);
		free(inotifybuf);
		close(ifd);
	}

	SDL_Quit();
}




int main(int argc, const char** argv){
	int status = EXIT_SUCCESS;				/*	Exit status.	*/
	SDL_Event event = {0};					/*	*/
	float elapse;							/*	Time elapse since start in seconds.	*/
	SDL_Point size;							/*	*/
	char* fragData = NULL;					/*	*/
	int x;									/*	iterator.	*/
	float mouse[2];							/*	*/


	/**/
	long int private_start;					/*	Timestamp start.	*/
	long int pretime;						/*	Previous timestamp.	*/
	long int deltatime;						/*	Delta timestamp.	*/
	int visable = 1;						/*	View visibility.	*/
	int renderInBackground = 0;				/*	whether being rendered in the background or not.	*/
	unsigned int needsFrameUpdate = 0;		/*	If time is enabled.	*/
	unsigned int needmouseupdate = 0;		/*	If mouse input is enabled.	*/
	int eventtimeout = INT32_MAX;			/*	*/


	/**/
	struct timeval timeval ={ 0, 1000 };			/*	Timeout for the inotify.	*/


	/*	Check if STDIN is piped.	*/
	isPipe = isatty(STDIN_FILENO) == 0;
	if(argc <= 1 && !isPipe){
		fprintf(stderr, "No argument.\n");
		return EXIT_FAILURE;
	}


	/*	Initialize glslview.	*/
	if(glslview_init(argc, argv) == 0){
		status = EXIT_FAILURE;
		goto error;
	}


	/*	*/
	private_start = SDL_GetPerformanceCounter();
	pretime = SDL_GetPerformanceCounter();


	if(ifd < 0 ){
		timeval.tv_sec = 0;
		timeval.tv_usec = 0;
	}


	/*	TODO improve later, because mouse input and other has to be taking into consideration.*/
	if(needsUpdate(shaders) ){
		eventtimeout = 0;
	}


	/*	*/
	while(isAlive){


		/*	*/
		while(SDL_WaitEventTimeout(&event, eventtimeout)){

			switch(event.type){
			case SDL_QUIT:
				isAlive = SDL_FALSE;
				eventtimeout = 0;
			break;
			case SDL_KEYDOWN:
				printf("%d\n", event.key.keysym.sym);
				if(event.key.keysym.sym == SDLK_RETURN && (event.key.keysym.mod & KMOD_CTRL )){
					fullscreen = ~fullscreen & SDL_TRUE;
					SDL_SetWindowFullscreen( window, fullscreen == SDL_TRUE ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
				}
			break;
			case SDL_MOUSEMOTION:
				for(x = 0; x < numShaderPass; x++){
					glUniform2fv(shaders[x].uniform.mouse, 1, &mouse[0]);
				}
			break;
			case SDL_MOUSEWHEEL:
			break;
			case SDL_WINDOWEVENT:
				switch(event.window.event){
				case SDL_WINDOWEVENT_RESIZED:
					glslview_set_viewport(event.window.data1, event.window.data2);
					for(x = 0; x < numShaderPass; x++){
						glslview_resize_screen(&event.window.data1, &shaders[x].uniform, &shaders[x], &fbackbuffertex);
					}
					glslview_rendergraphic(drawable, shaders, elapse, deltatime);
					break;
				case SDL_WINDOWEVENT_MOVED:
					for(x = 0; x < numShaderPass; x++){
						if(shaders[x].uniform.offset != -1){
							glUniform2i(shaders[x].uniform.offset, 0, 0);
						}
					}
					break;
				case SDL_WINDOWEVENT_SHOWN:
				case SDL_WINDOWEVENT_EXPOSED:
					visable = SDL_TRUE;
					SDL_GetWindowSize(window, &size.x, &size.y);
					glslview_set_viewport(size.x, size.y);
					for(x = 0; x < numShaderPass; x++){
						glslview_resize_screen(&size.x, &shaders[x].uniform, &shaders[x], &fbackbuffertex);
					}
					glslview_rendergraphic(drawable, shaders, elapse, deltatime);
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
					glslview_rendergraphic(drawable, shaders, elapse, deltatime);
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

								glDeleteProgram(shaders[x].shader.program);
								memset(&shaders[x].shader, 0, sizeof(glslviewShader));

								glslview_loadfile((const char*)fragPath[x], (void**)&fragData);
								if(glslview_create_shader(&shaders[x].shader, vertex, (const char*)fragData, NULL, NULL, NULL)){
									/**/
								}

								free(fragData);

								SDL_GetWindowSize(window, &size.x, &size.y);
								glUseProgram(shaders[x].shader.program);
								glslview_update_shader_uniform(&shaders[x].uniform, &shaders[x].uniform, size.x, size.y);
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
				glslview_rendergraphic(drawable, shaders, elapse, deltatime);
			}else{/*	render passes	*/
				sleep(1);
			}

		}/*	render condition.	*/

	}/*	End of main while loop.	*/


	error:	/*	*/

	/*	Release resources.	*/
	glslview_verbose_printf("glslview is terminating.\n");
	glslview_release_renderingapi();
	glslview_terminate();

	return status;
}
