#include<ELT/elt.h>
#include<ELT/graphic.h>
#include<getopt.h>
#include<GL/glx.h>
#include<GL/gl.h>
#include<string.h>
#include<errno.h>

#include<signal.h>
#include<sys/inotify.h>
#include<sys/poll.h>

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
int ifd = 0;						/*	*/
int wd;
char* fragPath;								/*	*/
char* inotifybuf = NULL;					/*	*/
unsigned int fbo;



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

	if(pre == 0){
		while((c = getopt_long(argc, argv, "vhs:ar:wg:V", longoption, &index)) != EOF){
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
			case '\?':
			case ':':
			default:
				break;
			}
		}

		/*	frag path is the only argument without a option.	*/
		fragPath = argv[optind];

	}else if(pre == 1){
		while((c = getopt_long(argc, argv, "vhs:fwst:nA:", &longoption, &index)) != EOF){
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
			}break;
			default:
				break;
			}
		}
	}

	/*	Reset getopt.	*/
	opterr = 0;
	optind = 0;
	optopt = 0;
	return TRUE;
}


/**/
struct uniform_location_t{
	unsigned int time;			/*	time in seconds as float.	*/
	unsigned int resolution;	/*	resolution. */
	unsigned int deltatime;		/*	deltatime.	*/
	unsigned int mouse;			/*	mouse.	*/
	unsigned int offset;		/*	offset.	*/
	unsigned int backbuffer;	/*	previous buffer.	*/
	unsigned int input;			/*	stdout input data.	*/
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
	ExEvent event = {0};
	ExWin drawable = NULL;
	struct uniform_location_t uniform = {0};
	ExShader shader = {0};

	ExSize size;
	ExChar title[512];

	char* fragData = NULL;

	unsigned int vao;
	unsigned int vbo;

	/*	*/
	if(argc <= 1){
		return EXIT_FAILURE;
	}

	/*	*/
	if(private_readargument(argc, argv, 0) == 2){
		return EXIT_SUCCESS;
	}

	/*	initialize ELT.	*/
	if(ExInit(0) == 0){
		status = EXIT_FAILURE;
		goto error;
	}

	/*	*/
	signal(SIGINT, catchSig);
	signal(SIGQUIT, catchSig);

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
	privatefprintf("%d\n", ExGetOpenGLVersion(NULL, NULL));
	ExSetWindowPos(window, size.width / 4, size.height / 4);


	if(private_readargument(argc, argv, 1) == 2){
		status = EXIT_SUCCESS;
		goto error;
	}


	if(ExLoadFile(fragPath, &fragData) < 0){
		goto error;
	}
	if(ExLoadShaderv(&shader, vertex, fragData, NULL, NULL, NULL) == 0){
		return EXIT_FAILURE;
	}else{
		uniform.time = glGetUniformLocation(shader.program, "time");
		uniform.deltatime = glGetUniformLocation(shader.program, "deltatime");
		uniform.resolution = glGetUniformLocation(shader.program, "resolution");
		uniform.mouse = glGetUniformLocation(shader.program, "mouse");
		uniform.offset = glGetUniformLocation(shader.program, "offset");
	}

	/*	generate vertex array for quad.	*/
	ExGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	ExGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, NULL );
	glBindVertexArray(0);


	glUseProgram(shader.program);
	glBindVertexArray(vao);
	drawable = ExGetCurrentGLDrawable();


	/*	set state of the opengl rendering pipeline.	*/
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(FALSE);
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

	long int private_start = ExGetHiResTime();
	long int pretime = ExGetHiResTime();

	/**/
	while(isAlive == TRUE){
		struct pollfd pfd = {ifd, POLLIN, 0};
		while(ExPollEvent(&event)){

			if(event.event & EX_EVENT_KEY_PRESSED){
				if(event.key.code == EXK_ENTER && event.key.ctrl){
					fullscreen = ~fullscreen & 0x1;
					ExGLFullScreen(fullscreen, window, 0, NULL);
				}
			}

			if(event.event & EX_EVENT_MOUSE_MOTION){
				float mouse[2] = {event.motion.x, event.motion.y};
				glUniform2fv(uniform.mouse, 1, &mouse[0]);
			}

			if( ( event.event & EX_EVENT_RESIZE) || (event.event & EX_EVENT_ON_FOCUSE) ){
				float resolution[2] = {event.size.width, event.size.height};
				glViewport(0, 0, event.size.width, event.size.height);
				glUniform2fv(uniform.resolution, 1, &resolution[0]);
			}

			if(event.event & EX_EVENT_ON_UNFOCUSE){

			}

			if(event.event & EX_EVENT_EXPOSE){

			}

			if( ( event.event & EX_EVENT_WINDOW_DESTROYED ) && event.destroy.window == window){
				goto error;
			}
		}


		int ret = poll(&pfd, 1, 1);
		if(ret < 0){
			privatefprintf("poll failed: %s\n", strerror(errno));
		}
		else if(ret == 0){
			float time = (float)(( ExGetHiResTime() - private_start) / 1E9);
			glUniform1fv(uniform.time,  1, &time);


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
			struct inotify_event event;
			int nbytes;
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
	}


	error:	/*	*/

	privatefprintf("glslview is terminating.\n");
	ExDestroyGLContext(window, ExGetCurrentOpenGLContext());
	ExDestroyWindow(window);
	ExDeleteShaderProgram(&shader);
	free(inotifybuf);
	inotify_rm_watch(ifd, wd);
	close(ifd);
	ExQuit();

	return status;
}
