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
#include <regex.h>
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
unsigned int isPipe;
unsigned int use_stdin_as_buffer = 0;			/*	*/
int stdin_buffer_size = 1;						/*	*/


/*	function pointers.	*/
pglslview_init_renderingapi glslview_init_renderingapi = NULL;
presize_screen glslview_resize_screen = NULL;
pupdate_shader_uniform glslview_update_shader_uniform = NULL;
pdisplaygraphic glslview_displaygraphic = NULL;
pupdate_update_uniforms glslview_update_uniforms = NULL;
pset_viewport glslview_set_viewport = NULL;
pswapbufferfunctype glslview_swapbuffer	= NULL;					/*	Function pointer for swap default framebuffer.	*/
pglslview_create_texture glslview_create_texture = NULL;
pglslview_create_shader glslview_create_shader = NULL;
pglslview_rendergraphic glslview_rendergraphic = NULL;






int glslview_readargument(int argc, const char** argv, int pass){
	static const struct option longoption[] = {
			{"version", 		no_argument, 		NULL, 'v'},				/*	application version.	*/
			{"alpha", 			no_argument, 		NULL, 'a'},				/*	use alpha channel.	*/
			{"fullscreen", 		no_argument, 		NULL, 'F'},				/*	use in fullscreen.	*/
			{"notify-file", 	no_argument, 		NULL, 'n'},				/*	enable inotify notification.	*/
			{"srgb",			no_argument, 		NULL, 'S'},				/*	sRGB.	*/
			{"Verbose", 		no_argument, 		NULL, 'V'},				/*	Verbose.	*/
			{"no-decoration", 	no_argument,	 	NULL, 'D'},				/*	Use no window decoration.	*/
			{"wallpaper", 		optional_argument, 	NULL, 'w'},				/*	use as wallpaper.	*/
			{"vsync", 			optional_argument, 	NULL, 's'},				/*	enable vsync.	*/
			{"stdin",			optional_argument, 	NULL, 'I'},				/*	stdin data as buffer.	*/
			{"debug", 			optional_argument, 	NULL, 'd'},				/*	Set application in debug mode.	*/
			{"antialiasing", 	optional_argument, 	NULL, 'A'},				/*	anti aliasing.	*/
			{"compression",		optional_argument, 	NULL, 'C'},				/*	Texture compression.	*/
			{"file", 			required_argument, 	NULL, 'f'},				/*	glsl shader file.	*/
			{"geometyshader",	required_argument, 	NULL, 'f'},				/*	geometry glsl shader file.	*/
			{"opengl", 			required_argument, 	NULL, 'g'},				/*	Opengl version.	*/
			{"renderer", 		required_argument, 	NULL, 'r'},				/*	Renderer API.	*/
			{"resolution-scale",required_argument, 	NULL, 'R'},				/*	Texture scale resolution (required gl_framebuffer_object).*/
			{"texture",			required_argument, 	NULL, 't'},				/*	Texture.	*/
			{"poly",			required_argument, 	NULL, 'p'},				/*	Polygon.	*/
			{"opencl",			required_argument, 	NULL, 'c'},				/*	Opencl.	*/

			{NULL, 0, NULL, 0}
	};


	int c;
	int index;
	int status = 1;
	const char* shortopts = "dDIsar:g:Vf:SA:t:vFnCp:w";


	/*	First argument pass.	*/
	if(pass == 0){
		glslview_verbose_printf("--------- First argument pass -------\n\n");
		while((c = getopt_long(argc, (char *const *)argv, shortopts, longoption, &index)) != EOF){
			switch(c){
			case 'v':{
				printf("Version %s.\n", glslview_getVersion());
				exit(EXIT_SUCCESS);
			}
			case 'V':
				verbose = SDL_TRUE;
				glslview_verbose_printf("Enable verbose.\n");
				break;
			case 'd':{	/*	enable debug.	*/
			    debug = SDL_TRUE;
			    int glatt;
			    SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &glatt);
			    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, glatt | SDL_GL_CONTEXT_DEBUG_FLAG);
			}break;
			case 'a':
				glslview_verbose_printf("Enable alpha buffer.\n");
				SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
				break;
			case 'S':
				glslview_verbose_printf("Set framebuffer to sRGB color space.\n");
				SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, SDL_TRUE);
				glEnable(GL_FRAMEBUFFER_SRGB);
				break;
			case 'A':
				SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, SDL_TRUE);
				if(optarg){
					SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, atoi(optarg));
				}
				else{
					SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);
				}
				glEnable(GL_MULTISAMPLE);
				glslview_verbose_printf("Set multisample framebuffer : %d samples.\n", optarg ? atoi(optarg) : 2);

				break;
			case 'r':
				if(optarg != NULL){
					int glatt;
					if(strcmp(optarg, "opengl") == 0){

						SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
						SDL_GL_GetAttribute(SDL_GL_CONTEXT_FLAGS, &glatt);
						glslview_verbose_printf("Set rendering API to OpenGL.\n");
					}
					if(strcmp(optarg, "openglcore") == 0){
						SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
						glslview_verbose_printf("Set rendering API to OpenGL core.\n");
					}
					else if(strcmp(optarg, "opengles") == 0){
						SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
						glslview_verbose_printf("Set rendering API to OpenGL-ES.\n");
					}
					else if(strcmp(optarg, "vulkan") == 0){
						glslview_init_renderingapi = glslview_init_vulkan;
						glslview_resize_screen = glslview_resize_screen_vk;
						glslview_displaygraphic = glslview_displaygraphic_vk;
						glslview_update_shader_uniform = glslview_update_shader_uniform_vk;
						glslview_update_uniforms = glslview_update_uniforms_vk;
						glslview_swapbuffer = SDL_GL_SwapWindow;
						glslview_create_texture = glslview_create_texture_vk;
						glslview_create_shader = glslview_create_shader_vk;
						glslview_rendergraphic = glslview_rendergraphic_vk;
						glslview_verbose_printf("Set rendering API to Vulkan.\n");
					}
				}
				break;
			case 'C':
				glslview_verbose_printf("Enable texture compression.\n");
				compression = SDL_TRUE;
				break;
			case 'g':
				if(optarg){
					int len = strlen(optarg);
					if(len > 3){
						continue;
					}
					/*	Set opengl version requested by input argument.	*/
					SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, strtol(optarg, NULL, 10) / 100);
					SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, (strtol(optarg, NULL, 10) % 100 ) / 10);
					glslview_verbose_printf("Set OpenGL version %d.%d0", strtol(optarg, NULL, 10) / 100, (strtol(optarg, NULL, 10) % 100) / 10);
				}
				break;
			case 'I':	/*	use pipe stdin as buffer.	*/
				use_stdin_as_buffer = SDL_TRUE;
				if(optarg){
					stdin_buffer_size = strtol(optarg, NULL, 10);
				}
				glslview_verbose_printf("Stdin used as input buffer for glsl shader with read size %d.\n", stdin_buffer_size);
				break;
			case 'f':
				if(optarg){
					fragPath[numFragPaths] = optarg;
					numFragPaths++;
					glslview_verbose_printf("shader file %s\n", optarg);
				}
				break;
			case 'c':	/*	TODO create opencl context.	*/
				break;
			case '\?':
			case ':':
			default:
				break;
			}
		}

		/*	fragment path is the only argument that has the option to use no flag.	*/
		if(c == -1 && optind < argc && fragPath == NULL){
			fragPath[numFragPaths] = (char*)argv[optind];
			numFragPaths++;
			glslview_verbose_printf("shader file %s\n", argv[optind]);
		}

	}else if(pass == 1){	/*	Second argument pass.	*/
		glslview_verbose_printf("--------- Second argument pass -------\n\n");

		/**/
		while((c = getopt_long(argc, (char *const *)argv, shortopts, longoption, &index)) != EOF){
			switch(c){
			case 'A':
				if(optarg){
					if(strcmp(optarg, "msaa") == 0){

					}

				}
				break;
			case 'F':	/*	Fullscreen.	*/
				fullscreen = SDL_TRUE;
				SDL_DisplayMode dismod;
				glslview_verbose_printf("Enable fullscreen mode.\n");
				SDL_GetCurrentDisplayMode(
						SDL_GetWindowDisplayIndex(window),
						&dismod);
				SDL_SetWindowSize(window, dismod.w, dismod.h);
				SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
				break;
			case 'w':{	/*	Set as desktop wallpaper.	*/	/*	TODO fix for other distro other than Ubuntu.*/
				SDL_Point size;
				SDL_Point location = {0};
				SDL_Rect rect;
				int index = 0;
				SDL_Window* desktop = NULL;
				if(desktop){
					SDL_GetWindowSize(desktop, &size.x,size.y);
					//SDL_SetWindowPosition()

					if(optarg){
						if(strlen(optarg) > 0){
							index = strtol(optarg, NULL, 10);
							glslview_verbose_printf("Monitor screen index %d selected for wallpaper.\n", index);
							/*
							ExGetScreenRect(index, &rect);
							location.x = rect.x;
							location.y = rect.y;
							size.width = rect.width;
							size.height = rect.height;
							*/
						}
					}

					/*	Resize window to fix desktop view.	*/
					//privatefprintf("Set view as desktop wallpaper %d.%d, %dx%d.\n", location.x, location.y, size.width, size.height);
					//ExSetWindowSize(window, size.width, size.height);
					//ExSetWindowPos(window, location.x, location.y);
					//ExMakeDesktopWindow(window);
					/*	TODO disable window flag events.	*/
					/*	ExSetWindowFlag(window, ExGetWindowFlag(window));	*/

				}else{
					glslview_verbose_printf("Couldn't find desktop window handle.\n");
					exit(EXIT_FAILURE);
				}
			}break;
			case 's':	/*	Enable OpenGL V Sync.	*/
				glslview_verbose_printf("Enable V-Sync.\n");
				SDL_GL_SetSwapInterval(1);
				break;
			case 'D':
				SDL_SetWindowBordered(window, SDL_FALSE);
				break;
			case 'n':{
				char buf[4096];
				int x;
				glslview_verbose_printf("Initialize inotify.\n");

				/*	initialize inotify.	*/
				ifd = inotify_init1(IN_NONBLOCK);
				if(ifd < 0){
					fprintf(stderr, "%s\n", strerror(errno));
					exit(EXIT_FAILURE);
				}

				/*	Get absolute path for inotify watch.	*/
				for(x = 0; x < numFragPaths; x++){
					memcpy(buf, fragPath[x], strlen(fragPath[x]) + 1);
					dirname(buf);


					if(( wd = inotify_add_watch(ifd, buf, IN_MODIFY | IN_DELETE)) < 0){
						fprintf(stderr, "Failed to add inotify %s %s.\n", buf, strerror(errno));
						exit(EXIT_FAILURE);
					}
					glslview_verbose_printf("Added %s directory to inotify watch.\n\n", buf);
				}

				inotifybuf = malloc(4096);
			case 't':
				if(optarg){
					regex_t reg;
					unsigned int width;
					unsigned int height;
					unsigned int bpp;
					void* bitdata;
					int x = 0;
					FREE_IMAGE_COLOR_TYPE colortype;
					FREE_IMAGE_FORMAT format;
					FIBITMAP* bitmap;
					unsigned int gformat = GL_RGB;
					unsigned int ginternalformat = GL_RGB;
					unsigned int type = GL_UNSIGNED_BYTE;

					/*	*/
					FreeImage_Initialise(0);
					glslview_verbose_printf("FreeImage version : %s\n\n", FreeImage_GetVersion());

					glslview_debug_printf("Attempt to load texture %s.\n", argv[optind + x -1]);

					/*	TODO add support for regular expression for texture.	*/
					regcomp(&reg, "*", 0);
					regexec(&reg, argv[optind + x -1], 0, NULL, 0);
					regfree(&reg);

					while( ( format = FreeImage_GetFileType(argv[optind + x -1], 0) ) != FIF_UNKNOWN ){

						/**/
						bitmap = FreeImage_Load( format, argv[optind + x -1], 0 );

						if(bitmap){
							glslview_verbose_printf("Reading texture %s for uniform tex%d.\n", argv[optind + x -1], nextTex);
							/*texturess = (glslviewTextureCollection*)realloc(texturess, (nextTex + 1) * sizeof(glslviewTextureCollection));*/

							/*	Extracting texture attributes.	*/
							colortype = FreeImage_GetColorType(bitmap);
							width = FreeImage_GetWidth(bitmap);
							height = FreeImage_GetHeight(bitmap);
							bpp = FreeImage_GetBPP(bitmap);



							/*	TODO Fix with the constants. */
							switch(colortype){
							case FIC_RGB:
								gformat = GL_RGB;
								ginternalformat = GL_RGB;
								bitmap = FreeImage_ConvertTo24Bits(bitmap);
								break;
							case FIC_RGBALPHA:
								gformat = GL_RGBA;
								ginternalformat = GL_RGBA;
								bitmap = FreeImage_ConvertTo32Bits(bitmap);
								break;
							default:
								break;
							}

							if(compression){
								/*	get opengl internal compression format.	*/
								switch(gformat){
								case GL_RGB:
									ginternalformat = GL_COMPRESSED_RGB;
								break;
								case GL_RGBA:
									ginternalformat = GL_COMPRESSED_RGBA;
									break;
								}
							}

							bitdata = FreeImage_GetBits(bitmap);

							/*	Create opengl 2D texture.	*/
							glslview_create_texture(&textures[nextTex], GL_TEXTURE_2D, 0, ginternalformat, width,
									height, 0, gformat, type, bitdata);

							nextTex++;
							glslview_verbose_printf("fileformat : %d \nwidth : %d\nheight : %d\nbpp : %d\n\n", format, width, height, bpp);

							FreeImage_Unload(bitmap);

						}else{
							glslview_verbose_printf("Failed to read texture %s.\n", optarg);
						}/**/
						x++;
					}
					FreeImage_DeInitialise();
				}
			}break;
			default:
				break;
			}/**/

		}/**/
	}

	/*	Reset getopt.	*/
	optarg = NULL;
	opterr = 0;
	optind = 0;
	optopt = 0;

	return status;
}

