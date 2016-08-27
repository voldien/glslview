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

#include<GL/gl.h>
#include<GL/glext.h>

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
static const float quad[4][3] = {
		{-1.0f, -1.0f, 0.0f},
		{-1.0f, 1.0f, 0.0f},
		{ 1.0f, -1.0f, 0.0f},
		{ 1.0f,  1.0f, 0.0f},

};

ExWin window = NULL;							/*	*/
ExBoolean fullscreen = 0;						/*	*/
ExBoolean verbose = 0;							/*	*/
ExBoolean debug = 0;							/*	*/
ExBoolean compression = 0;						/*	*/
unsigned int rendererapi = EX_OPENGL_CORE;		/*	*/
unsigned int isAlive = 1;						/*	*/
int ifd = -1;									/*	*/
int wd = -1;									/*	*/
char* fragPath = NULL;							/*	*/
char* inotifybuf = NULL;						/*	*/
unsigned int fbo = 0;							/*	*/
ExTexture ftex = {0};							/*	*/
ExTexture textures[8] = {{0}};					/*	*/
const int numTextures = sizeof(textures) / sizeof(textures[0]);
unsigned int nextTex = 0;						/*	*/
unsigned int use_stdin_as_buffer = 0;			/*	*/
int stdin_buffer_size = 1;						/*	*/


typedef void (*pswapbufferfunctype)(ExWin window);	/**/
pswapbufferfunctype pswapbuffer;					/**/


