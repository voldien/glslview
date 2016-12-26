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
#include<GL/gl.h>
#include<FreeImage.h>
/*	TODO remove glsl function later.	*/


#include<getopt.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<signal.h>

#include<sys/inotify.h>	/*	TODO fix such that it uses a portable solution.	*/

/*	Default value.	*/
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
	glslview_init_renderingapi = glslview_init_opengl;
	glslview_release_renderingapi = glslview_release_opengl;
	glslview_resize_screen = glslview_resize_screen_gl;
	glslview_displaygraphic = glslview_displaygraphic_gl;
	glslview_update_shader_uniform = glslview_update_shader_uniform_gl;
	glslview_update_uniforms = glslview_update_uniforms_gl;
	glslview_set_viewport = glslview_set_viewport_gl;
	glslview_swapbuffer = SDL_GL_SwapWindow;
	glslview_create_texture = glslview_create_texture_gl;
	glslview_create_shader = glslview_create_shader_gl;
	glslview_rendergraphic = glslview_rendergraphic_gl;
}



int glslview_init(int argc, const char** argv){
	int status = EXIT_SUCCESS;					/*	Return status of the function.	*/
	ssize_t srclen;								/*	*/

	SDL_DisplayMode displaymode;				/*	*/
	SDL_version sdlver;							/*	*/
	char title[512];							/*	*/
	char* fragData = NULL;						/*	*/
	int x;										/*	Iterator.	*/


	/*	Initialize default values that has to been set
	 *  in order for the software to work.*/
	glslview_default_init();


	/*	Display information about the application.	*/
	printf("\n");
	printf("glslview v%s\n", glslview_getVersion());

	SDL_GetVersion(&sdlver);
	printf("SDL version %d.%d.%d\n", sdlver.major, sdlver.minor, sdlver.patch);
	printf("==================\n\n");

	/*	Initialize SDL.	*/
	if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0){
		fprintf(stderr, "Failed to initialize SDL, %s.\n", SDL_GetError());
		status = EXIT_FAILURE;
		return status;
	}


	/*	*/
	if(glslview_readargument(argc, argv, 0) == 2){
		return EXIT_SUCCESS;
	}
	numShaderPass = numFragPaths;


	/*	*/
	signal(SIGILL, glslview_catchSig);
	signal(SIGINT, glslview_catchSig);
	signal(SIGABRT, glslview_catchSig);
	signal(SIGPIPE, glslview_catchSig);



	/*	Create window. */
	window = glslview_init_renderingapi();
	if(window == NULL){
		status = EXIT_FAILURE;
		return status;
	}


	/*	Set window attributes.	*/
	sprintf(title, "glslview %s", glslview_getVersion());
	SDL_ShowWindow(window);
	SDL_SetWindowTitle(window, title);
	SDL_SetWindowPosition(window, displaymode.w / 2, displaymode.h / 2);
	SDL_SetWindowSize(window, displaymode.w, displaymode.h );




	/*	Second argument read.	*/
	if(glslview_readargument(argc, argv, 1) == 2){
		status = EXIT_FAILURE;
		return status;
	}


	/*	Load shader fragment source code.	*/
	glslview_verbose_printf("----------- fetching source code ----------\n");
	shaders = malloc(sizeof(glslviewShaderCollection) * numFragPaths);
	memset(shaders, 0, sizeof(glslviewShaderCollection) * numFragPaths);

	/**/
	if(isPipe && !use_stdin_as_buffer){
		char buf[4096];
		ssize_t len;
		ssize_t totallen = 0;

		while( ( len = read(STDIN_FILENO, buf, sizeof(buf) ) ) > 0 ){
			fragData = realloc(fragData, totallen + len);
			memcpy(fragData + totallen, buf, len);
			totallen += len;
		}

	}
	else{
		for(x = 0; x < numFragPaths; x++){
			srclen = glslview_loadfile((const char*)fragPath[x], (void**)&fragData);
			glslview_debug_printf("Loaded shader file %s, with size of %d bytes.\n", fragPath[x], srclen);

			/*	compile shader.	*/
			glslview_verbose_printf("----------- compiling source code ----------\n");
			if(glslview_create_shader(&shaders[x].shader, vertex, fragData, NULL, NULL, NULL) == 0){
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

	return status;
}

const char* glslview_getVersion(void){
	return COMPILED_VERSION(GLSLVIEW_MAJOR_VERSION,
			GLSLVIEW_MINOR_VERSION,
			GLSLVIEW_REVISION_VERSION);
}


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
						glslview_release_renderingapi = glslview_release_vulkan;
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
					glslview_verbose_printf("Set OpenGL version %d.%d0.\n", strtol(optarg, NULL, 10) / 100, (strtol(optarg, NULL, 10) % 100) / 10);
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
					fprintf(stderr, "%s.\n", strerror(errno));
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
				assert(inotifybuf);
			case 't':
				if(optarg){

					unsigned int width;
					unsigned int height;
					unsigned int bpp;
					void* bitdata;
					int x = 0;
					FREE_IMAGE_COLOR_TYPE colortype;
					FREE_IMAGE_FORMAT format;
					FIBITMAP* bitmap = NULL;
					FIBITMAP* convbitmap = NULL;

					unsigned int gformat = TEXTURE_RGB;
					unsigned int ginternalformat = TEXTURE_RGB;
					unsigned int type = GL_UNSIGNED_BYTE;

					/*	*/
					FreeImage_Initialise(0);
					glslview_verbose_printf("FreeImage version : %s\n\n", FreeImage_GetVersion());

					glslview_debug_printf("Attempt to load texture %s.\n", argv[optind + x -1]);

					while( ( format = FreeImage_GetFileType(argv[optind + x - 1], 0) ) != FIF_UNKNOWN ){

						/**/
						bitmap = FreeImage_Load( format, argv[optind + x - 1], 0 );

						if(bitmap){
							glslview_verbose_printf("Reading texture %s for uniform tex%d.\n", argv[optind + x -1], nextTex);
							/*texturess = (glslviewTextureCollection*)realloc(texturess, (nextTex + 1) * sizeof(glslviewTextureCollection));*/

							/*	Extracting texture attributes.	*/
							colortype = FreeImage_GetColorType(bitmap);
							width = FreeImage_GetWidth(bitmap);
							height = FreeImage_GetHeight(bitmap);
							bpp = FreeImage_GetBPP(bitmap);



							switch(colortype){
							case FIC_RGB:
								gformat = TEXTURE_RGB;
								ginternalformat = TEXTURE_RGB;
								convbitmap = FreeImage_ConvertTo24Bits(bitmap);
								break;
							case FIC_RGBALPHA:
								gformat = TEXTURE_RGBA;
								ginternalformat = TEXTURE_RGBA;
								convbitmap = FreeImage_ConvertTo32Bits(bitmap);
								break;
							default:
								break;
							}

							if(compression){
								/*	get opengl internal compression format.	*/
								switch(gformat){
								case GL_RGB:
									ginternalformat = TEXTURE_COMPRESSION_RGB;
								break;
								case GL_RGBA:
									ginternalformat = TEXTURE_COMPRESSION_RGBA;
									break;
								default:
									break;
								}
							}


							bitdata = FreeImage_GetBits(convbitmap);
							if(bitdata == NULL){
								fprintf(stderr, "Failed to get image data.\n");
							}

							/*	Create opengl 2D texture.	*/
							glslview_create_texture(&textures[nextTex], GL_TEXTURE_2D, 0, ginternalformat, width,
									height, 0, gformat, type, bitdata);

							nextTex++;
							glslview_verbose_printf("fileformat : %d \nwidth : %d\nheight : %d\nbpp : %d\n\n", format, width, height, bpp);

							FreeImage_Unload(convbitmap);
							FreeImage_Unload(bitmap);


						}else{
							glslview_verbose_printf("Failed to read texture %s.\n", optarg);
						}

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





long int glslview_loadfile(const char* cfilename, void** bufferptr){
	FILE*f;
	void* buffer;
	long length;
	long int pos;
	*bufferptr = NULL;

	if(!cfilename){
		return -1;
	}

	f = fopen(cfilename, "rb");
	if(!f){
		return -1;
	}

	/*	Get file length in bytes.	*/
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


int glslview_verbose_printf(const char* format,...){
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

int glslview_debug_printf(const char* format,...){
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
