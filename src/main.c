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
#include<ELT/elt.h>
#include<ELT/graphic.h>
#include"internal.h"

#include<GL/gl.h>
#include<GL/glext.h>
#include<CL/cl.h>
#include<CL/cl_gl.h>
#include<regex.h>

#include<getopt.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>
#include<signal.h>

#include<sys/inotify.h>	/*	TODO fix such that it uses a portable solution.	*/


#include<FreeImage.h>



/*	for version 2.0, 3D objects with hpm for high performance matrices operators.
 *	and assimp for extracting geometrices;
 */
//#include<hpm/hpm.h>
//#include<assimp/cimport.h>


#ifndef GLSLVIEW_MAJOR_VERSION
	#define GLSLVIEW_MAJOR_VERSION	0
#endif	/*	GLSLVIEW_MAJOR_VERSION	*/
#ifndef GLSLVIEW_MINOR_VERSION
	#define GLSLVIEW_MINOR_VERSION	5
#endif	/*	GLSLVIEW_MINOR_VERSION	*/
#ifndef GLSLVIEW_REVISION_VERSION
	#define GLSLVIEW_REVISION_VERSION 0
#endif	/*	GLSLVIEW_REVISION_VERSION	*/


#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )

/*	default vertex shader.	*/
const char* vertex = ""
"#version 330 core\n"
"layout(location = 0) in vec3 vertex;\n"
"void main(void){\n"
"gl_Position = vec4(vertex,1.0);\n"
"}\n";

/**/
const char* quadfrag = ""
"#version 330 core\n"
"layout(location = 0) out vec4 fragColor;\n"
"uniform sampler2D cltex;\n"
"uniform vec2 resolution;\n"
"void main(void){\n"
"vec2 uv = gl_FragCoord.xy / resolution;\n"
"fragColor = texture(cltex, uv);\n"
"}\n";


/**/
const float quad[4][3] = {
		{-1.0f, -1.0f, 0.0f},
		{-1.0f, 1.0f, 0.0f},
		{ 1.0f, -1.0f, 0.0f},
		{ 1.0f,  1.0f, 0.0f},

};





ExWin window = NULL;							/*	Window.	*/
ExBoolean fullscreen = 0;						/*	Set window fullscreen.	*/
ExBoolean verbose = 0;							/*	enable verbose.	*/
ExBoolean debug = 0;							/*	enable debugging.	*/
ExBoolean compression = 0;						/*	Use compression.	*/
unsigned int rendererapi = EX_OPENGL_CORE;		/*	Rendering API.	*/
unsigned int isAlive = 1;						/*	*/
int ifd = -1;									/*	inotify file descriptor.*/
int wd = -1;									/*	inotify watch directory.	*/
char* inotifybuf = NULL;						/*	*/
unsigned int numFragPaths = 0;					/*	*/
unsigned int numShaderPass = 0;
char* fragPath[32] = {NULL};					/*	Path of fragment shader.	*/
char* clfrag = NULL;
unsigned int fbo = 0;							/*	*/
unsigned int ftextype = GL_FLOAT;
unsigned int ftexinternalformat = GL_RGBA;
unsigned int ftexformat = GL_RGBA;
ExTexture fbackbuffertex = {0};					/*	framebuffer texture for backbuffer uniform variable.	*/
ExTexture textures[8] = {{0}};					/*	*/
const int numTextures = sizeof(textures) / sizeof(textures[0]);
unsigned int nextTex = 0;						/*	*/
unsigned int use_stdin_as_buffer = 0;			/*	*/
int stdin_buffer_size = 1;						/*	*/

/*	opencl	*/
unsigned int usingopencl = 0;					/*	*/
cl_context clcontext = 0;						/*	*/
unsigned int ncldevices = 0;
cl_device_id* cldevice = NULL;					/*	*/
cl_command_queue clqueue = 0;
cl_program clprogram = 0;						/*	*/
cl_kernel clkernel = 0;							/*	*/
unsigned int numclframebuffer = 2;
cl_mem clmemframetexture[2] = {0};				/*	*/
unsigned int clcurrent = 0;
ExTexture clframetexture[2] = {{0}};				/*	*/
unsigned int numcltextures = 0;
cl_mem cltextures[16];

typedef void (*pswapbufferfunctype)(ExWin window);	/*	Function pointer data type.	*/


