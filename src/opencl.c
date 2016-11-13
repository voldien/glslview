#include"internal.h"
#include<SDL2/SDL.h>
#include<SDL2/SDL_syswm.h>
#include<math.h>
#include<assert.h>
#include<string.h>
#include<GL/gl.h>
#include<CL/cl.h>
#include<CL/cl_gl.h>


size_t global_work_offset[2] = {0,0};
size_t globalsize[2] = {128, 128};
size_t localsize[2] = { 8, 8 };

const char* get_cl_error_str(unsigned int errorcode){
	static const char* errorString[] = {
		"CL_SUCCESS",
		"CL_DEVICE_NOT_FOUND",
		"CL_DEVICE_NOT_AVAILABLE",
		"CL_COMPILER_NOT_AVAILABLE",
		"CL_MEM_OBJECT_ALLOCATION_FAILURE",
		"CL_OUT_OF_RESOURCES",
		"CL_OUT_OF_HOST_MEMORY",
		"CL_PROFILING_INFO_NOT_AVAILABLE",
		"CL_MEM_COPY_OVERLAP",
		"CL_IMAGE_FORMAT_MISMATCH",
		"CL_IMAGE_FORMAT_NOT_SUPPORTED",
		"CL_BUILD_PROGRAM_FAILURE",
		"CL_MAP_FAILURE",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"CL_INVALID_VALUE",
		"CL_INVALID_DEVICE_TYPE",
		"CL_INVALID_PLATFORM",
		"CL_INVALID_DEVICE",
		"CL_INVALID_CONTEXT",
		"CL_INVALID_QUEUE_PROPERTIES",
		"CL_INVALID_COMMAND_QUEUE",
		"CL_INVALID_HOST_PTR",
		"CL_INVALID_MEM_OBJECT",
		"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
		"CL_INVALID_IMAGE_SIZE",
		"CL_INVALID_SAMPLER",
		"CL_INVALID_BINARY",
		"CL_INVALID_BUILD_OPTIONS",
		"CL_INVALID_PROGRAM",
		"CL_INVALID_PROGRAM_EXECUTABLE",
		"CL_INVALID_KERNEL_NAME",
		"CL_INVALID_KERNEL_DEFINITION",
		"CL_INVALID_KERNEL",
		"CL_INVALID_ARG_INDEX",
		"CL_INVALID_ARG_VALUE",
		"CL_INVALID_ARG_SIZE",
		"CL_INVALID_KERNEL_ARGS",
		"CL_INVALID_WORK_DIMENSION",
		"CL_INVALID_WORK_GROUP_SIZE",
		"CL_INVALID_WORK_ITEM_SIZE",
		"CL_INVALID_GLOBAL_OFFSET",
		"CL_INVALID_EVENT_WAIT_LIST",
		"CL_INVALID_EVENT",
		"CL_INVALID_OPERATION",
		"CL_INVALID_GL_OBJECT",
		"CL_INVALID_BUFFER_SIZE",
		"CL_INVALID_MIP_LEVEL",
		"CL_INVALID_GLOBAL_WORK_SIZE",
	};

	/*	compute error index code. 	*/
	const int errorCount = sizeof(errorString) / sizeof(errorString[0]);
	const int index = -errorcode;

	/*	return error string.	*/
	return (index >= 0 && index < errorCount) ? errorString[index] : "Unspecified Error";
}