void glslview_catchSig(int signal){
	switch(signal){
	case SIGINT:
	case SIGQUIT:
		isAlive = SDL_FALSE;
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

}






int main(int argc, const char** argv){
	int status = EXIT_SUCCESS;				/*	*/
	SDL_Event event = {0};					/*	*/
	float ttime;
	SDL_Point size;							/*	*/
	char* fragData = NULL;					/*	*/
	int x;									/*	*/

	/**/
	long int private_start;					/*	*/
	long int pretime;
	long int deltatime;
	int visable = 1;
	int renderInBackground = 0;

	/**/
	struct timeval timeval;

	/*	*/
	isPipe = isatty(STDIN_FILENO) == 0;
	if(argc <= 1 && !isPipe){
		fprintf(stderr, "No argument.\n");
		return EXIT_FAILURE;
	}

	/**/
	if(glslview_init(argc, argv) != 0){
		goto error;
	}

	drawable = SDL_GL_GetCurrentWindow();


	/*	*/
	private_start = SDL_GetPerformanceCounter();
	pretime = SDL_GetPerformanceCounter();



	if(ifd > 0 ){
		timeval.tv_sec = 0;
		timeval.tv_usec = 1000;
	}else{
		timeval.tv_sec = 0;
		timeval.tv_usec = 0;
	}


	/*	*/
	if(glIsTexture(fbackbuffertex.texture) == GL_TRUE){
		glActiveTexture(GL_TEXTURE0 + numTextures);
		glBindTexture(fbackbuffertex.target, fbackbuffertex.texture);
	}

	/*	Bind all textures.	*/
	for(x = 0; x < numTextures; x++){
		if(glIsTexture(textures[x].texture) == GL_TRUE){
			glslview_verbose_printf("Binding texture %d.\n", x);
			glActiveTexture(GL_TEXTURE0 + x);
			glBindTexture(textures[x].target, textures[x].texture);
		}
	}

	/*	*/
	while(isAlive){

		/**/
		while(SDL_PollEvent(&event)){

			if(event.type == SDL_QUIT){
				isAlive = SDL_FALSE;
			}

			/**/
			if(event.type == SDL_KEYDOWN){
				printf("%d\n", event.key.keysym.sym);
				if(event.key.keysym.sym == SDLK_RETURN && (event.key.keysym.mod & KMOD_CTRL )){
					fullscreen = ~fullscreen & SDL_TRUE;
					SDL_SetWindowFullscreen( window, fullscreen == SDL_TRUE ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
				}
			}

			/**/
			if(event.type == SDL_MOUSEMOTION){

				float mouse[2];// = {location.x , -location.y };
				for(x = 0; x < numShaderPass; x++){
					glUniform2fv(shaders[x].uniform.mouse, 1, &mouse[0]);
				}
			}

			if(event.type == SDL_MOUSEWHEEL){

			}

			if(event.type == SDL_WINDOWEVENT){
				switch(event.window.event){
				case SDL_WINDOWEVENT_RESIZED:
					glslview_set_viewport(event.window.data1, event.window.data2);
					for(x = 0; x < numShaderPass; x++){
						glslview_resize_screen(&event.window.data1, &shaders[x].uniform, &shaders[x], &fbackbuffertex);
					}
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
					break;
				case SDL_WINDOWEVENT_HIDDEN:
					visable = SDL_FALSE;
					break;
				default:
					break;
				}
			}

			/*
			if( ( event.event & EX_EVENT_WINDOW_DESTROYED ) && event.destroy.window == window){
				isAlive = FALSE;
				goto error;
			}
			*/
		}

		ttime = (float)(( SDL_GetPerformanceCounter() - private_start) / 1E9);
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
					glslview_rendergraphic(drawable, shaders, ttime, deltatime);
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
				glslview_rendergraphic(drawable, shaders, ttime, deltatime);
			}else{/*	render passes	*/
				sleep(1);
			}

		}/*	render condition.	*/

	}/*	End of main while loop.	*/


	error:	/*	*/

	glslview_verbose_printf("glslview is terminating.\n");
	glslview_terminate();


	/*	Release OpenGL resources.	*/
	if(glc != NULL){
		for(x = 0; x < numShaderPass; x++){
			if(glIsProgram(shaders[x].shader.program) == GL_TRUE){
				glDeleteProgram(1, &shaders[x].shader.program);
			}
		}
		if(glIsVertexArray(vao) == GL_TRUE){
			glDeleteVertexArrays(1, &vao);
		}
		if(glIsBuffer(vbo) == GL_TRUE ){
			glDeleteBuffers(1, &vbo);
		}

		/*	textures.	*/
		for(x = 0; x < numTextures; x++){
			if(glIsTexture(textures[x].texture) == GL_TRUE){
				glDeleteTextures(1, &textures[x].texture);
			}
		}

		if(glIsTexture(fbackbuffertex.texture) == GL_TRUE){
			glDeleteTextures(1, &fbackbuffertex.texture);
		}

		SDL_GL_DeleteContext(glc);
	}
	if(window){
		SDL_DestroyWindow(window);
	}

	/*	*/
	if(ifd != -1){
		inotify_rm_watch(ifd, wd);
		free(inotifybuf);
		close(ifd);
	}

	SDL_Quit();
	return status;
}
