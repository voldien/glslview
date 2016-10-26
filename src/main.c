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
//#include"internal.h"

#include<GL/gl.h>
#include<GL/glext.h>
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
#include<hpm/hpm.h>
#include<assimp/cimport.h>
#include<assimp/scene.h>
#include<assimp/postprocess.h>


extern int privatefprintf(const char* format,...);
extern int debugprintf(const char* format,...);


/*	only needs to be called once if in polygone mode.	*/
void initmatrix(void){

	if(hpm_supportcpufeat(HPM_AVX2)){
		hpm_init(HPM_AVX2);
	}
	if(hpm_supportcpufeat(HPM_AVX)){
		hpm_init(HPM_AVX);
	}
	if(hpm_supportcpufeat(HPM_SSE2)){
		hpm_init(HPM_SSE2);
	}
	else{
		hpm_init(HPM_NOSIMD);
	}

}

typedef struct mesh_object_t{
	unsigned int vbo;
	unsigned int ibo;
	unsigned int vao;
	unsigned int indicescount;
	union{
		struct{
			hpmvec3f center;
			hpmvec3f size;
		};
		hpmvec3f aabb[2];
	};
}Mesh;

void loadpolygone(const char* cfilename, struct mesh_object_t* pmesh){
	const struct aiScene* scene;
	struct aiMesh* mesh;

	void* passimplib;	/*	for loading assimp library in runetime.	*/

	float* vertex;
	unsigned int* indices;

	unsigned int vao;
	unsigned int vbo;
	unsigned int ibo;

	unsigned int totalVerticesCount = 0;
	unsigned int totalIndicesCount = 0;
	unsigned int offsetVertices = 0;
	unsigned int offsetIndices = 0;
	unsigned int stride;
	unsigned int nfloats;
	unsigned int verticebuffersize;
	unsigned int indicesbuffersize;
	unsigned int nvertexfloats = 0;
	hpmvec3f max = {0};
	hpmvec3f min = {0};

	int x;
	int y;
	int z;
	scene = aiImportFile(cfilename, aiProcessPreset_TargetRealtime_Quality);

	if(scene == NULL){
		fprintf(stderr, aiGetErrorString());
		return;
	}

	/*	*/

	privatefprintf("Mesh count %d\n", scene->mNumMeshes);
	for(x = 0; x < scene->mNumMeshes; x++){
		mesh = scene->mMeshes[x];
		if(mesh){
			if(mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE){
				totalVerticesCount += mesh->mNumVertices;
				totalIndicesCount += mesh->mNumFaces;
			}
			else{
				debugprintf("excluded.\n");
			}
		}
	}
	totalIndicesCount *= 3;
	nfloats = (3 + 2 + 3 + 3 + 3);
	stride = nfloats * sizeof(float);
	privatefprintf("Total vertices count %d\n", totalVerticesCount);
	privatefprintf("Total indices count %d\n", totalIndicesCount);
	privatefprintf("Stride value %d\n", stride);


	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/*	*/
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void*)0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void*)12);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (const void*)20);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (const void*)32);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, stride, (const void*)44);

	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	verticebuffersize = totalVerticesCount * stride;
	indicesbuffersize = totalIndicesCount * sizeof(unsigned int);
	debugprintf("Allocating %d bytes for vertex buffer.\n", verticebuffersize);
	debugprintf("Allocating %d bytes for indices buffer.\n", indicesbuffersize);
	debugprintf("Process mesh %s.\n", mesh->mName.data);
	glBufferData(GL_ARRAY_BUFFER, verticebuffersize, NULL, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesbuffersize, NULL, GL_STATIC_DRAW);


	vertex = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	indices = (unsigned int*)glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);

	assert(vertex);
	assert(indices);

	for(x = 0; x < scene->mNumMeshes; x++){
		mesh = scene->mMeshes[x];
		debugprintf("Process mesh %s. v : %d, i : %d\n", mesh->mName.data, mesh->mNumVertices, mesh->mNumFaces);

		if(mesh->mPrimitiveTypes != aiPrimitiveType_TRIANGLE){
			continue;
		}

		for(y = 0; y < mesh->mNumVertices; y++){
			memcpy(&vertex[nvertexfloats + y * nfloats + 0], &mesh->mVertices[y], sizeof(struct aiVector3D));
			memcpy(&vertex[nvertexfloats + y * nfloats + 3], &mesh->mTextureCoords[0][y], sizeof(struct aiVector2D));
			memcpy(&vertex[nvertexfloats + y * nfloats + 5], &mesh->mNormals[y], sizeof(struct aiVector3D));
			memcpy(&vertex[nvertexfloats + y * nfloats + 8], &mesh->mTangents[y], sizeof(struct aiVector3D));
			memcpy(&vertex[nvertexfloats + y * nfloats + 11], &mesh->mBitangents[y], sizeof(struct aiVector3D));
		}

		for(z = 0; z < mesh->mNumFaces; z++){
			struct aiFace* face = &mesh->mFaces[z];
			GLuint f0 = face->mIndices[0] + offsetVertices;
			GLuint f1 = face->mIndices[1] + offsetVertices;
			GLuint f2 = face->mIndices[2] + offsetVertices;
			assert(face->mNumIndices == 3);		// Check if Indices Count is 3 other case error
			memcpy(&indices[(offsetIndices) + 3 * z + 0], &f0, sizeof(GLuint));
			memcpy(&indices[(offsetIndices) + 3 * z + 1], &f1, sizeof(GLuint));
			memcpy(&indices[(offsetIndices) + 3 * z + 2], &f2, sizeof(GLuint));
		}

		offsetVertices += mesh->mNumVertices;
		offsetIndices += mesh->mNumFaces * 3;
		nvertexfloats += mesh->mNumVertices * nfloats;
	}


	glUnmapBuffer(GL_ARRAY_BUFFER);
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	glBindVertexArray(0);
	pmesh->indicescount = totalIndicesCount;


	/*	*/
	debugprintf("Releasing assimp scene data.\n");
	aiReleaseImport(scene);
}


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


