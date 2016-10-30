#include"internal.h"
#include<GL/gl.h>
#include<CL/cl.h>
#include<CL/cl_gl.h>

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
cl_context glslview_createclcontext(ExOpenGLContext shared, unsigned int* numDevices, cl_device_id** device){
	cl_int ciErrNum;
	cl_context context;
	cl_platform_id* platforms;
	cl_device_id* devices = NULL;
	cl_device_id curgldevice;
	int x = 0;
	size_t i;

	/**/
	cl_context_properties props[] = {
			CL_CONTEXT_PLATFORM, (cl_context_properties)NULL,
			CL_GL_CONTEXT_KHR,   (cl_context_properties)shared,
			CL_GLX_DISPLAY_KHR,     (cl_context_properties)ExGetDisplay(),
			0
	};

	unsigned int nDevices = 0;
	unsigned int nPlatforms = 0;
	unsigned int nselectPlatform = 0;

	/*	get platform id.	*/
	ciErrNum = clGetPlatformIDs(0, NULL, &nPlatforms);
	platforms = malloc(sizeof(*platforms) * nPlatforms);
	ciErrNum = clGetPlatformIDs(nPlatforms, platforms, NULL);

	/*	iterate */
	for(x = 0; x < nPlatforms; x++){
		props[1] = (cl_context_properties)platforms[x];
		size_t bytes = 0;

		/*	queuring how much bytes we need to read	*/
		clGetGLContextInfoKHR(props, CL_DEVICES_FOR_GL_CONTEXT_KHR, 0, NULL, &bytes);
		clGetGLContextInfoKHR(props, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, 0, NULL, &bytes);

		// allocating the mem
		size_t devNum = bytes/sizeof(cl_device_id);
		devices = (cl_device_id*)realloc(devices, bytes + nDevices * sizeof(cl_device_id));

		/**/
		clGetGLContextInfoKHR(props, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, bytes, &devices[nDevices], NULL);
		nDevices += devNum;
		/*	iterate over all devices	*/
		for(i = 0; i < devNum; i++){
		      /*	enumerating the devices for the type, names, CL_DEVICE_EXTENSIONS, etc	*/
		}

	}
	/*	create context.	*/
	props[1] = platforms[nselectPlatform];
	context = clCreateContext(props, nDevices, devices, NULL, NULL, &ciErrNum);
	if(context == NULL){
		fprintf(stderr, "Failed to create OpenCL context. %d\n  [ %s ]", ciErrNum, get_cl_error_str(ciErrNum));
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

	ciErrNum = clBuildProgram(program, nDevices, device, NULL, NULL, NULL);
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

/**/
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


cl_context glslview_createCLContext(ExOpenGLContext shared, unsigned int* ncldevices, cl_device_id** devices){
	cl_command_queue queue;
	cl_context context;
	cl_platform_id platform;
	cl_int err;

	assert(shared);

	context = glslview_createclcontext(shared, ncldevices, devices);
	if(context == NULL){
		return NULL;
	}
	queue = glslview_createcommandqueue(context, (*devices)[0]);


	/*	*/
	ExCreateTexture(&clframetexture, GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0 , GL_RGB, GL_UNSIGNED_BYTE, NULL);
	clmemframetexture = clCreateFromGLTexture(context, CL_MEM_WRITE_ONLY,  GL_TEXTURE_2D, 0, clframetexture.texture, &err);

	return context;
}

cl_program glslview_createCLProgram(cl_context context, unsigned int nNumDevices, cl_device_id* id, const char* cfilename, UniformLocation* uniform){
	cl_program program;
	cl_kernel kernel;
	cl_mem texmem;
	cl_int err;
	int x;
	int kerneltexindex;
	cl_uint numKernelArgs;
	char argname[256];
	size_t argnamesize;
	unsigned int width;
	unsigned int height;


	program = glslview_createProgram(context, nNumDevices, id, cfilename);
	assert(program);
	kernel = clCreateKernel(program, "main", &err);
	if(err != CL_SUCCESS){
		fprintf(stderr, "Failed to create kernel %d %s\n", err, get_cl_error_str(err));
		return NULL;
	}




	/*	framebuffer image view attributes information.	*/
	err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clmemframetexture);

	/*	iterate through all the argument.	*/
	kerneltexindex = 1;
	err = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof(numKernelArgs), &numKernelArgs, NULL);
	for(x = kerneltexindex; x < numKernelArgs; x++){
		err = clGetKernelArgInfo(kernel, x, CL_KERNEL_ARG_NAME, 0, NULL, &argnamesize);
		err = clGetKernelArgInfo(kernel, x, CL_KERNEL_ARG_NAME, argnamesize, argname, &argnamesize);
		/*	check for predefine variable names.	*/
		if(strcmp(argname, "resolution") == 0){
			clSetKernelArg(kernel, kerneltexindex, sizeof(cl_uint2), NULL);
			uniform->resolution = x;
			continue;
		}
		if(strcmp(argname, "time") == 0){
			clSetKernelArg(kernel, kerneltexindex, sizeof(cl_float), NULL);
			uniform->time = x;
			continue;
		}
		if(strcmp(argname, "mouse") == 0){
			clSetKernelArg(kernel, kerneltexindex, sizeof(cl_uint2), NULL);
			uniform->mouse = x;
			continue;
		}
		if(strcmp(argname, "deltatime") == 0){
			clSetKernelArg(kernel, kerneltexindex, sizeof(cl_float), NULL);
			uniform->deltatime = x;
			continue;
		}

		kerneltexindex++;
	}

	/*	*/
	for(x = 0; x < sizeof(textures) / sizeof(textures[0]); x++){
		if(ExIsTexture( &textures[x] )){
			texmem = clCreateFromGLTexture((cl_context)context,
					CL_MEM_READ_ONLY,
					textures[x].target,
					0,
					textures[x].texture,
					&err);
			clSetKernelArg(kernel, kerneltexindex  + x + 0, sizeof(cl_mem), &texmem);
			uniform->tex[x + kerneltexindex] = x;

		}else{
			break;
		}
	}

	return program;
}


void glslview_renderclframe(cl_command_queue queue, cl_kernel kernel, unsigned int w, unsigned int h){

	int globalsize[3] = {1024, 1024,1};
	int localsize[3] = {w/1024, h / 1024, 1};
	clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalsize, localsize, 0, NULL, NULL);
	clFlush(queue);
}

