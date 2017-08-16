#include"glslview.h"
#include<signal.h>
#include<unistd.h>
#include<getopt.h>
#include<errno.h>
#include<GL/gl.h>
#include<SDL2/SDL.h>
#include<GL/gl.h>
#include<FreeImage.h>
/*	TODO remove glsl function later.	*/
#include<sys/inotify.h>	/*	TODO fix such that it uses a portable solution.	*/


const char* glslview_getVersion(void){
	return COMPILED_VERSION(GLSLVIEW_MAJOR_VERSION,
			GLSLVIEW_MINOR_VERSION,
			GLSLVIEW_REVISION_VERSION, GLSLVIEW_STATE_VERSION);
}


int glslview_init(int argc, const char** argv){

	int status = 1;							/*	Return status of the function.	*/
	ssize_t srclen;							/*	*/

	SDL_DisplayMode displaymode;			/*	*/
	SDL_version sdlver;						/*	*/
	char title[512];						/*	*/
	char* fragData = NULL;					/*	*/
	int x;									/*	Iterator.	*/


	/*	Read user argument options.	*/
	if(glslview_readargument(argc, argv, 0) == 2){
		return 0;
	}

	/*	Display information about the application.	*/
	printf("\n");
	printf("glslview v%s\n", glslview_getVersion());

	SDL_GetVersion(&sdlver);
	printf("SDL version %d.%d.%d\n", sdlver.major, sdlver.minor, sdlver.patch);
	printf("==================\n\n");

	/*	Initialize SDL.	*/
	if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0){
		fprintf(stderr, "Failed to initialize SDL, %s.\n", SDL_GetError());
		status = 0;
		return status;
	}

	numShaderPass = numFragPaths;

	/*	Assign signal interrupts.	*/
	signal(SIGILL, glslview_catchSig);
	signal(SIGINT, glslview_catchSig);
	signal(SIGABRT, glslview_catchSig);
	signal(SIGPIPE, glslview_catchSig);


	/*	Create window.	*/
	g_window = glslview_gl_init();
	if(g_window == NULL){
		fprintf(stderr, "Failed to init rendering API.\n");
		status = 0;
		return status;
	}

	/*	Set window attributes.	*/
	sprintf(title, "glslview %s", glslview_getVersion());
	SDL_ShowWindow(g_window);
	SDL_SetWindowTitle(g_window, title);
	SDL_GetCurrentDisplayMode(0, &displaymode);
	SDL_SetWindowPosition(g_window, displaymode.w / 4, displaymode.h / 4);
	SDL_SetWindowSize(g_window, displaymode.w / 2, displaymode.h  / 2 );

	/*	Second argument read.	*/
	if(glslview_readargument(argc, argv, 1) == 2){
		status = EXIT_FAILURE;
		return status;
	}


	/*	Load shader fragment source code.	*/
	glslview_verbose_printf("----------- fetching source code ----------\n");
	if(numFragPaths < 1){
		fprintf(stderr, "Requires at least one fragment argument.\n");
		return 0;
	}

	/*	Allocate shaders buffer collection.	*/
	g_shaders = malloc(sizeof(glslviewShaderCollection) * numFragPaths);
	assert(g_shaders);
	memset(g_shaders, 0, sizeof(glslviewShaderCollection) * numFragPaths);

	/*	Read from pipe if used.	*/
	if(g_isPipe && !use_stdin_as_buffer){
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

		/*	Iterathe through each fragment shader.	*/
		for(x = 0; x < numFragPaths; x++){
			srclen = glslview_loadString((const char*)fragPath[x], (void**)&fragData);
			glslview_debug_printf("Loaded shader file %s, with size of %d bytes.\n", fragPath[x], srclen);

			/*	Compile shader.	*/
			glslview_verbose_printf("----------- compiling source code ----------\n");
			if(glslview_gl_create_shader(&g_shaders[x].shader, vertex, fragData, NULL, NULL, NULL) == 0){
				fprintf(stderr, "%s, invalid shader.\n", fragPath[x]);
				return 0;
			}else{
				glslview_gl_update_shader_uniform(&g_shaders[x].uniform, &g_shaders[x].shader, displaymode.w, displaymode.h);
			}

			/*	free fragment source.	*/
			free(fragData);
			fragData = NULL;
		}
	}

	return status;
}

void glslview_terminate(void){

	/*	Releae rendering resources.	*/
	glslview_gl_release();

	if(g_window != NULL){
		SDL_DestroyWindow(g_window);
	}

	/*	*/
	if(ifd != -1){
		inotify_rm_watch(ifd, wd);
		free(inotifybuf);
		close(ifd);
	}

	SDL_Quit();
}


int glslview_readargument(int argc, const char** argv, int pass){


	static const struct option longoption[] = {
			{"version", 		no_argument, 		NULL, 'v'},				/*	application version.	*/
			{"alpha", 			no_argument, 		NULL, 'a'},				/*	use alpha channel.	*/
			{"fullscreen", 		no_argument, 		NULL, 'F'},				/*	use in fullscreen.	*/
			{"notify-file", 	no_argument, 		NULL, 'n'},				/*	enable inotify notification.	*/
			{"srgb",			no_argument, 		NULL, 'S'},				/*	sRGB.	*/
			{"verbose", 		no_argument, 		NULL, 'V'},				/*	Verbose.	*/
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
			{"param",			required_argument,	NULL, 'P'},				/*	*/
			{NULL, 0, NULL, 0}
	};


	int c;
	int index;
	int status = 1;
	const char* shortopts = "dDIsar:g:Vf:SA:t:vFnCp:wP:";

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
				glslview_set_verbosity_level(GLSLVIEW_VERBOSE);
				glslview_verbose_printf("Enable verbose.\n");
				break;
			case 'd':{	/*	enable debug.	*/
			    g_debug = SDL_TRUE;
			    int glatt;
				glslview_set_verbosity_level(GLSLVIEW_DEBUG);
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
				if(optarg){
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
				}
				break;
			case 'C':
				glslview_verbose_printf("Enable texture compression.\n");
				g_compression = SDL_TRUE;
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
				g_fullscreen = SDL_TRUE;
				SDL_DisplayMode dismod;
				glslview_verbose_printf("Enable fullscreen mode.\n");
				SDL_GetCurrentDisplayMode(
						SDL_GetWindowDisplayIndex(g_window),
						&dismod);
				SDL_SetWindowSize(g_window, dismod.w, dismod.h);
				SDL_SetWindowFullscreen(g_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
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
				SDL_SetWindowBordered(g_window, SDL_FALSE);
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
				break;
			case 't':
				if(optarg){

					unsigned int width;									/*	*/
					unsigned int height;								/*	*/
					unsigned int bpp;									/*	*/
					void* bitdata;										/*	*/
					int x = 0;											/*	*/
					FREE_IMAGE_COLOR_TYPE colortype;					/*	*/
					FREE_IMAGE_FORMAT format;							/**/
					FIBITMAP* bitmap = NULL;							/**/
					FIBITMAP* convbitmap = NULL;						/**/

					unsigned int gformat = TEXTURE_RGB;					/**/
					unsigned int ginternalformat = TEXTURE_RGB;			/**/
					unsigned int type = GL_UNSIGNED_BYTE;				/**/

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

							if(g_compression){
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
							glslview_gl_create_texture(&textures[nextTex], GL_TEXTURE_2D, 0, ginternalformat, width,
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
