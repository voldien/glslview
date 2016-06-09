#include<ELT/elt.h>
#include<ELT/graphic.h>
#include<getopt.h>
#include<GL/glx.h>
#include<GL/gl.h>

#include<sys/inotify.h>

/**/
const char* vertex = ""
"#version 330 core\n"
"layout(location = 0) in vec3 vertex;\n"
"void main(void){\n"
"gl_Position = vec4(vertex, 1);\n"
"}\n";

/**/
float quad[4][3] = {
		{-1.0f, -1.0f, 0.0f},
		{-1.0f, 1.0f, 0.0f},
		{ 1.0f, -1.0f, 0.0f},
		{ 1.0f,  1.0f, 0.0f},

};

ExWin window;
ExBoolean fullscreen = 0;
ExBoolean verbose = 0;
unsigned int rendererapi = EX_OPENGL_CORE;
unsigned int isAlive = TRUE;
unsigned int ifd = 0;
char* fragPath;


/**/
static int private_readargument(int argc, char** argv, int pre){
	static struct option longoption[] = {
			{"version", 		optional_argument, NULL, 'v'},
			{"Verbose", 		optional_argument, NULL, 'V'},
			{"alpha", 			no_argument, NULL, 'a'},
			{"wallpaper", 		no_argument, NULL, 'w'},
			{"fullscreen", 		no_argument, NULL, 'f'},
			{"vsync", 			no_argument, NULL, 's'},
			{"renderer", 		required_argument, NULL, 'r'},
			{"no-decoration", 	optional_argument, NULL, 'd'},
			{"debug", 			optional_argument, NULL, 'd'},
			{"notify-file", 	optional_argument, NULL, 'n'},
			{"opengl", 			required_argument, NULL, 'g'},
			{"antialiasing", 			required_argument, NULL, 'g'},
			{NULL, NULL, NULL, NULL}
	};

	int c;
	int index;

	if(pre == 0){
		while((c = getopt_long(argc, argv, "vhs:ar:wg:", longoption, &index)) != EOF){
			switch(c){
			case 'v':
				printf("version %d.%d.%d\n", 0, 0, 0);
				return 2;
			case 'V':
				verbose = TRUE;
				break;
			case 'h':
				return 2;
			case 'a':
				ExOpenGLSetAttribute(EX_OPENGL_ALPHA_SIZE, 8);
				fullscreen = TRUE;
				break;
			case 'r':
				if(optarg != NULL){
					if(strcmp(optarg, "opengl") == 0){
						rendererapi = EX_OPENGL;
					}
					if(strcmp(optarg, "openglcore") == 0){
						rendererapi = EX_OPENGL_CORE;
					}
					else if(strcmp(optarg, "opengles") == 0){
						rendererapi = EX_OPENGLES;
					}
				}
				break;
			case 'g':
				if(optarg){
					int len = strlen(optarg);
					if(len > 3){
						continue;
					}
					ExOpenGLSetAttribute(EX_OPENGL_MAJOR_VERSION, atoi(optarg) / 100);
					ExOpenGLSetAttribute(EX_OPENGL_MINOR_VERSION, (atoi(optarg) % 100 ) / 10);
				}
				break;
			case '\?':
			case ':':
			default:
				break;
			}
		}

		fragPath = argv[optind];

	}else if(pre == 1){
		while((c = getopt_long(argc, argv, "vhs:fws", &longoption, &index)) != EOF){
			switch(c){
			case 'f':
				ExGLFullScreen(TRUE, window, 0, NULL);
				break;
			case 'w':
				ExSetWindowParent(ExGetDesktopWindow(), window);
				break;
			case 's':
				ExOpenGLSetVSync(TRUE, ExGetCurrentGLDrawable());
				break;
			case 'n':
				ifd = inotify_init();
				if(inotify_add_watch(ifd, fragPath, IN_MODIFY) < 0){
					exit(EXIT_FAILURE);
				}
				break;
			default:
				break;
			}
		}
	}

	/*	*/
	opterr = 0;
	optind = 0;
	optopt = 0;
	return TRUE;
}

