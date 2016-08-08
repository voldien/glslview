/**

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
#include<getopt.h>
#include<GL/glx.h>
#include<GL/gl.h>
#include<GL/glext.h>
#include<string.h>
#include<errno.h>
#include<unistd.h>

#include<signal.h>
#include<sys/inotify.h>
#include<sys/poll.h>

#include<FreeImage.h>

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

ExWin window;								/*	*/
ExBoolean fullscreen = 0;					/*	*/
ExBoolean verbose = 0;						/*	*/
unsigned int rendererapi = EX_OPENGL_CORE;	/*	*/
unsigned int isAlive = TRUE;				/*	*/
int ifd = 0;								/*	*/
int wd = 0;									/*	*/
char* fragPath = NULL;						/*	*/
char* inotifybuf = NULL;					/*	*/
unsigned int fbo = 0;						/*	*/
ExTexture textures[8];						/*	*/
const int numTextures = sizeof(textures) / sizeof(textures[0]);
int use_stdin_as_buffer = 0;				/*	*/



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

/**/
static int private_readargument(int argc, char** argv, int pre){
	static const struct option longoption[] = {
			{"version", 		no_argument, NULL, 'v'},				/*	application version.	*/
			{"alpha", 			no_argument, NULL, 'a'},				/*	use alpha channel.	*/
			{"wallpaper", 		no_argument, NULL, 'w'},				/*	use as wallpaper.	*/
			{"fullscreen", 		no_argument, NULL, 'f'},				/*	use in fullscreen.	*/
			{"vsync", 			no_argument, NULL, 's'},				/*	enable vsync.	*/
			{"notify-file", 	no_argument, NULL, 'n'},				/*	enable inotify notification.	*/
			{"srgb",			no_argument, NULL, 'S'},				/*	sRGB.	*/
			{"stdin",			no_argument, NULL, 'I'},				/*	stdin data as buffer.	*/
			{"Verbose", 		optional_argument, NULL, 'V'},			/*	Verbose.	*/
			{"no-decoration", 	optional_argument, NULL, 'd'},			/*	*/
			{"debug", 			optional_argument, NULL, 'd'},			/*	set application in debug mode.	*/
			{"antialiasing", 	optional_argument, NULL, 'A'},			/*	anti aliasing.	*/
			{"opengl", 			required_argument, NULL, 'g'},			/*	opengl version.	*/
			{"renderer", 		required_argument, NULL, 'r'},			/*	render api.	*/
			{"resolution-scale",required_argument, NULL, 'R'},			/*	texture scale resolution (required gl_framebuffer_object).*/
			{"texture",			required_argument, NULL, 't'},			/*	texture.	*/


			{NULL, NULL, NULL, NULL}
	};

	int c;
	int index;
	int status = 1;
	const char* shortopts_f = "Ivhs:ar:wg:V";
	const char* shortopts_s = "t:vhs:fwst:nA:";

	/**/
	if(pre == 0){
		while((c = getopt_long(argc, argv, shortopts_f, longoption, &index)) != EOF){
			switch(c){
			case 'v':{
				printf("Version %d.%d.%d\n", 1, 0, 0);
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
				break;
			case 'A':
				ExOpenGLSetAttribute(EX_OPENGL_MULTISAMPLESAMPLES, 0);
				ExOpenGLSetAttribute(EX_OPENGL_MULTISAMPLEBUFFERS, TRUE);
				privatefprintf("Set multisample framebuffer.\n");

				break;
			case 'r':
				if(optarg != NULL){
					if(strcmp(optarg, "opengl") == 0){
						rendererapi = EX_OPENGL;
						privatefprintf("Set rendering api to opengl\n");
					}
					if(strcmp(optarg, "openglcore") == 0){
						rendererapi = EX_OPENGL_CORE;
						privatefprintf("Set rendering api to opengl core\n");
					}
					else if(strcmp(optarg, "opengles") == 0){
						rendererapi = EX_OPENGLES;
						privatefprintf("Set rendering api to opengl-es\n");
					}
				}
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
				use_stdin_as_buffer = True;
				privatefprintf("Stdin used as input buffer for glsl shader.\n");
				break;
			case '\?':
			case ':':
			default:
				break;
			}
		}

		/*	fragment path is the only argument without a option.	*/
		fragPath = argv[optind];

	}else if(pre == 1){
		while((c = getopt_long(argc, argv, shortopts_s, longoption, &index)) != EOF){
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
				}
				break;
			case 'd':	/*	enable opengl debug.	*/
				privatefprintf("Enable opengl debug.\n");
				break;
			case 'f':
				fullscreen = TRUE;
				ExGLFullScreen(TRUE, window, 0, NULL);
				break;
			case 'w':
				ExSetWindowParent(ExGetDesktopWindow(), window);
				break;
			case 's':
				privatefprintf("Enable V-Sync.\n");
				ExOpenGLSetVSync(TRUE, ExGetCurrentGLDrawable());
				break;
			case 'n':{
				char buf[PATH_MAX];
				ifd = inotify_init();
				if(ifd < 0){
					exit(EXIT_FAILURE);
				}
				ExGetAbsolutePath(fragPath, buf, sizeof(buf));
				ExGetDirectory(buf, buf, sizeof(buf));
				if(( wd = inotify_add_watch(ifd, buf, IN_MODIFY)) < 0){
					exit(EXIT_FAILURE);
				}
				privatefprintf("Initialize inotify.\n");
				privatefprintf("Added %s to inotify watch.\n", buf);
				inotifybuf = malloc(4096);
			case 't':
				if(optarg){
					/*
					FIBITMAP bitmap;
					FreeImage_Initialise(0);
					bitmap = FreeImage_Load(FreeImage_GetFileType(optarg), optarg, 0);
					FreeImage_GetBits(bitmap);
					*/
					//ExCreateTexture(&text[0], GL_TEXTURE_2D, 0, GL_RGB, )
				}
				break;
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

	return status;
}

/**
 *
 */
struct uniform_location_t{
	unsigned int time;			/*	time in seconds as float.	*/
	unsigned int resolution;	/*	resolution. */
	unsigned int deltatime;		/*	deltatime.	*/
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
};


void catchSig(int signal){
	switch(signal){
	case SIGINT:
	case SIGQUIT:
		isAlive = FALSE;
		break;
	}
}

int main(int argc, char** argv){
	ERESULT status = EXIT_SUCCESS;

	ExEvent event = {0};					/**/
	ExWin drawable = NULL;					/**/
	struct uniform_location_t uniform = {0};/**/
	ExShader shader = {0};					/**/
	unsigned int isPipe;					/**/


	ExSize size;							/**/
	ExChar title[512];						/**/
	float screen[2];

	char* fragData = NULL;
	int x;

	unsigned int vao;
	unsigned int vbo;
	long int private_start = ExGetHiResTime();
	/**/
	long int pretime;
	float time;

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

	privatefprintf("\n");
	privatefprintf("glslview v%d.%d%d\n========================\n\n", 1, 0, 0);

	/*	initialize ELT.	*/
	if(ExInit(0) == 0){
		status = EXIT_FAILURE;
		goto error;
	}

	/*	*/
	signal(SIGINT, catchSig);
	signal(SIGQUIT, catchSig);
	signal(SIGABRT, catchSig);


	/*	create window */
	ExGetPrimaryScreenSize(&size);
	window = ExCreateWindow(size.width / 4, size.height / 4, size.width / 2, size.height / 2, rendererapi);
	if(!window){
		status = EXIT_FAILURE;
		goto error;
	}

	/**/
	ExShowWindow(window);
	ExGetApplicationName(title, sizeof(title));
	ExSetWindowTitle(window, title);
	printf("-------------- OpenGL Information ------------------\n");
	privatefprintf("Opengl version %d %s\n", ExGetOpenGLVersion(NULL, NULL), "");
	privatefprintf("Opengl shader language version %d\n", ExGetOpenGLShadingVersion());
	ExSetWindowPos(window, size.width / 4, size.height / 4);
	ExSetWindowSize(window, size.width / 2, size.height / 2);


	/**/
	if(private_readargument(argc, argv, 1) == 2){
		status = EXIT_SUCCESS;
		goto error;
	}


	/*	load shader fragment source code.	*/
	privatefprintf("----------- fetching source code----------\n");
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
		if(ExLoadFile((const char*)fragPath, (void**)&fragData) < 0 ){
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
		privatefprintf("----------- fetching uniforms index location ----------\n");
		uniform.time = glGetUniformLocation(shader.program, "time");
		uniform.deltatime = glGetUniformLocation(shader.program, "deltatime");
		uniform.resolution = glGetUniformLocation(shader.program, "resolution");
		uniform.mouse = glGetUniformLocation(shader.program, "mouse");
		uniform.offset = glGetUniformLocation(shader.program, "offset");
		uniform.tex0 = glGetUniformLocation(shader.program, "tex0");
		uniform.tex1 = glGetUniformLocation(shader.program, "tex1");
		uniform.tex2 = glGetUniformLocation(shader.program, "tex2");
		uniform.tex3 = glGetUniformLocation(shader.program, "tex3");
		uniform.tex4 = glGetUniformLocation(shader.program, "tex4");
		uniform.tex5 = glGetUniformLocation(shader.program, "tex5");
		uniform.tex6 = glGetUniformLocation(shader.program, "tex6");
		uniform.tex7 = glGetUniformLocation(shader.program, "tex7");
		uniform.backbuffer = glGetUniformLocation(shader.program, "backbuffer");
		uniform.stdin = glGetUniformLocation(shader.program, "stdin");

	}

	/*	free fragment source.	*/
	free(fragData);
	fragData = NULL;

	/*	generate vertex array for quad.	*/
	privatefprintf("----------- constructing rendering quad. ----------\n");
	ExGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	ExGenBuffers(1, &vbo);

	/**/
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, NULL );

	/**/
	glBindVertexArray(0);



	drawable = ExGetCurrentGLDrawable();
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(FALSE);

	glUseProgram(shader.program);
	glBindVertexArray(vao);

	/*	bind all  textures.	*/
	for(x = 0; x < numTextures; x++){
		glActiveTexture(GL_TEXTURE0 + x);
		glBindTexture(textures[x].target, textures[x].texture);
	}

	/**/
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
		ExSetGLTransparent(window, EX_GLTRANSPARENT_DISABLE);
	}

	/*	*/
	private_start = ExGetHiResTime();
	pretime = ExGetHiResTime();

	/*	*/
	while(isAlive){
		int ret;
		struct pollfd pfd = {ifd, POLLIN, 0};
		/**/
		while(ExPollEvent(&event)){

			if(event.event & EX_EVENT_KEY_PRESSED){
				if(event.key.code == EXK_ENTER && event.key.ctrl){
					fullscreen = ~fullscreen & 0x1;
					ExGLFullScreen(fullscreen, window, 0, NULL);

				}
			}

			if(event.event & EX_EVENT_MOUSE_MOTION){
				float mouse[2] = {event.motion.x / screen[0], event.motion.y / screen[1]};
				glUniform2fv(uniform.mouse, 1, &mouse[0]);
			}

			if( ( event.event & EX_EVENT_RESIZE) || (event.event & EX_EVENT_ON_FOCUSE) ){
				screen[0] = event.size.width;
				screen[1] = event.size.height;
				glViewport(0, 0, event.size.width, event.size.height);
			}

			if(event.event & EX_EVENT_ON_FOCUSE){
				screen[0] = event.size.width;
				screen[1] = event.size.height;
				float resolution[2] = {event.size.width, event.size.height};
				glViewport(0, 0, event.size.width, event.size.height);
				glUniform2fv(uniform.resolution, 1, &resolution[0]);
			}

			if(event.event & EX_EVENT_ON_UNFOCUSE){

			}

			if(event.event & EX_EVENT_EXPOSE){
				screen[0] = event.size.width;
				screen[1] = event.size.height;
				float resolution[2] = {event.size.width, event.size.height};
				glViewport(0,0,event.size.width, event.size.height);
				glUniform2fv(uniform.resolution, 1, &resolution[0]);
			}

			if( ( event.event & EX_EVENT_WINDOW_DESTROYED ) && event.destroy.window == window){
				goto error;
			}

			if( event.event & EX_EVENT_MOUSEWHEEL ){

			}
		}


		/**/
		ret = poll(&pfd, wd, 1);

		if(ret < 0){
			privatefprintf("poll failed: %s\n", strerror(errno));
		}
		else if(ret == 0){
			float time = (float)(( ExGetHiResTime() - private_start) / 1E9);
			glUniform1fv(uniform.time,  1, &time);

			/**/
			if(use_stdin_as_buffer){
				int buffer;
				read(STDIN_FILENO, &buffer, sizeof(buffer));
			}

			/*	draw quad.	*/
			glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(quad) / sizeof(quad[0]));


			/*	TODO improve.	*/
			if(rendererapi & EX_OPENGL){
				ExSwapBuffers(drawable);
			}
			else if(rendererapi == EX_OPENGLES){
				ExSwapEGLBuffer(window);
			}

			glClear(GL_COLOR_BUFFER_BIT);
		}else{
			/**/
			struct inotify_event event;
			int nbytes;
			/**/
			while( (nbytes = read(ifd, &event, sizeof(event))) > 0){

				if(event.mask & IN_MODIFY){
					privatefprintf("update");
					ExDeleteShaderProgram(&shader);
					memset(&shader, 0, sizeof(shader));
					if(ExLoadShaderv(&shader, vertex, fragData, NULL, NULL, NULL)){

					}
					glUseProgram(shader.program);
				}
			}
		}
	}/*	*/

	error:	/*	*/

	privatefprintf("glslview is terminating.\n");

	/**/
	if(ExGetCurrentOpenGLContext()){
		if(glIsProgram(shader.program)){
			ExDeleteShaderProgram(&shader);
		}
		ExDestroyGLContext(window, ExGetCurrentOpenGLContext());
	}
	if(window){
		ExDestroyWindow(window);
	}

	free(inotifybuf);
	inotify_rm_watch(ifd, wd);
	close(ifd);
	ExQuit();
	return status;
}