const char* vertexpolygone = ""
"#version 330 core\n"
"layout(location = 0) in vec3 vertex;\n"
"layout(location = 1) in vec2 texturecoord;\n"
"layout(location = 2) in vec3 normal;\n"
"layout(location = 3) in vec3 tangent;\n"
"uniform mat4 mvp;\n"
"uniform mat4 model;\n"
"smooth out vec3 mvertex;\n"
"smooth out vec2 muv;\n"
"smooth out vec3 mnormal;\n"
"smooth out vec3 mtangent;\n"
"void main(void){\n"
"gl_Position = mvp * vec4(vertex,1.0);\n"
"mvertex = (mvp * vec4(vertex,1.0)).xyz;\n"
"muv = texturecoord;\n"
"mnormal = (model * vec4(normal,0.0)).xyz;\n"
"mtangent = (model * vec4(tangent,0.0)).xyz;\n"
"}\n";


/**/
static const float quad[4][3] = {
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
char* fragPath[32] = {NULL};					/*	Path of fragment shader.	*/
unsigned int numgeoPaths = 0;					/*	num.	*/
char* geoPath[32] = {NULL};						/*	Path of geometry shader.	*/
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
unsigned int usepolygone = 0;
Mesh mesh;
typedef void (*pswapbufferfunctype)(ExWin window);	/*	Function pointer data type.	*/
pswapbufferfunctype pswapbuffer;					/*	Function pointer for swap default framebuffer.	*/





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

/**
 *	GLSL uniform data struct.
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
	unsigned int tex8;			/*	texture 8.	*/
	unsigned int tex9;			/*	texture 9.	*/
	unsigned int tex10;			/*	texture 10.	*/
	unsigned int tex11;			/*	texture 11.	*/
	unsigned int tex12;			/*	texture 12.	*/
	unsigned int tex13;			/*	texture 13.	*/
	unsigned int tex14;			/*	texture 14.	*/
	unsigned int tex15;			/*	texture 15.	*/
	unsigned int mvp;			/*	model view projection matrix.	*/
	unsigned int model;			/*	world space matrix.	*/
	unsigned int perspective;	/*	perspective matrix.	*/
	unsigned int view;			/*	camera space matrix.	*/
}UniformLocation;

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
			{"file", 			required_argument, NULL, 'f'},			/*	glsl fragment shader file.	*/
			{"geometyshader",	required_argument, NULL, 'G'},			/*	geometry glsl shader file.	*/
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
	const char* shortopts = "dIhsar:g:Vf:SA:t:vFnCp:w";

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
			case 'c':	/*	TODO create opencl context.	*/
				break;
			case 'p':
				if(optarg){
					usepolygone = 1;
					initmatrix();
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
				break;
			case 'p':
				if(optarg){
					privatefprintf("Attempting to load polygone model %s.\n", optarg);
					loadpolygone(optarg, &mesh);
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


/**
 *
 *
 */
void glslview_update_shader_uniform(UniformLocation* uniform, ExShader* shader, int width, int height){
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
	uniform->tex8 = glGetUniformLocation(shader->program, "tex8");
	uniform->tex9 = glGetUniformLocation(shader->program, "tex9");
	uniform->tex10 = glGetUniformLocation(shader->program, "tex10");
	uniform->tex11 = glGetUniformLocation(shader->program, "tex11");
	uniform->tex12 = glGetUniformLocation(shader->program, "tex12");
	uniform->tex13 = glGetUniformLocation(shader->program, "tex13");
	uniform->tex14 = glGetUniformLocation(shader->program, "tex14");
	uniform->tex15 = glGetUniformLocation(shader->program, "tex15");
	uniform->backbuffer = glGetUniformLocation(shader->program, "backbuffer");
	uniform->mvp = glGetUniformLocation(shader->program, "mvp");
	uniform->model = glGetUniformLocation(shader->program, "model");
	uniform->view = glGetUniformLocation(shader->program, "view");
	uniform->perspective = glGetUniformLocation(shader->program, "perspective");


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
	debugprintf("tex8 %d\n", uniform->tex8);
	debugprintf("tex9 %d\n", uniform->tex9);
	debugprintf("tex10 %d\n", uniform->tex10);
	debugprintf("tex11 %d\n", uniform->tex11);
	debugprintf("tex12 %d\n", uniform->tex12);
	debugprintf("tex13 %d\n", uniform->tex13);
	debugprintf("tex14 %d\n", uniform->tex14);
	debugprintf("tex15 %d\n", uniform->tex15);
	debugprintf("mvp %d\n", uniform->mvp);
	debugprintf("model %d\n", uniform->model);
	debugprintf("view %d\n", uniform->view);
	debugprintf("perspective %d\n", uniform->perspective);
	debugprintf("backbuffer %d\n", uniform->backbuffer);

	glUseProgram(shader->program);

	res[0] = width;
	res[1] = height;
	glUniform2fv(uniform->resolution, 1, &res[0]);

	/**/
	privatefprintf("----------- Assigning texture sampler index ----------\n");
	glUniform1i(uniform->tex0, 0);
	glUniform1i(uniform->tex1, 1);
	glUniform1i(uniform->tex2, 2);
	glUniform1i(uniform->tex3, 3);
	glUniform1i(uniform->tex4, 4);
	glUniform1i(uniform->tex5, 5);
	glUniform1i(uniform->tex6, 6);
	glUniform1i(uniform->tex7, 7);
	glUniform1i(uniform->tex8, 8);
	glUniform1i(uniform->tex9, 9);
	glUniform1i(uniform->tex10, 10);
	glUniform1i(uniform->tex11, 11);
	glUniform1i(uniform->tex12, 12);
	glUniform1i(uniform->tex13, 13);
	glUniform1i(uniform->tex14, 14);
	glUniform1i(uniform->tex15, 15);
	glUniform1i(uniform->backbuffer, numTextures);


	/*	Create backbuffer.	*/
	if(uniform->backbuffer != -1){
		if(ExIsTexture(&fbackbuffertex)){
			ExDeleteTexture(&fbackbuffertex);
		}
		ExCreateTexture(&fbackbuffertex, GL_TEXTURE_2D,  0, ftexinternalformat, width, height, 0, ftexformat, ftextype, NULL);
		privatefprintf("Created backbuffer.	\n");
	}else{
		if(ExIsTexture(&fbackbuffertex)){
			ExDeleteTexture(&fbackbuffertex);
		}
	}

}

/**
 *
 *
 */
void glslview_resize_screen(ExEvent* event, UniformLocation* uniform, ExShader* shader, ExTexture* ftexture){
	float resolution[2] = {event->size.width, event->size.height};
	glViewport(0, 0, event->size.width, event->size.height);
	glUniform2fv(uniform->resolution, 1, &resolution[0]);
	privatefprintf("%dx%d.\n", event->size.width, event->size.height);


	if(ftexture){
		if(ExIsTexture(&fbackbuffertex)){
			ExDeleteTexture(&fbackbuffertex);
			ExCreateTexture(&fbackbuffertex, GL_TEXTURE_2D,  0, ftexinternalformat, event->size.width, event->size.height, 0, ftexformat, ftextype, NULL);
		}
	}
}

/**
 *
 */
void glslview_displaygraphic(ExWin drawable){


	if(usepolygone){
		glBindVertexArray(mesh.vao);
		glDrawElements(GL_TRIANGLES, mesh.indicescount, GL_UNSIGNED_INT, NULL);
		glBindVertexArray(0);
	}
	else{
		/*	draw quad.	*/
		glDrawArrays(GL_TRIANGLE_STRIP, 0, sizeof(quad) / sizeof(quad[0]));
	}
	/**/
	pswapbuffer(drawable);
}


void glslview_terminate(void){

}






int main(int argc, const char** argv){
	ERESULT status = EXIT_SUCCESS;				/*	*/

	ExEvent event = {0};						/*	*/
	ExWin drawable = NULL;						/*	*/

	struct uniform_location_t uniform[32] = {{0}};	/*	uniform.	*/
	ExShader shader[32] = {{0}};						/*	*/
	unsigned int numShaderPass = 0;					/*	*/
	unsigned int isPipe;						/*	*/
	long int srclen;							/*	*/

	ExSize size;								/*	*/
	ExChar title[512];							/*	*/

	char* fragData = NULL;						/*	*/
	int x;										/*	*/

	/*	quad buffer.	*/
	unsigned int vao = 0;						/*	*/
	unsigned int vbo = 0;						/*	*/
	hpmvec4x4f_t model;							/*	world space matrix.	*/
	hpmvec4x4f_t view;							/*	camera space matrix.	*/
	hpmvec4x4f_t perspective;					/*	perspective matrix.	*/
	hpmvec4x4f_t mvp;
	hpmvec4x4f_t tmpmat;
	hpmvec3f campos = {0.0f};							/*	camera position.	*/
	hpmquatf camrotate;							/*	camera rotation as a quaternion.	*/
	hpmquatf rotate;

	pswapbuffer = ExSwapBuffers;				/*	TODO resolve for EGL or GLX/WGL.	*/




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

	/*	*/
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


	if(usepolygone){
		hpm_quat_identityfv(&camrotate);
		hpm_mat4x4_identityfv(view);
		hpm_mat4x4_identityfv(model);
		hpm_mat4x4_projfv(perspective, HPM_RAD2DEG(90.0f), (float)size.width / (float)size.height, 0.15f, 1000.0f);
	}


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

				if(usepolygone){
					/*	update the perspective.	*/
					hpm_mat4x4_projfv(perspective, HPM_RAD2DEG(90.0f),
							(float)event.size.width / (float)event.size.height,
							0.15f, 1000.0f);
				}
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


		if(usepolygone){
			hpm_mat4x4_multiply_mat4x4fv(perspective, view, tmpmat);
			hpm_mat4x4_multiply_mat4x4fv(tmpmat, model, mvp);
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
				if(visable || renderInBackground){
					for(x = 0; x < numShaderPass; x++){
						glUseProgram(shader[x].program);

						if(uniform[x].time != -1){
							float time = (float)(( ExGetHiResTime() - private_start) / 1E9);
							glUniform1fv(uniform[x].time,  1, &time);
						}
						deltatime = ExGetHiResTime() - pretime;
						pretime = ExGetHiResTime();
						if(uniform[x].deltatime != -1){
							glUniform1f(uniform[x].deltatime, (float)((float)deltatime / (float)1E9));
						}

						/*	*/
						if(use_stdin_as_buffer){
							int buffer;
							if(read(STDIN_FILENO, (void*)&buffer, stdin_buffer_size) > 0){
								glUniform1iv(uniform[x].stdin, sizeof(GLint), (const GLint*)&buffer);
							}
						}


						glslview_displaygraphic(drawable);


						if(uniform[x].backbuffer != -1){
							glActiveTexture(GL_TEXTURE0 + numTextures);
							glBindTexture(fbackbuffertex.target, fbackbuffertex.texture);
							glCopyTexImage2D(fbackbuffertex.target, 0, GL_RGBA, 0, 0, fbackbuffertex.width, fbackbuffertex.height, 0);
						}
					}

					glClear(GL_COLOR_BUFFER_BIT);
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
				for(x = 0; x < numShaderPass; x++){
					float time = (float)(( ExGetHiResTime() - private_start) / 1E9);
					glUniform1fv(uniform[x].time,  1, &time);
					/**/
					if(use_stdin_as_buffer){
						int buffer;
						if(read(STDIN_FILENO, (void*)&buffer, stdin_buffer_size) > 0){
							glUniform1iv(uniform[x].stdin, 4, (const GLint*)&buffer);
						}
					}

					glslview_displaygraphic(drawable);

					/*	*/
					if(uniform[x].backbuffer != -1){
						glActiveTexture(GL_TEXTURE0 + numTextures);
						glBindTexture(fbackbuffertex.target, fbackbuffertex.texture);
						glCopyTexImage2D(fbackbuffertex.target, 0, GL_RGBA, 0, 0, fbackbuffertex.width, fbackbuffertex.height, 0);
					}

					glClear(GL_COLOR_BUFFER_BIT);

				}/*	render passes	*/

			}/*	render condition.	*/
		}

	}/*	End of main while loop.	*/


	error:	/*	*/

	privatefprintf("glslview is terminating.\n");
	glslview_terminate();


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
