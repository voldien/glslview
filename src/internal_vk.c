#include"internal.h"
#include<vulkan/vulkan.h>

VkInstance instance;
VkPhysicalDevice kdevice;
VkDevice device;
VkQueue queue;
VkCommandBuffer commandbuffer;



int glslview_init_vulkan(SDL_Window* window){
	VkResult result;
	unsigned int num_devices;
	uint32_t queue_families_count = 0;
	VkPhysicalDevice stackdevice[8];
	VkApplicationInfo appInfo = {};

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "glslview";
	appInfo.applicationVersion = VK_MAKE_VERSION(0,6,5);
	appInfo.pEngineName = "";
	appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
	appInfo.apiVersion = VK_API_VERSION_1_0;


	VkInstanceCreateInfo createinfo = {};
	createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createinfo.pApplicationInfo = &appInfo;

	result = vkCreateInstance(&createinfo, NULL, &instance);
	if(result != VK_SUCCESS){

	}


	if( vkEnumeratePhysicalDevices( instance, &num_devices, NULL ) != VK_SUCCESS){

	}



	if( vkEnumeratePhysicalDevices( instance, &num_devices, &stackdevice[0] ) != VK_SUCCESS ) {

	}


	vkGetPhysicalDeviceQueueFamilyProperties( kdevice, &queue_families_count, NULL );


	VkDeviceQueueCreateInfo queueinfo = {};
	queueinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

	VkDeviceCreateInfo deviceinfo = {};
	deviceinfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;


	result = vkCreateDevice(kdevice, &deviceinfo, NULL, &device);






	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	/*createInfo.pfnCallback = debugCallback;*/


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