/**/
cl_context glslview_createclcontext(void* shared, unsigned int* numDevices, cl_device_id** device){
	cl_int err;
	cl_context context;
	cl_platform_id* platforms;
	cl_device_id* devices = NULL;
	cl_device_id curgldevice;
	int x = 0;
	size_t i;
	SDL_SysWMinfo info;
	SDL_VERSION(&info.version);
	SDL_GetWindowWMInfo(window, &info);

	assert(shared);

	/**/
	cl_context_properties props[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)NULL,
			CL_GL_CONTEXT_KHR,   (cl_context_properties)shared,
			CL_GLX_DISPLAY_KHR,     (cl_context_properties)info.info.x11.display,
			0
	};

	unsigned int nDevices = 0;
	unsigned int nPlatforms = 0;
	unsigned int nselectPlatform = 0;

	/*	get platform id.	*/
	err = clGetPlatformIDs(0, NULL, &nPlatforms);
	if(nPlatforms == 0){
		fprintf(stderr, "%s\n", get_cl_error_str(err));
		exit(EXIT_FAILURE);
	}
	platforms = malloc(sizeof(*platforms) * nPlatforms);
	err = clGetPlatformIDs(nPlatforms, platforms, NULL);


	/*	iterate */
	for(x = 0; x < nPlatforms; x++){
		props[1] = (cl_context_properties)platforms[x];
		size_t bytes = 0;

		/*	queuring how much bytes we need to read	*/
		err = clGetGLContextInfoKHR(props, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, 0, NULL, &bytes);
		if(err != CL_SUCCESS){
			fprintf(stderr, "%s\n", get_cl_error_str(err));
		}

		// allocating the mem
		size_t devNum = bytes/sizeof(cl_device_id);
		devices = (cl_device_id*)realloc(devices, bytes + nDevices * sizeof(cl_device_id));

		/**/
		err = clGetGLContextInfoKHR(props, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, bytes, &devices[nDevices], NULL);
		if(err != CL_SUCCESS){
			fprintf(stderr, "%s\n", get_cl_error_str(err));
		}

		nDevices += devNum;

		/*	iterate over all devices	*/
		for(i = 0; i < devNum; i++){
		      /*	enumerating the devices for the type, names, CL_DEVICE_EXTENSIONS, etc	*/
		}

	}
	if(nDevices < 1){
		fprintf(stderr, "No devices.\n");
	}


	/*	create context.	*/
	props[1] = (cl_context_properties)platforms[nselectPlatform];
	context = clCreateContext(props, nDevices, devices, NULL, NULL, &err);
	if(context == NULL){
		fprintf(stderr, "Failed to create OpenCL context. %d\n  [ %s ]", err, get_cl_error_str(err));
	}

	if(device){
		*device = devices;
	}
	if(numDevices){
		*numDevices = nDevices;
	}

	free(platforms);
	return context;
}


void glslview_clrelease(void){
	unsigned int x;


	clFlush(clqueue);
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
}


/*	Create OpenCL program.	*/
cl_program glslview_createProgram(cl_context context, unsigned int nDevices, cl_device_id* device, const char* cfilename){
	cl_int ciErrNum;
	cl_program program;
	char* source;
	FILE* f;
	long int flen;
	f = fopen(cfilename, "rb");
	fseek(f, 0, SEEK_END);
	flen = ftell(f);
	fseek(f, SEEK_SET, 0);
	source = (char*)malloc(flen);
	fread(source, 1, flen, f);
	fclose(f);

	program = clCreateProgramWithSource(context, 1, (const char **)&source, NULL, &ciErrNum);

	if(program == NULL || ciErrNum != CL_SUCCESS){
		fprintf(stderr, "Failed to create program %d %s\n", ciErrNum, get_cl_error_str(ciErrNum));
	}


	ciErrNum = clBuildProgram(program, nDevices, device, "-cl-kernel-arg-info", NULL, NULL);
	if(ciErrNum != CL_SUCCESS){
		if(ciErrNum == CL_BUILD_PROGRAM_FAILURE){
			size_t build_log_size = 900;
			char build_log[900];
			size_t build_log_ret;

			ciErrNum =  clGetProgramBuildInfo(program, device[0], CL_PROGRAM_BUILD_LOG, build_log_size, build_log, &build_log_ret);
			fprintf(stderr, build_log );
		}
	}
	free(source);
	return program;
}

cl_command_queue glslview_createcommandqueue(cl_context context, cl_device_id device){
	cl_int error;
	cl_command_queue queue;
	cl_command_queue_properties pro = 0;
	queue = clCreateCommandQueue(context,
			device,
			pro,
			&error);
	/**/
	if(error != CL_SUCCESS){
		fprintf(stderr, "Failed to create command queue . %d \n", error);
	}

	return queue;
}


cl_context glslview_createCLContext(void* shared, unsigned int* ncldevices, cl_device_id** devices){
	cl_command_queue queue;
	cl_context context;
	cl_platform_id platform;
	cl_int err;
	int i;

	assert(shared);

	context = glslview_createclcontext(shared, ncldevices, devices);
	if(context == NULL){
		return NULL;
	}


	queue = glslview_createcommandqueue(context, (*devices)[0]);
	clqueue = queue;


	return context;
}