pswapbufferfunctype glslview_swapbuffer;					/*	Function pointer for swap default framebuffer.	*/

/*	function pointers.	*/
presize_screen glslview_resize_screen = NULL;
pupdate_shader_uniform glslview_update_shader_uniform = NULL;
pdisplaygraphic glslview_displaygraphic = NULL;
pupdate_update_uniforms glslview_update_uniforms = NULL;
pswapbufferfunctype glslview_swapbuffer	= NULL;					/*	Function pointer for swap default framebuffer.	*/





void glslview_default_init(void){
	glslview_resize_screen = glslview_resize_screen_gl;
	glslview_displaygraphic = glslview_displaygraphic_gl;
	glslview_update_shader_uniform = glslview_update_shader_uniform_gl;
	glslview_update_uniforms = glslview_update_uniforms_gl;
	glslview_swapbuffer = ExSwapBuffers;
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

#define COMPILED_VERSION(major, minor, revision) EX_STR(major)EX_TEXT(".")EX_STR(minor)EX_TEXT(".")EX_STR(revision)
const char* glslview_getVersion(void){
	return COMPILED_VERSION(GLSLVIEW_MAJOR_VERSION, GLSLVIEW_MINOR_VERSION, GLSLVIEW_REVISION_VERSION);
}


/*	TODO relocate later!	*/
extern cl_context glslview_createCLContext(ExOpenGLContext shared, unsigned int* nNumDevices, cl_device_id** id);
extern cl_program glslview_createCLProgram(cl_context context, unsigned int nNumDevices, cl_device_id* id, const char* cfilename, UniformLocation* uniform);

/**/
static int private_glslview_readargument(int argc, const char** argv, int pre){
	static const struct option longoption[] = {
			{"version", 		no_argument, NULL, 'v'},				/*	application version.	*/
			{"alpha", 			no_argument, NULL, 'a'},				/*	use alpha channel.	*/
			{"fullscreen", 		no_argument, NULL, 'F'},				/*	use in fullscreen.	*/
			{"notify-file", 	no_argument, NULL, 'n'},				/*	enable inotify notification.	*/
			{"srgb",			no_argument, NULL, 'S'},				/*	sRGB.	*/
			{"Verbose", 		no_argument, NULL, 'V'},				/*	Verbose.	*/
			{"wallpaper", 		optional_argument, NULL, 'w'},			/*	use as wallpaper.	*/
			{"vsync", 			optional_argument, NULL, 's'},			/*	enable vsync.	*/
			{"stdin",			optional_argument, NULL, 'I'},			/*	stdin data as buffer.	*/
			{"no-decoration", 	optional_argument, NULL, 'D'},			/*	*/
			{"debug", 			optional_argument, NULL, 'd'},			/*	Set application in debug mode.	*/
			{"antialiasing", 	optional_argument, NULL, 'A'},			/*	anti aliasing.	*/
			{"compression",		optional_argument, NULL, 'C'},			/*	Texture compression.	*/
			{"file", 			required_argument, NULL, 'f'},			/*	glsl shader file.	*/
			{"geometyshader",	required_argument, NULL, 'f'},			/*	geometry glsl shader file.	*/
			{"opengl", 			required_argument, NULL, 'g'},			/*	Opengl version.	*/
			{"renderer", 		required_argument, NULL, 'r'},			/*	Renderer API.	*/
			{"resolution-scale",required_argument, NULL, 'R'},			/*	Texture scale resolution (required gl_framebuffer_object).*/
			{"texture",			required_argument, NULL, 't'},			/*	Texture.	*/
			{"poly",			required_argument, NULL, 'p'},			/*	Polygon.	*/
			{"opencl",			required_argument, NULL, 'c'},			/*	Opencl.	*/

			{NULL, 0, NULL, 0}
	};

	/**/
	int c;
	int index;
	int status = 1;
	const char* shortopts = "dIhsar:g:Vf:SA:t:vFnCp:wc:";

	/*	*/
	if(pre == 0){
		privatefprintf("--------- First argument pass -------\n\n");
		while((c = getopt_long(argc, (char *const *)argv, shortopts, longoption, &index)) != EOF){
			switch(c){
			case 'v':{
				printf("Version %s.\n", glslview_getVersion());
				return (2);
			}
			case 'V':
				verbose = TRUE;
				privatefprintf("Enable verbose.\n");
				break;
			case 'h':{
				return (2);
			}
			case 'd':{	/*	enable debug.	*/
			    debug = 1;
			}break;
			case 'a':
				privatefprintf("Enable alpha buffer.\n");
				ExOpenGLSetAttribute(EX_OPENGL_ALPHA_SIZE, 8);
				break;
			case 'S':
				privatefprintf("Set framebuffer to sRGB color space.\n");
				ExOpenGLSetAttribute(EX_OPENGL_FRAMEBUFFER_SRGB_CAPABLE, TRUE);
				glEnable(GL_FRAMEBUFFER_SRGB);
				break;
			case 'A':
				ExOpenGLSetAttribute(EX_OPENGL_MULTISAMPLEBUFFERS, TRUE);
				if(optarg){
					ExOpenGLSetAttribute(EX_OPENGL_MULTISAMPLESAMPLES, atoi(optarg));
				}
				else{
					ExOpenGLSetAttribute(EX_OPENGL_MULTISAMPLESAMPLES, 2);
				}
				glEnable(GL_MULTISAMPLE);
				privatefprintf("Set multisample framebuffer : %d samples.\n", optarg ? atoi(optarg) : 2);

				break;
			case 'r':
				if(optarg != NULL){
					if(strcmp(optarg, "opengl") == 0){
						rendererapi = EX_OPENGL;
						privatefprintf("Set rendering API to OpenGL.\n");
					}
					if(strcmp(optarg, "openglcore") == 0){
						rendererapi = EX_OPENGL_CORE;
						privatefprintf("Set rendering API to OpenGL core.\n");
					}
					else if(strcmp(optarg, "opengles") == 0){
						rendererapi = EX_OPENGLES;
						privatefprintf("Set rendering API to OpenGL-ES.\n");
					}
					else if(strcmp(optarg, "vulkan") == 0){
						rendererapi = EX_VULKAN;
						glslview_resize_screen = glslview_resize_screen_vk;
						glslview_displaygraphic = glslview_displaygraphic_vk;
						glslview_update_shader_uniform = glslview_update_shader_uniform_vk;
						glslview_update_uniforms = glslview_update_uniforms_vk;
						glslview_swapbuffer = ExSwapBuffers;
						privatefprintf("Set rendering API to Vulkan.\n");
					}
				}
				break;
			case 'C':
				privatefprintf("Enable texture compression.\n");
				compression = 1;
				break;
			case 'g':
				if(optarg){
					int len = strlen(optarg);
					if(len > 3){
						continue;
					}
					/*	Set opengl version requested by input argument.	*/
					ExOpenGLSetAttribute(EX_OPENGL_MAJOR_VERSION, strtol(optarg, NULL, 10) / 100);
					ExOpenGLSetAttribute(EX_OPENGL_MINOR_VERSION, (strtol(optarg, NULL, 10) % 100 ) / 10);
					privatefprintf("Set OpenGL version %d.%d0", strtol(optarg, NULL, 10) / 100, (strtol(optarg, NULL, 10) % 100) / 10);
				}
				break;
			case 'I':	/*	use pipe stdin as buffer.	*/
				use_stdin_as_buffer = 1;
				if(optarg){
					stdin_buffer_size = strtol(optarg, NULL, 10);
				}
				privatefprintf("Stdin used as input buffer for glsl shader with read size %d.\n", stdin_buffer_size);
				break;
			case 'f':
				if(optarg){
					fragPath[numFragPaths] = optarg;
					numFragPaths++;
					privatefprintf("shader file %s\n", optarg);
				}
				break;
			case 'c':
				if(optarg){
					usingopencl = 1;
				}else{

				}
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
			privatefprintf("shader file %s\n", argv[optind]);
		}

	}else if(pre == 1){
		privatefprintf("--------- Second argument pass -------\n\n");

		while((c = getopt_long(argc, (char *const *)argv, shortopts, longoption, &index)) != EOF){
			switch(c){
			case 'A':
				if(optarg){
					if(strcmp(optarg, "msaa") == 0){

					}

				}
				break;
			case 'F':	/*	Fullscreen.	*/
				fullscreen = 1;
				privatefprintf("Set fullscreen.\n");
				ExGLFullScreen(1, window, 0, NULL);
				break;
			case 'w':{	/*	Set as desktop wallpaper.	*/	/*	TODO fix for other distro other than Ubuntu.*/
				ExSize size;
				ExPoint location = {0};
				ExRect rect;
				int index = 0;
				ExWin desktop = ExGetDesktopWindow();
				if(desktop){
					ExGetWindowSizev(desktop, &size);
					ExSetWindowParent(desktop, window);

					if(optarg){
						if(strlen(optarg) > 0){
							index = strtol(optarg, NULL, 10);
							privatefprintf("Monitor screen index %d selected for wallpaper.\n", index);
							ExGetScreenRect(index, &rect);
							location.x = rect.x;
							location.y = rect.y;
							size.width = rect.width;
							size.height = rect.height;
						}
					}

					/*	Resize window to fix desktop view.	*/
					privatefprintf("Set view as desktop wallpaper %d.%d, %dx%d.\n", location.x, location.y, size.width, size.height);
					ExSetWindowSize(window, size.width, size.height);
					ExSetWindowPos(window, location.x, location.y);
					//ExMakeDesktopWindow(window);
					/*	TODO disable window flag events.	*/
					/*	ExSetWindowFlag(window, ExGetWindowFlag(window));	*/

				}else{
					privatefprintf("Couldn't find desktop window handle.\n");
					exit(EXIT_FAILURE);
				}
			}break;
			case 's':	/*	Enable OpenGL V Sync.	*/
				privatefprintf("Enable V-Sync.\n");
				ExOpenGLSetVSync(TRUE, ExGetCurrentGLDrawable());
				break;
			case 'n':{
				char buf[PATH_MAX];
				int x;
				privatefprintf("Initialize inotify.\n");

				/*	initialize inotify.	*/
				ifd = inotify_init1(IN_NONBLOCK);
				if(ifd < 0){
					fprintf(stderr, "%s\n", strerror(errno));
					exit(EXIT_FAILURE);
				}

				/*	Get absolute path for inotify watch.	*/
				for(x = 0; x < numFragPaths; x++){
					ExGetAbsolutePath(fragPath[x], buf, sizeof(buf));
					ExGetDirectory(buf, buf, sizeof(buf));
					if(( wd = inotify_add_watch(ifd, buf, IN_MODIFY | IN_DELETE)) < 0){
						exit(EXIT_FAILURE);
					}
					privatefprintf("Added %s directory to inotify watch.\n\n", buf);
				}

				inotifybuf = malloc(4096);
			case 'c':
				if(optarg){	/*	Requires argument.	*/
					if(clcontext == NULL){
						privatefprintf("Creating OpenCL Context.\n");
						clcontext = glslview_createCLContext(ExGetCurrentOpenGLContext(), &ncldevices, &cldevice);
						if(clcontext == NULL){
							exit(EXIT_FAILURE);
						}
					}
					privatefprintf("Creating OpenCL program, source %s\n", optarg);
					clprogram = glslview_createCLProgram(clcontext, ncldevices, cldevice, optarg, NULL);
					if(clprogram == NULL){
						exit(EXIT_FAILURE);
					}


				}
				break;
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
					privatefprintf("FreeImage version : %s\n\n", FreeImage_GetVersion());

					debugprintf("Attempt to load texture %s.\n", argv[optind + x -1]);


					/*	TODO add support for regular expression for texture.	*/
					regcomp(&reg, "*", 0);
					regexec(&reg, argv[optind + x -1], 0, NULL, 0);
					regfree(&reg);

					while( ( format = FreeImage_GetFileType(argv[optind + x -1], 0) ) != FIF_UNKNOWN ){

						/**/
						bitmap = FreeImage_Load( format, argv[optind + x -1], 0 );

						if(bitmap){
							privatefprintf("Reading texture %s for uniform tex%d.\n", argv[optind + x -1], nextTex);

							/*	Extracting texture attributes.	*/
							colortype = FreeImage_GetColorType(bitmap);
							width = FreeImage_GetWidth(bitmap);
							height = FreeImage_GetHeight(bitmap);
							bpp = FreeImage_GetBPP(bitmap);
							bitdata = FreeImage_GetBits(bitmap);


							switch(colortype){
							case FIC_RGB:
								gformat = GL_RGB;
								ginternalformat = GL_RGB;
								break;
							case FIC_RGBALPHA:
								gformat = GL_RGBA;
								ginternalformat = GL_RGB;
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

							/*	Create opengl 2D texture.	*/
							ExCreateTexture(&textures[nextTex], GL_TEXTURE_2D, 0, ginternalformat, width,
									height, 0, gformat, type, bitdata);

							nextTex++;
							privatefprintf("fileformat : %d \nwidth : %d\nheight : %d\nbpp : %d\n\n", format, width, height, bpp);

							FreeImage_Unload(bitmap);

						}else{
							privatefprintf("Failed to read texture %s.\n", optarg);
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
		isAlive = FALSE;
		break;
	case SIGTERM:
	case SIGABRT:
		exit(0);
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
	ERESULT status = EXIT_SUCCESS;				/*	*/

	ExEvent event = {0};						/*	*/
	ExWin drawable = NULL;						/*	*/

	UniformLocation uniform[32] = {{0}};	/*	uniform.	*/
	ExShader shader[32] = {{0}};						/*	*/
	unsigned int isPipe;						/*	*/
	long int srclen;							/*	*/

	float ttime;

	ExSize size;								/*	*/
	ExChar title[512];							/*	*/

	char* fragData = NULL;						/*	*/
	int x;										/*	*/

	/*	quad buffer.	*/
	unsigned int vao = 0;						/*	*/
	unsigned int vbo = 0;						/*	*/






	/**/
	long int private_start = ExGetHiResTime();	/*	*/
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


	/*	Init values that has to been set in order for the software to work.	*/
	glslview_default_init();
	/*	First argument reading pass.	*/
	if(private_glslview_readargument(argc, argv, 0) == 2){
		return EXIT_SUCCESS;
	}
	numShaderPass = numFragPaths;

	/**/
	printf("\n");
	printf("glslview v%d.%d.%d\n", GLSLVIEW_MAJOR_VERSION, GLSLVIEW_MINOR_VERSION, GLSLVIEW_REVISION_VERSION);
	printf("==================\n\n");

	/*	Initialize ELT.	*/
	privatefprintf("ELT version %s\n", ExGetVersion());
	if(ExInit(EX_INIT_NONE) == 0){
		status = EXIT_FAILURE;
		goto error;
	}

	/*	*/
	signal(SIGILL, glslview_catchSig);
	signal(SIGINT, glslview_catchSig);
	signal(SIGQUIT, glslview_catchSig);
	signal(SIGABRT, glslview_catchSig);
	signal(SIGPIPE, glslview_catchSig);



	/*	Create window. */
	ExGetPrimaryScreenSize(&size);
	size.width /= 2;
	size.height /= 2;
	window = ExCreateWindow(size.width / 2, size.height / 2, size.width, size.height, rendererapi);
	if(!window){
		status = EXIT_FAILURE;
		goto error;
	}
	/*
	glc = ExGetCurrentOpenGLContext();
	assert(glc);
	if(usingopencl){
		clc = createCLContext(glc, &deviceid);
	}
	*/

	/*	*/
	ExShowWindow(window);
	ExGetApplicationName(title, sizeof(title));
	ExSetWindowTitle(window, title);
	ExSetWindowPos(window, size.width / 2, size.height / 2);
	ExSetWindowSize(window, size.width, size.height);


	/*	Display OpenGL information.	*/
	privatefprintf("-------------- OpenGL Information ------------------\n");
	privatefprintf("OpenGL version %d %s\n", ExGetOpenGLVersion(NULL, NULL), "");
	privatefprintf("OpenGL shader language version %d\n", ExGetOpenGLShadingVersion());
	privatefprintf("OpenGL RENDERER %s\n\n", glGetString(GL_RENDERER));

	/*	*/
	if(private_glslview_readargument(argc, argv, 1) == 2){
		status = EXIT_FAILURE;
		goto error;
	}



	/*	Load shader fragment source code.	*/

	if(usingopencl == 0){
		privatefprintf("----------- fetching source code ----------\n");
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
				srclen = ExLoadFile((const char*)fragPath[x], (void**)&fragData);
				debugprintf("Loaded shader file %s, with size of %d bytes.\n", fragPath[x], srclen);

				/*	compile shader.	*/
				privatefprintf("----------- compiling source code ----------\n");
				if(ExLoadShaderv(&shader[x], vertex, fragData, NULL, NULL, NULL) == 0){
					fprintf(stderr, "Invalid shader.\n");
					status = EXIT_FAILURE;
					goto error;
				}else{
					glslview_update_shader_uniform(&uniform[x], &shader[x], size.width, size.height);
				}

				if( srclen < 0 ){
					status = EXIT_FAILURE;
					goto error;
				}

				/*	free fragment source.	*/
				free(fragData);
				fragData = NULL;
			}
		}
	}else{
		privatefprintf("----------- Creating quad fragment ----------\n");
		if(ExLoadShaderv(&shader[0], vertex, quadfrag, NULL, NULL, NULL) == 0){
			fprintf(stderr, "Invalid shader.\n");
			status = EXIT_FAILURE;
			goto error;
		}else{
			glslview_update_shader_uniform(&uniform[0], &shader[0], size.width, size.height);
		}

		glslview_acquirecltextures(clcontext, clqueue, clkernel);
	}






	/*	generate vertex array for quad.	*/
	privatefprintf("----------- constructing rendering quad. ----------\n");
	ExGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	ExGenBuffers(1, &vbo);

	/*	*/
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, (const void*)0);

	/*	*/
	glBindVertexArray(0);

	/*	*/
	drawable = ExGetCurrentGLDrawable();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(FALSE);
	glDisable(GL_STENCIL_TEST);

	/*	*/
	ExGetWindowSizev(window, &size);
	glViewport(0, 0, size.width, size.height);

	/*	*/

	glBindVertexArray(vao);




	/*	*/
	if(ExOpenGLGetAttribute(EX_OPENGL_ALPHA_SIZE, NULL) > 0){
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glEnable(GL_ALPHA_TEST);
		glColorMask(TRUE, TRUE, TRUE, TRUE);
		ExSetGLTransparent(window, EX_GLTRANSPARENT_ENABLE);
	}
	else{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glColorMask(TRUE, TRUE, TRUE, FALSE);
		glDisable(GL_ALPHA_TEST);
	}

	/*	*/
	private_start = ExGetHiResTime();
	pretime = ExGetHiResTime();



	if(ifd > 0 ){
		timeval.tv_sec = 0;
		timeval.tv_usec = 1000;
	}else{
		timeval.tv_sec = 0;
		timeval.tv_usec = 0;
	}


	/*	*/
	if(ExIsTexture(&fbackbuffertex)){
		glActiveTexture(GL_TEXTURE0 + numTextures);
		glBindTexture(fbackbuffertex.target, fbackbuffertex.texture);
	}

	/*	Bind all textures.	*/
	for(x = 0; x < numTextures; x++){
		if(ExIsTexture(&textures[x])){
			privatefprintf("Binding texture %d.\n", x);
			glActiveTexture(GL_TEXTURE0 + x);
			glBindTexture(textures[x].target, textures[x].texture);
		}
	}

	/*	*/
	while(isAlive){

		/**/
		while(ExPollEvent(&event)){

			/**/
			if(event.event & EX_EVENT_KEY_PRESSED){
				if(event.key.code == EXK_ENTER && event.key.ctrl){
					fullscreen = ~fullscreen & 0x1;
					ExGLFullScreen(fullscreen, window, 0, NULL);
				}
			}

			/**/
			if(event.event & EX_EVENT_MOUSE_MOTION){
				ExPoint location;
				ExGetMouseState(&location.x, &location.y);
				float mouse[2] = {location.x , -location.y };
				for(x = 0; x < numShaderPass; x++){
					glUniform2fv(uniform[x].mouse, 1, &mouse[0]);
				}
			}

			if( ( event.event & EX_EVENT_RESIZE) || (event.event & EX_EVENT_ON_FOCUSE)  ||  (event.event & EX_EVENT_SIZE) ){
				for(x = 0; x < numShaderPass; x++){
					glslview_resize_screen(&event, &uniform[x], &shader[x], &fbackbuffertex);
				}
			}

			if(event.event & EX_EVENT_ON_FOCUSE){
				for(x = 0; x < numShaderPass; x++){
					glslview_resize_screen(&event, &uniform[x], &shader[x], &fbackbuffertex);
				}
			}

			if(event.event & EX_EVENT_ON_UNFOCUSE){

			}

			if(event.event & EX_EVENT_EXPOSE){
				visable = 1;
			}

			if( ( event.event & EX_EVENT_WINDOW_DESTROYED ) && event.destroy.window == window){
				isAlive = FALSE;
				goto error;
			}

			if( event.event & EX_EVENT_MOUSEWHEEL ){

			}

			if( event.event & EX_EVENT_WINDOW_HIDE){
				printf("window hidden.\n");
				visable = FALSE;
			}
			if(event.event & EX_EVENT_WINDOW_SHOW){
				printf("window showing.\n");
				visable = TRUE;
			}

			if( event.event & EX_EVENT_WINDOW_MOVE){
				for(x = 0; x < numShaderPass; x++){
					if(uniform[x].offset != -1){
						glUniform2i(uniform[x].offset, 0, 0);
					}
				}
			}
		}



		ttime = (float)(( ExGetHiResTime() - private_start) / 1E9);
		deltatime = ExGetHiResTime() - pretime;
		pretime = ExGetHiResTime();
		/*	TODO fix such that its not needed to redefine some code twice for the rendering code section.	*/
		if(ifd != -1){
			fd_set readfd;
			int ret;
			FD_ZERO(&readfd);
			FD_SET(ifd, &readfd);
			ret = select(FD_SETSIZE, &readfd, NULL, NULL, &timeval);

			if(ret < 0){
				privatefprintf("Select failed: %s\n", strerror(errno));
			}
			else if(ret == 0){
				if(visable || renderInBackground){
					glslview_rendergraphic(drawable, shader, uniform, ttime, deltatime);
				}
			}else{
				struct inotify_event ionevent;

				char buffer[EVENT_BUF_LEN];
				/**/
				int nbytes;
				/**/
				while( (nbytes = read(ifd, &ionevent, EVENT_BUF_LEN)) > 0){
					privatefprintf("inotify event fetching.\n");
					read(ifd, &buffer, ionevent.len);

					printf(ionevent.name);
					if(ionevent.mask & IN_MODIFY){

						for(x = 0; x < numFragPaths; x++){
							if(strcmp(ionevent.name, ExGetBaseName( fragPath[x], NULL, 0) ) == 0){
								privatefprintf("Updating %s\n", fragPath[x]);

								ExDeleteShaderProgram(&shader[x]);
								memset(&shader[x], 0, sizeof(shader[0]));

								ExLoadFile((const ExChar*)fragPath[x], (void**)&fragData);
								if(ExLoadShaderv(&shader[x], vertex, (const char*)fragData, NULL, NULL, NULL)){
									/**/
								}

								free(fragData);

								ExGetWindowSizev(window, &size);
								glUseProgram(shader[x].program);
								glslview_update_shader_uniform(&uniform[x], &shader[x], size.width, size.height);
								break;
							}
						}


					}
					if(ionevent.mask & IN_DELETE){
						privatefprintf("File deleted.\n");
					}
				}

			}
		}/*	if fd != -1*/
		else{

			if(visable || renderInBackground){
				glslview_rendergraphic(drawable, shader, uniform, ttime, deltatime);
			}/*	render passes	*/


		}/*	render condition.	*/

	}/*	End of main while loop.	*/


	error:	/*	*/

	privatefprintf("glslview is terminating.\n");
	glslview_terminate();

	if(usingopencl){
		for(x = 0; x < numclframebuffer; x++){
			clReleaseMemObject(clmemframetexture[x]);
		}
		for(x = 0; x < numcltextures; x++){
			clReleaseMemObject(cltextures[x]);
		}
		clReleaseProgram(clprogram);
		clReleaseCommandQueue(clqueue);
		clReleaseContext(clcontext);
	}

	/*	Release OpenGL resources.	*/
	if(ExGetCurrentOpenGLContext()){
		for(x = 0; x < numShaderPass; x++){
			if(glIsProgram(shader[x].program) == GL_TRUE){
				ExDeleteShaderProgram(&shader[x]);
			}
		}
		if( glIsVertexArray(vao) == GL_TRUE){
			glDeleteVertexArrays(1, &vao);
		}
		if( glIsBuffer(vbo) == GL_TRUE ){
			glDeleteBuffers(1, &vbo);
		}

		/*	textures.	*/
		for(x = 0; x < numTextures; x++){
			if(ExIsTexture(&textures[x])){
				ExDeleteTexture(&textures[x]);
			}
		}

		if(ExIsTexture(&fbackbuffertex)){
			ExDeleteTexture(&fbackbuffertex);
		}

		ExDestroyGLContext(window, ExGetCurrentOpenGLContext());
	}
	if(window){
		ExDestroyWindow(window);
	}

	/*	*/
	if(ifd != -1){
		inotify_rm_watch(ifd, wd);
		free(inotifybuf);
		close(ifd);
	}
	ExShutDown();
	return status;
}