/**/
struct uniform_location_t{
	unsigned int time;
	unsigned int resolution;
	unsigned int deltatime;
	unsigned int mouse;		/*	mouse	*/
	unsigned int offset;
	unsigned int backbuffer;
};


int main(int argc, char** argv){
	ExEvent event = {0};
	struct uniform_location_t uniform = {0};
	ExShader shader = {0};
	ExSize size;

	ExWin drawable;
	ExChar title[512];


	char* fragData = NULL;

	unsigned int vao;
	unsigned int vbo;

	if(argc <= 1){
		return EXIT_FAILURE;
	}

	if(private_readargument(argc, argv, 0) == 2){
		return EXIT_FAILURE;
	}

	if(ExInit(0) == 0){
		return EXIT_FAILURE;
	}

	ExGetPrimaryScreenSize(&size);
	window = ExCreateWindow(size.width / 4, size.height / 4, size.width / 2, size.height / 2, rendererapi);
	if(!window){
		return EXIT_FAILURE;
	}
	ExShowWindow(window);
	ExGetApplicationName(title, sizeof(title));
	ExSetWindowTitle(window, title);
	printf("%d\n", ExGetOpenGLVersion(NULL, NULL));
	ExSetWindowPos(window, size.width / 4, size.height / 4);


	if(private_readargument(argc, argv, 1) == 2){
		return EXIT_FAILURE;
	}



	if(ExLoadFile(fragPath, &fragData) < 0){
		goto error;
	}
	if(ExLoadShaderv(&shader, vertex, fragData, NULL, NULL, NULL) == 0){
		return EXIT_FAILURE;
	}else{
		uniform.time = glGetUniformLocation(shader.program, "time");
		uniform.resolution = glGetUniformLocation(shader.program, "resolution");
		uniform.mouse = glGetUniformLocation(shader.program, "mouse");
		uniform.offset = glGetUniformLocation(shader.program, "offset");
	}


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
	glClearColor(0,0,0,1);

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(FALSE);
	if(ExOpenGLGetAttribute(EX_OPENGL_ALPHA_SIZE, NULL) > 0){
		glEnable(GL_ALPHA_TEST);
		glColorMask(TRUE, TRUE, TRUE, TRUE);
		ExSetGLTransparent(window, EX_GLTRANSPARENT_ENABLE);
	}
	else{
		glColorMask(TRUE, TRUE, TRUE, FALSE);
		glDisable(GL_ALPHA_TEST);
		ExSetGLTransparent(window, EX_GLTRANSPARENT_DISABLE);
	}

	long int private_start = ExGetHiResTime();
	while(isAlive){
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

			if(event.event & EX_EVENT_RESIZE){
				glViewport(0,0,event.size.width, event.size.height);
			}

			if(event.event & EX_EVENT_ON_FOCUSE){
				float resolution[2] = {event.size.width, event.size.height};
				glViewport(0,0,event.size.width, event.size.height);
				glUniform2fv(uniform.resolution, 1, &resolution[0]);
			}

			if(event.event & EX_EVENT_ON_UNFOCUSE){

			}

			if(event.event & EX_EVENT_EXPOSE){

			}
		}
		float time = (float)(( ExGetHiResTime() - private_start) / 1E9);
		glUniform1fv(uniform.time,  1, &time);


		/**/
		glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(quad) / sizeof(quad[0]));
		if(rendererapi & EX_OPENGL){
			ExSwapBuffers(drawable);
		}
		else if(rendererapi == EX_OPENGLES){
			ExSwapEGLBuffer(window);
		}

		glClear(GL_COLOR_BUFFER_BIT);
	}

	error:	/*	*/
	ExDestroyWindow(window);
	ExDeleteShaderProgram(&shader);
	ExQuit();

	return EXIT_SUCCESS;
}