cl_program glslview_createCLProgram(cl_context context, unsigned int nNumDevices, cl_device_id* id, const char* cfilename, UniformLocation* uniform){
	cl_program program;
	cl_kernel kernel;
	cl_mem texmem;
	cl_int err;
	int x;
	int y;
	unsigned int texind = 0;
	int kerneltexindex;
	cl_uint numKernelArgs;
	char argname[256];
	size_t argnamesize;
	size_t argtype;
	char argtypename[256];
	/*	framebuffer image view attributes information.	*/
	const unsigned int w = 1920 / 2;
	const unsigned int h = 1080 / 2;


	program = glslview_createProgram(context, nNumDevices, id, cfilename);
	assert(program);
	kernel = clCreateKernel(program, "main", &err);
	clkernel = kernel;
	if(err != CL_SUCCESS){
		fprintf(stderr, "Failed to create kernel %d %s\n", err, get_cl_error_str(err));
		return NULL;
	}




	memset(uniform, -1, sizeof(UniformLocation));
	/*	iterate through all the argument.	*/
	kerneltexindex = 0;
	err = 0;
	err = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(numKernelArgs), &numKernelArgs, NULL);
	for(x = kerneltexindex; x < numKernelArgs; x++){

		err = clGetKernelArgInfo(kernel, x, CL_KERNEL_ARG_TYPE_NAME, 0, NULL, &argtype);
		err = clGetKernelArgInfo(kernel, x, CL_KERNEL_ARG_TYPE_NAME, argtype, &argtypename[0], NULL);
		err = clGetKernelArgInfo(kernel, x, CL_KERNEL_ARG_TYPE_QUALIFIER, 0, NULL, &argtype);
		err = clGetKernelArgInfo(kernel, x, CL_KERNEL_ARG_TYPE_QUALIFIER, argtype, &argtype, NULL);
		err = clGetKernelArgInfo(kernel, x, CL_KERNEL_ARG_NAME, 0, NULL, &argnamesize);
		err = clGetKernelArgInfo(kernel, x, CL_KERNEL_ARG_NAME, argnamesize, argname, &argnamesize);

		/*	check for predefine variable names.	*/
		if(strcmp(argname, "fragColor") == 0){
			err = clSetKernelArg(kernel, kerneltexindex, sizeof(cl_mem), &clmemframetexture[0]);
			continue;
		}
		if(strcmp(argname, "resolution") == 0){
			err = clSetKernelArg(kernel, kerneltexindex, sizeof(cl_uint2), NULL);
			uniform->resolution = x;
			continue;
		}
		if(strcmp(argname, "time") == 0){
			err = clSetKernelArg(kernel, kerneltexindex, sizeof(cl_float), NULL);
			uniform->time = x;
			continue;
		}
		if(strcmp(argname, "mouse") == 0){
			err = clSetKernelArg(kernel, kerneltexindex, sizeof(cl_uint2), NULL);
			uniform->mouse = x;
			continue;
		}
		if(strcmp(argname, "deltatime") == 0){
			err = clSetKernelArg(kernel, kerneltexindex, sizeof(cl_float), NULL);
			uniform->deltatime = x;
			continue;
		}
		if(strcmp(argname, "width") == 0){
			err = clSetKernelArg(kernel, kerneltexindex, sizeof(cl_uint), &w);
			uniform->deltatime = x;
			continue;
		}
		if(strcmp(argname, "height") == 0){
			err = clSetKernelArg(kernel, kerneltexindex, sizeof(cl_uint), &h);
			uniform->deltatime = x;
			continue;
		}
		if(strcmp(argtypename, "image2d_t") == 0){
			if(glIsTexture( textures[texind].texture ) == GL_TRUE){
				texmem = clCreateFromGLTexture((cl_context)context,
						CL_MEM_READ_ONLY,
						textures[texind].target,
						0,
						textures[texind].texture,
						&err);
				if(texmem == NULL){

					continue;
				}

				cltextures[texind] = texmem;
				texind = ++numcltextures;
				clSetKernelArg(kernel, x, sizeof(cl_mem), &texmem);
				cluniform.tex[texind] = x;

				/*	Acquire */
				err = clEnqueueAcquireGLObjects(clqueue, texind, &cltextures[x], 0, NULL, NULL);
				if(err != CL_SUCCESS){
					fprintf(stderr, get_cl_error_str(err));
				}
			}

		}/**/


		kerneltexindex++;
	}

	return program;
}

