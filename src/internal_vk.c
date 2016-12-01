#include"internal.h"
#include<SDL2/SDL.h>
#include<SDL2/SDL_syswm.h>
#define VK_USE_PLATFORM_XLIB_KHR
#include<vulkan/vulkan.h>

VkInstance instance;
VkPhysicalDevice kdevice;
VkDevice device;
VkQueue queue;
VkCommandBuffer commandbuffer;


SDL_Window* glslview_init_vulkan(void){
	SDL_Window* win;
	SDL_DisplayMode displaymode;
	SDL_SysWMinfo wininfo = {0};
	VkResult result;
	unsigned int num_devices;
	uint32_t queue_families_count = 0;
	VkPhysicalDevice* stackdevice;
	VkApplicationInfo appInfo = {};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "glslview";
	appInfo.applicationVersion = VK_MAKE_VERSION(	GLSLVIEW_MAJOR_VERSION,
													GLSLVIEW_MINOR_VERSION,
													GLSLVIEW_REVISION_VERSION);

	appInfo.pEngineName = "glslview";
	appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.pNext = NULL;


	VkInstanceCreateInfo createinfo;
	createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createinfo.pApplicationInfo = &appInfo;
	createinfo.flags = 0;
	createinfo.enabledLayerCount = 0;
	createinfo.ppEnabledLayerNames = NULL;
	createinfo.enabledExtensionCount = 0;
	createinfo.ppEnabledExtensionNames = NULL;
	createinfo.pNext = NULL;

	/**/
	result = vkCreateInstance(&createinfo, NULL, &instance);
	if(result != VK_SUCCESS){
		fprintf(stderr, "Failed to create instance %d.\n", result);
		return NULL;
	}


	/**/
	result = vkEnumeratePhysicalDevices( instance, &num_devices, NULL );
	if( result != VK_SUCCESS){
		fprintf(stderr, "Failed to numeratePhysicalDevices %d.\n", result);
		return NULL;
	}


	/*	*/
	stackdevice = malloc(num_devices * sizeof(VkPhysicalDevice));
	assert(stackdevice);
	result = vkEnumeratePhysicalDevices( instance, &num_devices, &stackdevice[0] );
	if( result != VK_SUCCESS ) {
		fprintf(stderr, "Failed to numeratePhysicalDevices %d.\n", result);
	}



	vkGetPhysicalDeviceQueueFamilyProperties( kdevice, &queue_families_count, NULL );


	VkDeviceQueueCreateInfo queueinfo = {};
	queueinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

	VkDeviceCreateInfo deviceinfo = {};
	deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;


	/*	*/
	result = vkCreateDevice(kdevice, &deviceinfo, NULL, &device);
	if(result != VK_SUCCESS){

	}





	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	/*createInfo.pfnCallback = debugCallback;*/


	/*	Create window.	*/
	SDL_GetCurrentDisplayMode(0, &displaymode);
	displaymode.w /= 2;
	displaymode.h /= 2;
	win = SDL_CreateWindow("", displaymode.w/ 2, displaymode.h / 2, displaymode.w, displaymode.h, SDL_WINDOW_RESIZABLE);


	SDL_VERSION(&wininfo.version);
	SDL_GetWindowWMInfo(win, &wininfo);

	VkSurfaceKHR surface;
	VkXlibSurfaceCreateInfoKHR surfaceinfo = {};
	surfaceinfo.dpy = wininfo.info.x11.display;
	surfaceinfo.window = wininfo.info.x11.window;
	surfaceinfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;

	result = vkCreateXlibSurfaceKHR(instance, &surfaceinfo, NULL, &surface);
	if(result != VK_SUCCESS){
		fprintf(stderr, "Failed to create Xlib Surface %d.\n", result);
		return NULL;
	}


	return window;
}

void glslview_resize_screen_vk(int* res, struct uniform_location_t* uniform, glslviewShader* shader, glslviewTexture* ftexture){
	VkViewport viewport;
	viewport.x = 0;
	viewport.y = 0;
	//viewport.width = event->size.width;
	//viewport.height = event->size.height;
	viewport.minDepth = 1.0;
	viewport.maxDepth = 0.0;

	vkCmdSetViewport(commandbuffer, 1, 1, &viewport);

}


void glslview_update_shader_uniform_vk(struct uniform_location_t* uniform, glslviewShader* shader, int width, int height){


}

void glslview_displaygraphic_vk(SDL_Window* drawable){

	/**/
	glslview_swapbuffer(drawable);
}





void glslview_update_uniforms_vk(UniformLocation* uniform, glslviewShader* shader, float ttime, long int deltatime){



}

glslviewTexture* glslview_create_texture_vk(glslviewTexture* texture, unsigned int target, int level, int internalFormat, int width, int height, int border, unsigned int format, unsigned int type, const void *pixels){

	return NULL;
}

int glslview_create_shader_vk(glslviewShader* texture, const char* cvertexSource, const char* cfragmentSource, const char* cgeometry_source, const char* ctess_c_source, const char* ctess_e_source){

	return 0;
}

void glslview_rendergraphic_vk(SDL_Window* drawable, glslviewShaderCollection* shader, float ttime, float deltatime){


}




