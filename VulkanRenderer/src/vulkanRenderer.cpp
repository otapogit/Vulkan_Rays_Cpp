
#include <vulkan/vulkan.h>
#include <stdexcept>
#include "glm/ext.hpp"

bool VKfoo() {

	VkInstance instance;
	{
		// 1.1. Specify the application information.
		// One important info is the "apiVersion"
		/*
		  VkApplicationInfo structure is used to indicate application information which
		  can be used by the underlying driver.

		  For the most parts there are no required informations, users/developers can
		  specify their own pApplicationName, applicationVersion, pEngineName, and engineVersion.
		  The apiVersion is used to requires a given Vulkan version from the driver.
		  If a VK_API_VERSION_1_0 is required the driver must return an 1.0 "Instance".
		*/
		VkApplicationInfo appInfo;
		{
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pNext = NULL;
			appInfo.pApplicationName = "MinimalVkTriangle";
			appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.pEngineName = "RAW";
			appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
			appInfo.apiVersion = VK_API_VERSION_1_1;
		}

		// 1.2. Specify the Instance creation information.
		// The Instance level Validation and debug layers must be specified here.
		/*
		  VkInstanceCreateInfo is required to create a Vulkan instance in which
		  any other Vulkan operation is done (This is similar to the OpenGL context).

		  The pApplicationInfo is an optional argument, if it is a NULL pointer a default app Info is used.
		  Layers are used to track/modify/extend existing Vulkan API calls.
		  For example: the "VK_LAYER_KHRONOS_validation" is used to provide information on invalid
			API calls and structure configuration(s).

		  Extensions are used to add extra API methods and to use any it must be specified
		  otherwise it is not usable.
		  For example: to have a presenting surface a KHR_surface extension is required.
		*/
		VkInstanceCreateInfo createInfo;
		{
			createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
			createInfo.pNext = NULL;
			createInfo.flags = 0;
			createInfo.pApplicationInfo = &appInfo;
			createInfo.enabledLayerCount = 0;
			createInfo.enabledExtensionCount = 0;
			createInfo.ppEnabledExtensionNames = NULL;
		}

		// 1.3. Create the Vulkan instance.
		if (vkCreateInstance(&createInfo, NULL, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
			return false;
		}

	}
	return true;
}