/**/
static int privatefprintf(const char* format,...){
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

static int debugprintf(const char* format,...){
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


/**/
static int private_readargument(int argc, const char** argv, int pre){
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
			{"texture-compression",optional_argument, NULL, 'C'},		/*	Texture compression.	*/
			{"file", 			required_argument, NULL, 'f'},			/*	glsl shader file.	*/
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
	const char* shortopts_f = "dIhsar:g:Vf:SA:t:vFwnCp:";		/**/

	/*	*/
	if(pre == 0){
		privatefprintf("--------- First argument pass -------\n\n");
		while((c = getopt_long(argc, (char *const *)argv, shortopts_f, longoption, &index)) != EOF){
			switch(c){
			case 'v':{
				printf("Version %d.%d.%d\n", GLSLVIEW_MAJOR_VERSION, GLSLVIEW_MINOR_VERSION, GLSLVIEW_REVISION_VERSION);
				return (2);
			}
			case 'V':
				verbose = TRUE;
				privatefprintf("Enable verbose.\n");
				break;
			case 'h':{
				return (2);
			}
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
					ExOpenGLSetAttribute(EX_OPENGL_MAJOR_VERSION, atoi(optarg) / 100);
					ExOpenGLSetAttribute(EX_OPENGL_MINOR_VERSION, (atoi(optarg) % 100 ) / 10);
					privatefprintf("Set OpenGL version %d.%d0", atoi(optarg) / 100, (atoi(optarg) % 100) / 10);
				}
				break;
			case 'I':
				use_stdin_as_buffer = 1;
				if(optarg){
					stdin_buffer_size = atoi(optarg);
				}
				privatefprintf("Stdin used as input buffer for glsl shader with read size %d.\n", stdin_buffer_size);
				break;
			case 'f':
				if(optarg){
					fragPath = optarg;
				}
				break;
			case '\?':
			case ':':
			default:
				break;
			}
		}

		/*	fragment path is the only argument without a option.	*/
		if(c == -1 && optind < argc && fragPath == NULL){
			fragPath = (char*)argv[optind];
		}

	}else if(pre == 1){
		privatefprintf("--------- Second argument pass -------\n\n");

		while((c = getopt_long(argc, (char *const *)argv, shortopts_f, longoption, &index)) != EOF){
			switch(c){
			case 'A':
				if(optarg){
					if(strcmp(optarg, "dsr") == 0){
						/**/
						if(ExIsOpenGLExtensionSupported("GL_ARB_framebuffer_object")){
							ExGenFrameBuffers(1, &fbo);
							///glBindFrameBuffers(GL_DRAW_FRAMEBUFFER, fbo);
						}
					}
					else if(strcmp(optarg, "msaa") == 0){

					}

				}
				break;
			case 'd':{	/*	enable debug.	*/
			    debug = 1;
			}break;
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
						privatefprintf("Monitor screen index %d selected for wallpaper.\n", index);
						index = atoi(optarg);
						ExGetScreenRect(index, &rect);
						location.x = rect.x;
						location.y = rect.y;
						size.width = rect.width;
						size.height = rect.height;
					}

					/*	Resize window to fix desktop view.	*/
					privatefprintf("Set view as desktop wallpaper %dx%d.\n", size.width, size.height);
					ExSetWindowSize(window, size.width, size.height);
					ExSetWindowPos(window, location.x, location.y);

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
				privatefprintf("Initialize inotify.\n");

				/*	initialize inotify.	*/
				ifd = inotify_init1(IN_NONBLOCK);
				if(ifd < 0){
					fprintf(stderr, "%s\n", strerror(errno));
					exit(EXIT_FAILURE);
				}

				/*	Get absolute path for inotify watch.	*/
				ExGetAbsolutePath(fragPath, buf, sizeof(buf));
				ExGetDirectory(buf, buf, sizeof(buf));
				if(( wd = inotify_add_watch(ifd, buf, IN_MODIFY | IN_DELETE)) < 0){
					exit(EXIT_FAILURE);
				}

				privatefprintf("Added %s directory to inotify watch.\n\n", buf);
				inotifybuf = malloc(4096);
			case 't':
				if(optarg){
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
	opterr = 0;
	optind = 0;
	optopt = 0;
	//optarg = NULL;

	return status;
}

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


void catchSig(int signal){
	switch(signal){
	case SIGINT:
	case SIGQUIT:
		isAlive = FALSE;
		break;
	}
}


/**
 *
 *
 */
void update_shader_uniform(struct uniform_location_t* uniform, ExShader* shader, int width, int height){
	int x;
	float res[2];

	privatefprintf("----------- fetching uniforms index location ----------\n");
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
	uniform->backbuffer = glGetUniformLocation(shader->program, "backbuffer");

	debugprintf("time %d\n", uniform->time);
	debugprintf("deltatime %d\n", uniform->deltatime);
	debugprintf("resolution %d\n", uniform->resolution);
	debugprintf("mouse %d\n", uniform->mouse);
	debugprintf("offset %d\n", uniform->offset);
	debugprintf("stdin %d\n", uniform->stdin);
	debugprintf("tex0 %d\n", uniform->tex0);
	debugprintf("tex1 %d\n", uniform->tex1);
	debugprintf("tex2 %d\n", uniform->tex2);
	debugprintf("tex3 %d\n", uniform->tex3);
	debugprintf("tex4 %d\n", uniform->tex4);
	debugprintf("tex5 %d\n", uniform->tex5);
	debugprintf("tex6 %d\n", uniform->tex6);
	debugprintf("tex7 %d\n", uniform->tex7);
	debugprintf("backbuffer %d\n", uniform->backbuffer);

	glUseProgram(shader->program);

	res[0] = width;
	res[1] = height;
	glUniform2fv(uniform->resolution, 1, &res[0]);

	privatefprintf("----------- Assigning texture sampler index ----------\n");
	glUniform1i(uniform->tex0, 0);
	glUniform1i(uniform->tex1, 1);
	glUniform1i(uniform->tex2, 2);
	glUniform1i(uniform->tex3, 3);
	glUniform1i(uniform->tex4, 4);
	glUniform1i(uniform->tex5, 5);
	glUniform1i(uniform->tex6, 6);
	glUniform1i(uniform->tex7, 7);
	glUniform1i(uniform->backbuffer, numTextures);


	/*	Create backbuffer.	*/
	if(uniform->backbuffer != -1){
		unsigned int ftextype = GL_FLOAT;
		unsigned int ftexinternalformat = GL_RGBA;
		unsigned int format = ftexinternalformat;
		if(ExIsTexture(&ftex)){
			ExDeleteTexture(&ftex);
		}

		ExCreateTexture(&ftex, GL_TEXTURE_2D,  0, ftexinternalformat, width, height, 0, format, ftextype, NULL);

	}else{
		if(ExIsTexture(&ftex)){
			ExDeleteTexture(&ftex);
		}
	}

}

/**
 *
 *
 */
void resize_screen(ExEvent* event, struct uniform_location_t* uniform, ExShader* shader, ExTexture* ftexture){
	float resolution[2] = {event->size.width, event->size.height};
	glViewport(0, 0, event->size.width, event->size.height);
	glUniform2fv(uniform->resolution, 1, &resolution[0]);
	privatefprintf("%dx%d.\n", event->size.width, event->size.height);

	if(ftexture){
		unsigned int ftextype = GL_FLOAT;
		unsigned int ftexinternalformat = GL_RGBA;
		if(ExIsTexture(&ftex)){
			ExDeleteTexture(&ftex);
			ExCreateTexture(&ftex, GL_TEXTURE_2D,  0, ftexinternalformat, event->size.width, event->size.height, 0, GL_RGB, ftextype, NULL);
		}
	}
}

void displaygraphic(ExWin drawable){
	/*	draw quad.	*/
	glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(quad) / sizeof(quad[0]));

	pswapbuffer(drawable);
}


int main(int argc, const char** argv){
	ERESULT status = EXIT_SUCCESS;				/*	*/

	ExEvent event = {0};						/*	*/
	ExWin drawable = NULL;						/*	*/
	long int private_start = ExGetHiResTime();	/*	*/
	struct uniform_location_t uniform = {0};	/*	uniform.	*/
	ExShader shader = {0};						/*	*/
	unsigned int isPipe;						/*	*/
	long int srclen;							/*	*/

	ExSize size;								/*	*/
	ExChar title[512];							/*	*/

	char* fragData = NULL;						/*	*/
	int x;										/*	*/

	unsigned int vao = 0;						/*	*/
	unsigned int vbo = 0;						/*	*/

	pswapbuffer = ExSwapBuffers;				/*	TODO resolve for EGL or GLX/WGL.	*/

	/**/
	long int pretime;
	long int deltatime;

	/**/
	struct timeval timeval;

	/*	*/
	isPipe = isatty(STDIN_FILENO) == 0;
	if(argc <= 1 && !isPipe){
		fprintf(stderr, "No argument.\n");
		return EXIT_FAILURE;
	}

	/*	*/
	if(private_readargument(argc, argv, 0) == 2){
		return EXIT_SUCCESS;
	}

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
	signal(SIGINT, catchSig);
	signal(SIGQUIT, catchSig);
	signal(SIGABRT, catchSig);


	/*	Create window. */
	ExGetPrimaryScreenSize(&size);
	size.width /= 2;
	size.height /= 2;
	window = ExCreateWindow(size.width / 2, size.height / 2, size.width, size.height, rendererapi);
	if(!window){
		status = EXIT_FAILURE;
		goto error;
	}

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
	if(private_readargument(argc, argv, 1) == 2){
		status = EXIT_FAILURE;
		goto error;
	}


	/*	load shader fragment source code.	*/
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
		srclen = ExLoadFile((const char*)fragPath, (void**)&fragData);

		if( srclen < 0 ){
			status = EXIT_FAILURE;
			goto error;
		}
	}

	/*	compile shader.	*/
	privatefprintf("----------- compiling source code ----------\n");
	if(ExLoadShaderv(&shader, vertex, fragData, NULL, NULL, NULL) == 0){
		fprintf(stderr, "Invalid shader.\n");
		goto error;
		status = EXIT_FAILURE;
	}else{
		update_shader_uniform(&uniform, &shader, size.width, size.height);
	}

	/*	free fragment source.	*/
	free(fragData);
	fragData = NULL;

	/*	generate vertex array for quad.	*/
	privatefprintf("----------- constructing rendering quad. ----------\n");
	ExGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	ExGenBuffers(1, &vbo);

	/*	*/
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, NULL );

	/*	*/
	glBindVertexArray(0);

	/*	*/
	drawable = ExGetCurrentGLDrawable();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(FALSE);

	/*	*/
	ExGetWindowSizev(window, &size);
	glViewport(0, 0, size.width, size.height);

	/*	*/
	glUseProgram(shader.program);
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
	if(ExIsTexture(&ftex)){
		glActiveTexture(GL_TEXTURE0 + numTextures);
		glBindTexture(ftex.target, ftex.texture);
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
				glUniform2fv(uniform.mouse, 1, &mouse[0]);
			}

			if( ( event.event & EX_EVENT_RESIZE) || (event.event & EX_EVENT_ON_FOCUSE)  ||  (event.event & EX_EVENT_SIZE) ){
				resize_screen(&event, &uniform, &shader, &ftex);
			}

			if(event.event & EX_EVENT_ON_FOCUSE){
				resize_screen(&event, &uniform, &shader, &ftex);
			}

			if(event.event & EX_EVENT_ON_UNFOCUSE){

			}

			if(event.event & EX_EVENT_EXPOSE){
			}

			if( ( event.event & EX_EVENT_WINDOW_DESTROYED ) && event.destroy.window == window){
				isAlive = FALSE;
				goto error;
			}

			if( event.event & EX_EVENT_MOUSEWHEEL ){

			}

			if( event.event & EX_EVENT_WINDOW_MOVE){
				if(uniform.offset != -1){
					glUniform2i(uniform.offset, 0, 0);
				}
			}
		}


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

				if(uniform.time != -1){
					float time = (float)(( ExGetHiResTime() - private_start) / 1E9);
					glUniform1fv(uniform.time,  1, &time);
				}
				deltatime = ExGetHiResTime() - pretime;
				pretime = ExGetHiResTime();
				if(uniform.deltatime != -1){
					glUniform1f(uniform.deltatime, (float)((float)deltatime / (float)1E9));
				}

				/*	*/
				if(use_stdin_as_buffer){
					int buffer;
					if(read(STDIN_FILENO, (void*)&buffer, stdin_buffer_size) > 0){
						glUniform1iv(uniform.stdin, sizeof(GLint), (const GLint*)&buffer);
					}
				}

				displaygraphic(drawable);
				if(uniform.backbuffer != -1){
					glActiveTexture(GL_TEXTURE0 + numTextures);
					glBindTexture(ftex.target, ftex.texture);
					glCopyTexImage2D(ftex.target, 0, GL_RGBA, 0, 0, ftex.width, ftex.height, 0);
				}

				glClear(GL_COLOR_BUFFER_BIT);

			}else{
				struct inotify_event ionevent;

				char buffer[EVENT_BUF_LEN];
				/**/
				int nbytes;
				/**/
				while( (nbytes = read(ifd, &ionevent, EVENT_BUF_LEN)) > 0){
					privatefprintf("inotify event fetching.\n");
					read(ifd, &buffer, ionevent.len);

					if(ionevent.mask & IN_MODIFY){

						if(strcmp(ionevent.name, fragPath) == 0){
							privatefprintf("Updating %s\n", ionevent.name);

							ExDeleteShaderProgram(&shader);
							memset(&shader, 0, sizeof(shader));

							ExLoadFile((const ExChar*)ionevent.name, (void**)&fragData);
							if(ExLoadShaderv(&shader, vertex, fragData, NULL, NULL, NULL)){

							}
							free(fragData);

							ExGetWindowSizev(window, &size);
							glUseProgram(shader.program);
							update_shader_uniform(&uniform, &shader, size.width, size.height);

						}
					}
					if(ionevent.mask & IN_DELETE){

					}
				}

			}
		}/*	if fd != -1*/
		else{

			float time = (float)(( ExGetHiResTime() - private_start) / 1E9);
			glUniform1fv(uniform.time,  1, &time);

			/**/
			if(use_stdin_as_buffer){
				int buffer;
				if(read(STDIN_FILENO, (void*)&buffer, stdin_buffer_size) > 0){
					glUniform1iv(uniform.stdin, 4, (const GLint*)&buffer);
				}
			}

			displaygraphic(drawable);
			if(uniform.backbuffer != -1){
				glActiveTexture(GL_TEXTURE0 + numTextures);
				glBindTexture(ftex.target, ftex.texture);
				glCopyTexImage2D(ftex.target, 0, GL_RGBA, 0, 0, ftex.width, ftex.height, 0);
			}

			glClear(GL_COLOR_BUFFER_BIT);
		}

	}/*	End of main while loop.	*/

	error:	/*	*/

	privatefprintf("glslview is terminating.\n");

	/*	Release OpenGL resources.	*/
	if(ExGetCurrentOpenGLContext()){
		if(glIsProgram(shader.program) == GL_TRUE){
			ExDeleteShaderProgram(&shader);
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

		if(ExIsTexture(&ftex)){
			ExDeleteTexture(&ftex);
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