void glslview_acquirecltextures(cl_context context, cl_command_queue queue, cl_kernel kernel){
	cl_mem texmem;
	cl_int err;
	int x;
	int kerneltexindex;

	/*	TODO resolve the index.	*/
	kerneltexindex = 3;
	for(x = 0; x < numTextures; x++){
		if(glIsTexture( textures[x].texture ) == GL_TRUE){
			texmem = clCreateFromGLTexture((cl_context)context,
					CL_MEM_READ_ONLY,
					textures[x].target,
					0,
					textures[x].texture,
					&err);
			if(texmem == NULL){

				continue;
			}

			cltextures[numcltextures] = texmem;
			numcltextures++;
			clSetKernelArg(kernel, kerneltexindex  + x + 0, sizeof(cl_mem), &texmem);
			cluniform.tex[x + kerneltexindex] = x;
			/*	Acquire */
			err = clEnqueueAcquireGLObjects(clqueue, numcltextures, &cltextures[x], 0, NULL, NULL);
			if(err != CL_SUCCESS){
				fprintf(stderr, get_cl_error_str(err));
			}


		}else{
			break;
		}
	}


}


void glslview_cl_resize(unsigned int width, unsigned int height){
	GLuint clearColor[4] = {0, 0, 0, 0};
	cl_int err;
	int i;

	/*	Create framebuffer.	*/
	if( !( width == clframetexture[0].width && height == clframetexture[0].height ) ){
		clFlush(clqueue);
		glFlush();
		glFinish();
		for(i = 0; i < numclframebuffer; i++){

			if(glIsTexture(clframetexture[i].texture) == GL_TRUE){
				glDeleteTextures(1, &clframetexture[i].texture);
				glslview_create_texture(&clframetexture[i], GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0 , GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				/*
				glBindTexture(clframetexture[i].target,clframetexture[i].texture);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0 , GL_RGBA, GL_UNSIGNED_BYTE, NULL);
				clframetexture[i].width = width;
				clframetexture[i].height = height;
				*/
			}else{
				glslview_create_texture(&clframetexture[i], GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0 , GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			}
			glBindTexture(clframetexture[i].target, clframetexture[i].texture);
			glClearTexImage(clframetexture[i].texture, 0, GL_RGBA, GL_UNSIGNED_BYTE, &clearColor);

			/**/
			if(clmemframetexture[i] != NULL){
				err = clReleaseMemObject(clmemframetexture[i]);
				clmemframetexture[i] = NULL;
				if(err != CL_SUCCESS){
					fprintf(stderr, "%s\n", get_cl_error_str(err));
				}
			}

			/**/
			clmemframetexture[i] = clCreateFromGLTexture(clcontext, CL_MEM_WRITE_ONLY,
								GL_TEXTURE_2D, 0,clframetexture[i].texture, &err);
			if(err != CL_SUCCESS){
				fprintf(stderr, "%s\n", get_cl_error_str(err));
			}
		}
	}



	if(clkernel){
		int res[2] = {width, height};

		/*	set work group.	*/
		globalsize[0] = fmax( 128 * (width / 512) , 128);
		globalsize[1] = fmax( 128 * (height / 512) , 128);
		localsize[0] = fmax( 8 * (width / 512) , 4);
		localsize[1] = fmax( 8 * (height / 512) , 4);





		/**/
		for(i = 0; i < sizeof(cluniform.tex) / sizeof(cluniform.tex[0]); i++){
			if(cluniform.tex[i] != -1){
				clSetKernelArg(clkernel, cluniform.tex[i], sizeof(cl_mem), &cltextures[i]);
			}
		}

		/**/
		err = clSetKernelArg(clkernel, 0, sizeof(cl_mem), &clmemframetexture[0]);
		err = clSetKernelArg(clkernel, 1, sizeof(cl_uint), &width);
		err = clSetKernelArg(clkernel, 2, sizeof(cl_uint), &height);

		/**/
		if(cluniform.resolution != -1){
			err = clSetKernelArg(clkernel, cluniform.resolution, sizeof(cl_int2), &res[0]);
		}

	}

}
void glslview_cl_createframebuffer(unsigned int width, unsigned int height){

}

void glslview_renderclframe(cl_command_queue queue, cl_kernel kernel){
	cl_int err = 0;



	err |= clEnqueueAcquireGLObjects(queue, 1, &clmemframetexture[clcurrent], 0,0, NULL);
	err |= clEnqueueNDRangeKernel(queue, kernel, 2, &global_work_offset[0], &globalsize[0], &localsize[0], 0, NULL, NULL);
	err |= clFlush(queue);
	err |= clEnqueueReleaseGLObjects(queue, 1,  &clmemframetexture[clcurrent], 0, 0, NULL);

	/*	Prevent the system from crashing.	*/
	if(err != CL_SUCCESS){
		fprintf(stderr, "%s\n", get_cl_error_str(err));
		glslview_clrelease();
		exit(EXIT_FAILURE);
	}
}

