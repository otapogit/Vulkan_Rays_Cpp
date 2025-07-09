#include <iostream>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include "core/core.h"

#include <cassert>



namespace core {



	VulkanCore::VulkanCore() {

	}
	VulkanCore::~VulkanCore() {

		printf("\n---------------------------------------\n\n");


		vkFreeCommandBuffers(m_device, m_cmdBufPool, 1, &m_copyCmdBuf);

		for (uint32_t i = 0; i < m_frameBuffers.size();i++) {
			vkDestroyFramebuffer(m_device,m_frameBuffers[i],NULL);
		}
		printf("Destroyed FrameBuffers\n");

		m_queue.Destroy();

		printf("Destroyed Queue semaphores\n");
		
		vkDestroyCommandPool(m_device, m_cmdBufPool, NULL);

		printf("Destroyed Command Pool\n");

		for (int i = 0; i < m_imageViews.size(); i++) {
			vkDestroyImageView(m_device, m_imageViews[i], NULL);
		}

		vkDestroySwapchainKHR(m_device, m_swapChain, NULL);

		//Destruir dispositivos l�gicos
		vkDestroyDevice(m_device, NULL);
		printf("Device destroyed\n");

		//No es necesario destruir los dispositivos fisicos

		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger = VK_NULL_HANDLE;
		vkDestroyDebugUtilsMessenger = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
		vkDestroyDebugUtilsMessenger(m_instance, m_debugMessenger, NULL);
		if (!vkDestroyDebugUtilsMessenger) {
			printf("Cannot find addres of vkDestroyDebugUtilsMessenger\n");
		}
		printf("Destroyed Debug callback\n");


		PFN_vkDestroySurfaceKHR vkDestroySurface = VK_NULL_HANDLE;
		vkDestroySurface = (PFN_vkDestroySurfaceKHR)vkGetInstanceProcAddr(m_instance, "vkDestroySurfaceKHR");
		vkDestroySurfaceKHR(m_instance, m_surface, NULL);
		printf("GLFW window surface destroyed\n");


		vkDestroyInstance(m_instance, NULL);
		printf("Destroyed Instance\n");

	}

	/*void VulkanCore::Init(const char* pAppName, GLFWwindow* pWindow) {

		m_pWindow = pWindow;
		CreateInstance(pAppName);
		CreateDebugCallback();
		//No hace falta surface
		CreateSurface(pWindow);
		m_physDevices.Init(m_instance, m_surface);
		m_queueFamily = m_physDevices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, true);
		CreateDevice();

		CreateSwapChain();
		CreateCommandBufferPool();
		//Ahora mismo solo la primera q

		m_queue.Init(m_device, m_swapChain, m_queueFamily, 0);
		CreateCommandBuffer(1, &m_copyCmdBuf);

		CreateDepthResources();
	}*/

	void VulkanCore::Init(const char* pAppName, uint32_t width, uint32_t height) {
		// Eliminar: m_pWindow = pWindow;
		m_offscreenWidth = width;
		m_offscreenHeight = height;

		m_numImages = 1;

		CreateInstance(pAppName);
		CreateDebugCallback();
		// ELIMINAR: CreateSurface(pWindow);
		m_physDevices.Init(m_instance, VK_NULL_HANDLE); // Cambiar m_surface por VK_NULL_HANDLE
		m_queueFamily = m_physDevices.SelectDevice(VK_QUEUE_GRAPHICS_BIT, false); // Cambiar true por false
		CreateDevice();
		// ELIMINAR: CreateSwapChain();
		CreateOffscreenImages(); // NUEVA FUNCI�n
		CreateCommandBufferPool();
		m_queue.Init(m_device, VK_NULL_HANDLE, m_queueFamily, 0); // Cambiar m_swapChain por VK_NULL_HANDLE
		CreateCommandBuffer(1, &m_copyCmdBuf);
		CreateDepthResources();
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageTypeFlagBitsEXT Severity,
		VkDebugUtilsMessageTypeFlagsEXT Type,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) 
	{
		printf("Debug callback: %s\n", pCallbackData->pMessage);
		printf("	Severity %s\n", Get_DebugSeverityString(Severity));
		printf("	Type %s\n", Get_DebugType(Type));
		printf("	Objects");
		for (uint32_t i = 0; i < pCallbackData->objectCount; i++) {
			printf("%llx ", pCallbackData->pObjects[i].objectHandle);
		}
		return VK_FALSE;
	}

	void VulkanCore::CreateDebugCallback() {
		VkDebugUtilsMessengerCreateInfoEXT MessengerCreateInfo = {};

		MessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		MessengerCreateInfo.pNext = NULL;
		MessengerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		MessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		MessengerCreateInfo.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)DebugCallback;
		MessengerCreateInfo.pUserData = NULL;

		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger = VK_NULL_HANDLE;
		vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT");
		if (!vkCreateDebugUtilsMessenger) {
			printf("Couldnt create Callback");
			exit(1);
		}
		VkResult res = vkCreateDebugUtilsMessenger(m_instance, &MessengerCreateInfo, NULL, &m_debugMessenger);
		CHECK_VK_RESULT(res, "Debug utils messenger");
		printf("Debug utils messenger created\n");

	}

	void VulkanCore::CreateInstance(const char* pAppName) {
		std::vector<const char*> Layers = {
			"VK_LAYER_KHRONOS_validation"
		};

		std::vector<const char*> Extensions = {
			VK_KHR_SURFACE_EXTENSION_NAME,
#if defined(_WIN32)
			"VK_KHR_win32_surface",
#endif
#if defined(__APPLE__)
			"VK_MVK_macos_surface",
#endif
#if defined(__linux__)
			"VK_KHR_xcb_surface"
#endif
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME

		};


		VkApplicationInfo AppInfo = {};
		AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		AppInfo.pNext = NULL;
		AppInfo.pApplicationName = pAppName,
		AppInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		AppInfo.pEngineName = "OtapoEngine";
		AppInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
		AppInfo.apiVersion = VK_API_VERSION_1_0;
		

		VkInstanceCreateInfo CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		CreateInfo.pNext = NULL;
		CreateInfo.flags = 0;
		CreateInfo.pApplicationInfo = &AppInfo;
		CreateInfo.enabledLayerCount = (uint32_t)(Layers.size());
		CreateInfo.ppEnabledLayerNames = Layers.data();
		CreateInfo.enabledExtensionCount = (uint32_t)(Extensions.size());
		CreateInfo.ppEnabledExtensionNames = Extensions.data();

		//No allocator
		VkResult res = vkCreateInstance(&CreateInfo, NULL, &m_instance);
		CHECK_VK_RESULT(res,"Create instance");
		printf("Vulkan instance created \n");

	}

	void VulkanCore::CreateDevice() {
		float qPriorities[] = { 1.0f };

		VkDeviceQueueCreateInfo qInfo = {};
		qInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		qInfo.pNext = NULL;
		qInfo.flags = 0;
		qInfo.queueFamilyIndex = m_queueFamily;
		//SOLO SE USA 1 QUEUE POR AHORA, AUMENTAR PARA IR MAS RAPIDO
		//La cosa seria coger todas las queues disponibles del dispositivo ty queue family
		//Seria m_devices.selected().m_qFamilyProps[indice seleccionado].queuecount
		qInfo.queueCount = 1;
		qInfo.pQueuePriorities = &qPriorities[0];



		std::vector<const char*> DevExts = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME,
			VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
			VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
			VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
			VK_KHR_SPIRV_1_4_EXTENSION_NAME,                 // Requerida por ray tracing
			VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME,     // Requerida por SPIRV 1.4
			VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,       // Requerida por ray tracing
			VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,     // Requerida por acceleration structure
			VK_KHR_RAY_QUERY_EXTENSION_NAME
		};



		VkPhysicalDeviceFeatures DeviceFeatures = { 0 };
		if (m_physDevices.Selected().m_features.geometryShader == VK_FALSE) {
			fprintf(stderr, "Geometry shader not supported");
		}
		if (m_physDevices.Selected().m_features.tessellationShader == VK_FALSE) {
			fprintf(stderr, "Tesselation shader not supported");
		}
		DeviceFeatures.geometryShader = VK_TRUE;
		DeviceFeatures.tessellationShader = VK_TRUE;

		VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures = {};
		bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
		bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;

		// Caracter�sticas de acceleration structure - ESTA ES LA CR�TICA QUE FALTABA
		VkPhysicalDeviceAccelerationStructureFeaturesKHR asFeatures = {};
		asFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
		asFeatures.accelerationStructure = VK_TRUE;
		asFeatures.pNext = &bufferDeviceAddressFeatures;

		VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeatures = {};
		rtPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
		rtPipelineFeatures.rayTracingPipeline = VK_TRUE;
		rtPipelineFeatures.pNext = &asFeatures;

		VkDeviceCreateInfo DeviceCreateInfo = {};
		DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		DeviceCreateInfo.pNext = &rtPipelineFeatures;
		DeviceCreateInfo.flags = 0;
		DeviceCreateInfo.queueCreateInfoCount = 1;
		DeviceCreateInfo.pQueueCreateInfos = &qInfo;
		DeviceCreateInfo.enabledLayerCount = 0;
		DeviceCreateInfo.ppEnabledLayerNames = NULL;
		DeviceCreateInfo.enabledExtensionCount = (uint32_t)DevExts.size();
		DeviceCreateInfo.ppEnabledExtensionNames = DevExts.data();
		DeviceCreateInfo.pEnabledFeatures = &DeviceFeatures;

		VkResult res = vkCreateDevice(m_physDevices.Selected().m_physDevice, &DeviceCreateInfo, NULL, &m_device);
		CHECK_VK_RESULT(res, "Create device\n");
		printf("\nDevice created\n");

	}
	/*
	void VulkanCore::CreateSurface(GLFWwindow* pWindow) {
		if (glfwCreateWindowSurface(m_instance, pWindow, NULL, &m_surface)) {
			fprintf(stderr, "Error creating GLFW window surface\n");
			exit(1);
		}
		printf("GLFW window surface created\n");


	}*/

	core::PhysicalDevice VulkanCore::GetSelectedPhysicalDevice() {
		return m_physDevices.Selected();
	}

	/*
		Para double buffering pedir una m�s
	*/
	static uint32_t ChooseNumImages(const VkSurfaceCapabilitiesKHR& Caps) {
		uint32_t RequestedNumImages = Caps.minImageCount + 1;
		int FinalNumImages = 0;
		if ((Caps.maxImageCount > 0) && (RequestedNumImages > Caps.maxImageCount)) {
			FinalNumImages = Caps.maxImageCount;
		}
		else {
			FinalNumImages = RequestedNumImages;
		}
		return FinalNumImages;
	}

	/*
		Ver que modo queremos usar, si esperar al VSync o no
	*/
	static VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& PresentModes) {

		for (int i = 0; i < PresentModes.size(); i++) {
			if (PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				return PresentModes[i];
			}
		}

		//Fifo siempre soportado
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	/*
		Devuelve un formato espec�fico de surface y colorspace
		BGRA8 y SRGB
	*/
	static VkSurfaceFormatKHR ChooseSurfaceFormatAndColorSpace(const std::vector<VkSurfaceFormatKHR>& SurfaceFormats) {
		for (int i = 0; i < SurfaceFormats.size(); i++) {
			if ((SurfaceFormats[i].format == VK_FORMAT_B8G8R8A8_SRGB) &&
				(SurfaceFormats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)) {
				return SurfaceFormats[i];
			}
		}
		return SurfaceFormats[0];
	}



	void VulkanCore::CreateSwapChain() {
		const VkSurfaceCapabilitiesKHR& SurfaceCaps = m_physDevices.Selected().m_surfaceCaps;

		uint32_t NumImages = ChooseNumImages(SurfaceCaps);

		const std::vector<VkPresentModeKHR>& PresentModes = m_physDevices.Selected().m_presentModes;
		VkPresentModeKHR PresentMode = ChoosePresentMode(PresentModes);

		VkSurfaceFormatKHR SurfaceFormat = ChooseSurfaceFormatAndColorSpace(m_physDevices.Selected().m_surfaceFormats);
		m_swapChainSurfaceFormat = SurfaceFormat;

		VkSwapchainCreateInfoKHR SwapChainCreateInfo = {};
		SwapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		SwapChainCreateInfo.pNext = NULL;
		SwapChainCreateInfo.flags = 0;
		SwapChainCreateInfo.surface = m_surface;
		SwapChainCreateInfo.minImageCount = NumImages;
		SwapChainCreateInfo.imageFormat = SurfaceFormat.format;
		SwapChainCreateInfo.imageExtent = SurfaceCaps.currentExtent;
		SwapChainCreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
		SwapChainCreateInfo.imageArrayLayers = 1;
		//Importante
		SwapChainCreateInfo.imageUsage = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		SwapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		//Estos 2 siguientes solo para modos shared
		SwapChainCreateInfo.pQueueFamilyIndices = &m_queueFamily;
		SwapChainCreateInfo.queueFamilyIndexCount = 1;
		SwapChainCreateInfo.preTransform = SurfaceCaps.currentTransform;
		SwapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		SwapChainCreateInfo.presentMode = PresentMode;
		SwapChainCreateInfo.clipped = VK_TRUE;

		VkResult res = vkCreateSwapchainKHR(m_device, &SwapChainCreateInfo, NULL, &m_swapChain);
		CHECK_VK_RESULT(res, "vkCreateSwapChainKHR\n");

		printf("Swap chain created\n");

		uint32_t NumSwapChainImages = 0;
		res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, NULL);
		CHECK_VK_RESULT(res, "vkGetSwapChainImagesKHR\n");
		//Control
		assert(NumImages == NumSwapChainImages);

		printf("Number of images %d\n", NumSwapChainImages);
		m_numImages = NumSwapChainImages;
		m_images.resize(NumSwapChainImages);
		m_imageViews.resize(NumSwapChainImages);

		res = vkGetSwapchainImagesKHR(m_device, m_swapChain, &NumSwapChainImages, m_images.data());
		CHECK_VK_RESULT(res, "vkGetSwapChainImagesKHR (2)\n");

		//No mipmaping o multiple levels
		int LayerCount = 1;
		int MipLevels = 1;
		for (uint32_t i = 0; i < NumSwapChainImages; i++) {
			m_imageViews[i] = CreateImageView(m_device, m_images[i], SurfaceFormat.format,
					VK_IMAGE_ASPECT_COLOR_BIT);
		}
	}

	void VulkanCore::CreateCommandBufferPool() {

		VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
		cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		cmdPoolCreateInfo.pNext = NULL;
		cmdPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		cmdPoolCreateInfo.queueFamilyIndex = m_queueFamily;

		VkResult res = vkCreateCommandPool(m_device, &cmdPoolCreateInfo, NULL, &m_cmdBufPool);
		CHECK_VK_RESULT(res, "vkCreateCommandPool\n");

		printf("Command buffer pool created\n");
	}

	/*
		Solo Crea buffers primarios por ahora
	*/
	void VulkanCore::CreateCommandBuffer(uint32_t count, VkCommandBuffer* cmdBufs) {

		VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
		cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufAllocInfo.pNext = NULL;
		cmdBufAllocInfo.commandPool = m_cmdBufPool;
		//Buffer primario
		cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufAllocInfo.commandBufferCount = count;

		VkResult res = vkAllocateCommandBuffers(m_device, &cmdBufAllocInfo, cmdBufs);
		CHECK_VK_RESULT(res, "vkAllocateCommandBuffers\n");

		printf("%d command buffers created\n", count);
	}
	
	void VulkanCore::FreeCommandBuffers(uint32_t count, const VkCommandBuffer* pCmdBufs) {

		m_queue.WaitIdle();
		vkFreeCommandBuffers(m_device, m_cmdBufPool, count, pCmdBufs);
	}

	const VkImage& VulkanCore::GetImage(int Index) const
	{
		if (Index >= m_numImages) {
			fprintf(stderr, "Error getting image");
			exit(1);
		}

		return m_images[Index];
	}

	VkRenderPass VulkanCore::CreateSimpleRenderPass() {
		
		VkAttachmentDescription AttachDesc = {};
		AttachDesc.flags = 0;
		AttachDesc.format = m_swapChainSurfaceFormat.format;
		AttachDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		AttachDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		AttachDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		AttachDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		AttachDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		AttachDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		AttachDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference AttachRef = {};
		AttachRef.attachment = 0;
		AttachRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkFormat DepthFormat = m_physDevices.Selected().m_depthFormat;

		VkAttachmentDescription DepthAttachment = {};
		DepthAttachment.flags = 0;
		DepthAttachment.format = DepthFormat;
		DepthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		DepthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		DepthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		DepthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		DepthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		DepthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		DepthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference DepthAttachmentRef = {};
		DepthAttachmentRef.attachment = 1;
		DepthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		std::vector<VkAttachmentDescription> Attachments;
		Attachments.push_back(AttachDesc);
		Attachments.push_back(DepthAttachment);

		VkSubpassDescription SubpassDesc = {};
		SubpassDesc.flags = 0;
		SubpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		SubpassDesc.inputAttachmentCount = 0;
		SubpassDesc.pInputAttachments = NULL;
		SubpassDesc.colorAttachmentCount = 1;
		SubpassDesc.pResolveAttachments = NULL;
		SubpassDesc.pColorAttachments = &AttachRef;
		SubpassDesc.pDepthStencilAttachment = &DepthAttachmentRef;
		SubpassDesc.preserveAttachmentCount = 0;
		SubpassDesc.pPreserveAttachments = NULL;

		VkRenderPassCreateInfo RenderPassCreateInfo = {};
		RenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		RenderPassCreateInfo.pNext = NULL;
		RenderPassCreateInfo.flags = 0;
		RenderPassCreateInfo.attachmentCount = (uint32_t)Attachments.size();
		RenderPassCreateInfo.pAttachments = Attachments.data();
		RenderPassCreateInfo.subpassCount = 1;
		RenderPassCreateInfo.pSubpasses = &SubpassDesc;
		RenderPassCreateInfo.dependencyCount = 0;
		RenderPassCreateInfo.pDependencies = NULL;

		VkRenderPass RenderPass;

		VkResult res = vkCreateRenderPass(m_device, &RenderPassCreateInfo, NULL, &RenderPass);
		CHECK_VK_RESULT(res, "vkCreateRenderPass\n");
		printf("Created simple render pass\n");

		return RenderPass;
	}

	std::vector<VkFramebuffer> VulkanCore::CreateFrameBuffers(VkRenderPass RenderPass) {
		m_frameBuffers.resize(m_numImages);

		

		VkResult res;

		for (uint32_t i = 0; i < m_numImages; i++) {
			std::vector<VkImageView> Attachments;
			Attachments.push_back(m_imageViews[i]);
			Attachments.push_back(m_depthImages[i].m_view);

			VkFramebufferCreateInfo fbCreateInfo = {};
			fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			fbCreateInfo.renderPass = RenderPass;
			fbCreateInfo.attachmentCount = (uint32_t) Attachments.size();
			fbCreateInfo.pAttachments = Attachments.data();
			fbCreateInfo.width = m_offscreenWidth;
			fbCreateInfo.height = m_offscreenHeight;
			fbCreateInfo.layers = 1;

			res = vkCreateFramebuffer(m_device, &fbCreateInfo, NULL, &m_frameBuffers[i]);
			CHECK_VK_RESULT(res, "vkCreateFramebuffer\n");
		}

		printf("Framebuffers created \n");

		return m_frameBuffers;

	}

	BufferMemory VulkanCore::CreateVertexBuffer(const void* pVertices, size_t Size, bool rt) {

		VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags MemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		BufferMemory StagingVB = CreateBuffer(Size, Usage, MemProps);

		// Step 2: map the memory of the stage buffer
		void* pMem = NULL;
		VkDeviceSize Offset = 0;
		VkMemoryMapFlags Flags = 0;
		VkResult res = vkMapMemory(m_device, StagingVB.m_mem, Offset,
			StagingVB.m_allocationSize, Flags, &pMem);
		CHECK_VK_RESULT(res, "vkMapMemory\n");

		// Step 3: copy the vertices to the staging buffer
		memcpy(pMem, pVertices, Size);

		// Step 4: unmap/release the mapped memory
		vkUnmapMemory(m_device, StagingVB.m_mem);

		// Step 5: create the final buffer
		Usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT ;
		if (rt) {
			Usage = Usage | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		}
		MemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ;
		BufferMemory VB = CreateBuffer(Size, Usage, MemProps, rt);

		// Step 6: copy the staging buffer to the final buffer
		CopyBufferToBuffer(VB.m_buffer, StagingVB.m_buffer, Size);

		// Step 7: release the resources of the staging buffer
		StagingVB.Destroy(m_device);

		return VB;
	}

	BufferMemory VulkanCore::CreateIndexBuffer(const void* pIndices, size_t Size, bool rt) {
		// Step 1: create a staging buffer
		VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags MemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		BufferMemory StagingIB = CreateBuffer(Size, Usage, MemProps);

		// Step 2: map the memory of the staging buffer
		void* pMem = NULL;
		VkDeviceSize Offset = 0;
		VkMemoryMapFlags Flags = 0;
		VkResult res = vkMapMemory(m_device, StagingIB.m_mem, Offset,
			StagingIB.m_allocationSize, Flags, &pMem);
		CHECK_VK_RESULT(res, "vkMapMemory\n");

		// Step 3: copy the indices to the staging buffer
		memcpy(pMem, pIndices, Size);

		// Step 4: unmap/release the mapped memory
		vkUnmapMemory(m_device, StagingIB.m_mem);

		// Step 5: create the final index buffer
		Usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT ;
		if (rt) {
			Usage = Usage | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		}
		MemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT ;
		BufferMemory IB = CreateBuffer(Size, Usage, MemProps, rt);

		// Step 6: copy the staging buffer to the final buffer
		CopyBufferToBuffer(IB.m_buffer, StagingIB.m_buffer, Size);

		// Step 7: release the resources of the staging buffer
		StagingIB.Destroy(m_device);

		return IB;
	}

	/*
		A lo mejor (muy probablemente) luego haya que cambiar el usage a storagebuffer para poder pasarlo como buffer uniforme al shader
	*/


	BufferMemory VulkanCore::CreateNormalBuffer(const std::vector<glm::vec3>& nrmls, bool rt) {
		size_t Size = nrmls.size() * sizeof(glm::vec3);

		VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags MemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		BufferMemory StagingBuffer = CreateBuffer(Size, Usage, MemProps);

		// Step 2: map the memory of the stage buffer
		void* pMem = NULL;
		VkDeviceSize Offset = 0;
		VkMemoryMapFlags Flags = 0;
		VkResult res = vkMapMemory(m_device, StagingBuffer.m_mem, Offset,
			StagingBuffer.m_allocationSize, Flags, &pMem);
		CHECK_VK_RESULT(res, "vkMapMemory\n");

		// Step 3: copy the normals to the staging buffer
		memcpy(pMem, nrmls.data(), Size);

		// Step 4: unmap/release the mapped memory
		vkUnmapMemory(m_device, StagingBuffer.m_mem);

		// Step 5: create the final buffer
		Usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if (rt) {
			Usage = Usage | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		}
		MemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		BufferMemory NormalBuffer = CreateBuffer(Size, Usage, MemProps, rt);

		// Step 6: copy the staging buffer to the final buffer
		CopyBufferToBuffer(NormalBuffer.m_buffer, StagingBuffer.m_buffer, Size);

		// Step 7: release the resources of the staging buffer
		StagingBuffer.Destroy(m_device);

		return NormalBuffer;
	}

	BufferMemory VulkanCore::CreateUVBuffer(const std::vector<glm::vec2>& uv, bool rt) {
		size_t Size = uv.size() * sizeof(glm::vec2);

		VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags MemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
		BufferMemory StagingBuffer = CreateBuffer(Size, Usage, MemProps);

		// Step 2: map the memory of the stage buffer
		void* pMem = NULL;
		VkDeviceSize Offset = 0;
		VkMemoryMapFlags Flags = 0;
		VkResult res = vkMapMemory(m_device, StagingBuffer.m_mem, Offset,
			StagingBuffer.m_allocationSize, Flags, &pMem);
		CHECK_VK_RESULT(res, "vkMapMemory\n");

		// Step 3: copy the UV coordinates to the staging buffer
		memcpy(pMem, uv.data(), Size);

		// Step 4: unmap/release the mapped memory
		vkUnmapMemory(m_device, StagingBuffer.m_mem);

		// Step 5: create the final buffer
		Usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		if (rt) {
			Usage = Usage | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		}
		MemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		BufferMemory UVBuffer = CreateBuffer(Size, Usage, MemProps, rt);

		// Step 6: copy the staging buffer to the final buffer
		CopyBufferToBuffer(UVBuffer.m_buffer, StagingBuffer.m_buffer, Size);

		// Step 7: release the resources of the staging buffer
		StagingBuffer.Destroy(m_device);

		return UVBuffer;
	}

	BufferMemory VulkanCore::CreateBufferBlas(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags) {
		return CreateBuffer(size, usage, flags, true);
	}
	BufferMemory VulkanCore::CreateBufferACC(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags) {
		return CreateBuffer(size, usage, flags, false);
	}

	BufferMemory VulkanCore::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, bool rt) {
		VkBufferCreateInfo vbCreateInfo = {};
		vbCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vbCreateInfo.size = Size;
		vbCreateInfo.usage = Usage;
		//Cambiar esto a concurrente si necesario
		vbCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		BufferMemory Buf;

		// Step 1: create a buffer
		VkResult res = vkCreateBuffer(m_device, &vbCreateInfo, NULL, &Buf.m_buffer);
		CHECK_VK_RESULT(res, "vkCreateBuffer\n");
		printf("Buffer created\n");

		// Step 2: get the buffer memory requirements
		VkMemoryRequirements MemReqs = { 0 };
		vkGetBufferMemoryRequirements(m_device, Buf.m_buffer, &MemReqs);
		printf("Buffer requires %d bytes\n", (int)MemReqs.size);

		Buf.m_allocationSize = MemReqs.size;

		// Step 3: get the memory type index
		uint32_t MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, Properties);
		//printf("Memory type index %d\n", MemoryTypeIndex);



		// Step 4: allocate memory
		VkMemoryAllocateInfo MemAllocInfo = {};
		MemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		MemAllocInfo.pNext = NULL;
		MemAllocInfo.allocationSize = MemReqs.size;
		MemAllocInfo.memoryTypeIndex = MemoryTypeIndex;

		VkMemoryAllocateFlagsInfo allocFlagsInfo = {};
		if (Usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
			allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
			allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
			MemAllocInfo.pNext = &allocFlagsInfo;
		}

		res = vkAllocateMemory(m_device, &MemAllocInfo, NULL, &Buf.m_mem);
		CHECK_VK_RESULT(res, "vkAllocateMemory error %d\n");

		// Step 5: bind memory
		res = vkBindBufferMemory(m_device, Buf.m_buffer, Buf.m_mem, 0);
		CHECK_VK_RESULT(res, "vkBindBufferMemory error %d\n");

		return Buf;
	}

	uint32_t VulkanCore::GetMemoryTypeIndex(uint32_t MemTypeBitsMask, VkMemoryPropertyFlags ReqMemPropFlags)
	{
		const VkPhysicalDeviceMemoryProperties& MemProps = m_physDevices.Selected().m_memProps;

		for (uint i = 0; i < MemProps.memoryTypeCount; i++) {
			const VkMemoryType& MemType = MemProps.memoryTypes[i];
			uint CurBitmask = (1 << i);
			bool IsCurMemTypeSupported = (MemTypeBitsMask & CurBitmask);
			bool HasRequiredMemProps = ((MemType.propertyFlags & ReqMemPropFlags) == ReqMemPropFlags);

			if (IsCurMemTypeSupported && HasRequiredMemProps) {
				return i;
			}
		}

		printf("Cannot find memory type for type %x requested mem props %x\n", MemTypeBitsMask, ReqMemPropFlags);
		exit(1);
		return -1;
	}

	void VulkanCore::CopyBufferToBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size)
	{
		BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferCopy BufferCopy = {};
		BufferCopy.srcOffset = 0;
		BufferCopy.dstOffset = 0;
		BufferCopy.size = Size;
		

		vkCmdCopyBuffer(m_copyCmdBuf, Src, Dst, 1, &BufferCopy);

		SubmitCopyCommand();
	}

	void BufferMemory::Destroy(VkDevice Device)
	{
		if (m_mem) {
			vkFreeMemory(Device, m_mem, NULL);
		}
		if (m_buffer) {
			vkDestroyBuffer(Device, m_buffer, NULL);
		}
	}

	std::vector<BufferMemory> VulkanCore::CreateUniformBuffers(size_t Size) {

		//Puedes pasar vector como referencia para ahorrar espacio
		std::vector<BufferMemory> UniformBuffers;

		UniformBuffers.resize(m_numImages);
		for (int i = 0; i < UniformBuffers.size(); i++) {
			UniformBuffers[i] = CreateUniformBuffer(Size);
			printf("UniformBufferCreado\n");
		}

		return UniformBuffers;

	}


	BufferMemory VulkanCore::CreateUniformBuffer(size_t Size) {

		BufferMemory Buffer;

		VkBufferUsageFlags Usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		VkMemoryPropertyFlags Memprops = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		//El mismo createBuffer de antes
		Buffer = CreateBuffer(Size, Usage, Memprops);
		return Buffer;
	}

	void BufferMemory::Update(VkDevice Device, const void* pData, size_t Size) {

		void* pMem = NULL;
		VkResult res = vkMapMemory(Device, m_mem, 0, Size, 0, &pMem);
		CHECK_VK_RESULT(res, "vkMapMemory\n");
		memcpy(pMem, pData, Size);
		vkUnmapMemory(Device, m_mem);
	}

	void VulkanCore::CreateTexture(const char* pFilename, VulkanTexture& Tex)
	{
		int ImageWidth = 0;
		int ImageHeight = 0;
		int ImageChannels = 0;

		stbi_set_flip_vertically_on_load(1);

		// Step #1: load the image pixels
		stbi_uc* pPixels = stbi_load(pFilename, &ImageWidth, &ImageHeight, &ImageChannels, STBI_rgb_alpha);

		if (!pPixels) {
			printf("Error loading texture from '%s'\n", pFilename);
			exit(1);
		}

		// Step #2: create the image object and populate it with pixels
		VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;
		CreateTextureImageFromData(Tex, pPixels, ImageWidth, ImageHeight, Format);

		// Step #3: release the image pixels. We don't need them after this point
		stbi_image_free(pPixels);

		// Step #4: create the image view
		VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		Tex.m_view = CreateImageView(m_device, Tex.m_image, Format, AspectFlags);

		VkFilter MinFilter = VK_FILTER_LINEAR;
		VkFilter MaxFilter = VK_FILTER_LINEAR;
		VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		// Step #5: create the texture sampler
		Tex.m_sampler = CreateTextureSampler(m_device, MinFilter, MaxFilter, AddressMode);

		printf("Texture from '%s' created\n", pFilename);
	}


	void VulkanCore::CreateTextureFromData(const void* pPixels, int ImageWidth, int ImageHeight, VulkanTexture& Tex)
	{
		// Step #1: create the image object and populate it with pixels
		VkFormat Format = VK_FORMAT_R8G8B8A8_SRGB;
		CreateTextureImageFromData(Tex, pPixels, ImageWidth, ImageHeight, Format);

		// Step #2: create the image view
		VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		Tex.m_view = CreateImageView(m_device, Tex.m_image, Format, AspectFlags);

		VkFilter MinFilter = VK_FILTER_LINEAR;
		VkFilter MaxFilter = VK_FILTER_LINEAR;
		VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		// Step #3: create the texture sampler
		Tex.m_sampler = CreateTextureSampler(m_device, MinFilter, MaxFilter, AddressMode);

		printf("Texture from data created\n");
	}


	void VulkanTexture::Destroy(VkDevice Device)
	{
		if (m_sampler)
			printf("\nDestroying sampler\n");
			vkDestroySampler(Device, m_sampler, NULL);
		if(m_view)
			vkDestroyImageView(Device, m_view, NULL);
		if(m_image)
			vkDestroyImage(Device, m_image, NULL);
		if(m_mem)
			vkFreeMemory(Device, m_mem, NULL);
	}

	void VulkanCore::CreateTextureImageFromData(VulkanTexture& Tex, const void* pPixels,
		uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TexFormat, uint32_t bpp)
	{
		VkImageUsageFlagBits Usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_SAMPLED_BIT);
		//device local es en la gpu
		VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		CreateImage(Tex, ImageWidth, ImageHeight, TexFormat, Usage, PropertyFlags);

		UpdateTextureImage(Tex, ImageWidth, ImageHeight, TexFormat, pPixels, bpp);
	}
	void VulkanCore::CreateTextureImage(VulkanTexture& Tex, uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TexFormat) {
		VkImageUsageFlagBits Usage = (VkImageUsageFlagBits)(VK_IMAGE_USAGE_TRANSFER_DST_BIT |
			VK_IMAGE_USAGE_STORAGE_BIT| VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		//device local es en la gpu
		VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		CreateImage(Tex, ImageWidth, ImageHeight, TexFormat, Usage, PropertyFlags);

		VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		Tex.m_view = CreateImageView(m_device, Tex.m_image, TexFormat, AspectFlags);
	}

	void VulkanCore::CreateTexture(uint8_t* texels, uint32_t width, uint32_t height, uint32_t bpp, VulkanTexture& Tex)
	{
		if (!texels) {
			printf("Error: texel data is null\n");
			exit(1);
		}

		// Step #1: create the image object and populate it with pixels
		VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;
		CreateTextureImageFromData(Tex, texels, width, height, Format, bpp);

		// Step #2: create the image view
		VkImageAspectFlags AspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;
		Tex.m_view = CreateImageView(m_device, Tex.m_image, Format, AspectFlags);

		VkFilter MinFilter = VK_FILTER_LINEAR;
		VkFilter MaxFilter = VK_FILTER_LINEAR;
		VkSamplerAddressMode AddressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

		// Step #3: create the texture sampler
		Tex.m_sampler = CreateTextureSampler(m_device, MinFilter, MaxFilter, AddressMode);

		printf("Texture created from texel data (%dx%d, %d bpp)\n", width, height, bpp);
	}

	void VulkanCore::CreateImage(VulkanTexture& Tex, uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TexFormat,
		VkImageUsageFlags UsageFlags, VkMemoryPropertyFlagBits PropertyFlags)
	{
		VkImageCreateInfo ImageInfo = {};
		ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		ImageInfo.pNext = NULL;
		ImageInfo.flags = 0;
		ImageInfo.imageType = VK_IMAGE_TYPE_2D;
		ImageInfo.format = TexFormat;
		VkExtent3D ex = {};
		ex.width = ImageWidth;
		ex.height = ImageHeight;
		ex.depth = 1;
		ImageInfo.extent = ex;
		ImageInfo.mipLevels = 1;
		ImageInfo.arrayLayers = 1;
		ImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		ImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageInfo.usage = UsageFlags;
		ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		ImageInfo.queueFamilyIndexCount = 0;
		ImageInfo.pQueueFamilyIndices = NULL;
		ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;


		// Step #1: create the image object
		VkResult res = vkCreateImage(m_device, &ImageInfo, NULL, &Tex.m_image);
		CHECK_VK_RESULT(res, "vkCreateImage error");

		// Step 2: get the buffer memory requirements
		VkMemoryRequirements MemReqs = { 0 };
		vkGetImageMemoryRequirements(m_device, Tex.m_image, &MemReqs);
		printf("Image requires %d bytes\n", (int)MemReqs.size);

		// Step 3: get the memory type index
		uint32_t MemoryTypeIndex = GetMemoryTypeIndex(MemReqs.memoryTypeBits, PropertyFlags);
		printf("Memory type index %d\n", MemoryTypeIndex);

		// Step 4: allocate memory
		VkMemoryAllocateInfo MemAllocInfo = {};
		MemAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		MemAllocInfo.pNext = NULL;
		MemAllocInfo.allocationSize = MemReqs.size;
		MemAllocInfo.memoryTypeIndex = MemoryTypeIndex;

		res = vkAllocateMemory(m_device, &MemAllocInfo, NULL, &Tex.m_mem);
		CHECK_VK_RESULT(res, "vkAllocateMemory error");

		// Step 5: bind memory
		res = vkBindImageMemory(m_device, Tex.m_image, Tex.m_mem, 0);
		CHECK_VK_RESULT(res, "vkBindBufferMemory error %d\n");
	}


	void VulkanCore::UpdateTextureImage(VulkanTexture& Tex, uint32_t ImageWidth, uint32_t ImageHeight,
		VkFormat TexFormat, const void* pPixels, uint32_t bpp)
	{
		int BytesPerPixel;
		if (bpp > 99999)
			BytesPerPixel = GetBytesPerTexFormat(TexFormat);
		else
			BytesPerPixel = bpp;

		VkDeviceSize LayerSize = ImageWidth * ImageHeight * BytesPerPixel;
		int LayerCount = 1;
		VkDeviceSize ImageSize = LayerCount * LayerSize;

		VkBufferUsageFlags Usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		VkMemoryPropertyFlags Properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		BufferMemory StagingTex = CreateBuffer(ImageSize, Usage, Properties);

		StagingTex.Update(m_device, pPixels, ImageSize);

		TransitionImageLayout(Tex.m_image, TexFormat,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		CopyBufferToImage(Tex.m_image, StagingTex.m_buffer, ImageWidth, ImageHeight);

		TransitionImageLayout(Tex.m_image, TexFormat,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		StagingTex.Destroy(m_device);
	}


	void VulkanCore::TransitionImageLayout(VkImage& Image, VkFormat Format,
		VkImageLayout OldLayout, VkImageLayout NewLayout)
	{
		BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		ImageMemBarrier(m_copyCmdBuf, Image, Format, OldLayout, NewLayout);

		SubmitCopyCommand();
	}
	
	void VulkanCore::CopyBufferToImage(VkImage Dst, VkBuffer Src, uint32_t ImageWidth, uint32_t ImageHeight)
	{
		BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferImageCopy BufferImageCopy = {};
		BufferImageCopy.bufferOffset = 0;
		BufferImageCopy.bufferRowLength = 0;
		BufferImageCopy.bufferImageHeight = 0;
		BufferImageCopy.imageSubresource = VkImageSubresourceLayers {
			VK_IMAGE_ASPECT_COLOR_BIT,//.aspectMask 
			0,//.mipLevel 
			0,//.baseArrayLayer 
			1//.layerCount 
		};
		BufferImageCopy.imageOffset = VkOffset3D {0,0,0};
		BufferImageCopy.imageExtent = VkExtent3D {ImageWidth,ImageHeight,1 };


		vkCmdCopyBufferToImage(m_copyCmdBuf, Src, Dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &BufferImageCopy);
		SubmitCopyCommand();
	}

	void VulkanCore::SubmitCopyCommand()
	{
		vkEndCommandBuffer(m_copyCmdBuf);

		m_queue.SubmitSync(m_copyCmdBuf);

		m_queue.WaitIdle();
	}

	void VulkanCore::CreateDepthResources()
	{
		int NumSwapChainImages = (int)m_numImages;

		m_depthImages.resize(NumSwapChainImages);

		VkFormat DepthFormat = m_physDevices.Selected().m_depthFormat;

	
		for (int i = 0; i < NumSwapChainImages; i++) {
			VkImageUsageFlagBits Usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			CreateImage(m_depthImages[i], m_offscreenWidth, m_offscreenHeight, DepthFormat,
				Usage, PropertyFlags);

			VkImageLayout OldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			VkImageLayout NewLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			TransitionImageLayout(m_depthImages[i].m_image, DepthFormat, OldLayout, NewLayout);

			m_depthImages[i].m_view = CreateImageView(m_device, m_depthImages[i].m_image,
				DepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		}
	}

	// M�todo para copiar los contenidos de una VulkanTexture a un buffer
	size_t VulkanCore::copyResultBytes(uint8_t* buffer, size_t bufferSize, VulkanTexture* tex, int width, int height) {
		if (!tex || !tex->m_image || !buffer) {
			return 0;
		}

		// Obtener las propiedades de la imagen
		VkImageSubresource subresource = {};
		subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresource.mipLevel = 0;
		subresource.arrayLayer = 0;

		VkSubresourceLayout layout;
		vkGetImageSubresourceLayout(m_device, tex->m_image, &subresource, &layout);

		// Verificar si el buffer es suficientemente grande
		if (bufferSize < layout.size) {
			printf("Buffer size insufficient. Required: %zu, Available: %zu\n",
				(size_t)layout.size, bufferSize);
			return 0;
		}

		// Crear un buffer de staging para copiar la imagen
		VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

		BufferMemory stagingBuffer = CreateBuffer(layout.size, usage, properties);

		// Transicionar la imagen al layout apropiado para lectura
		TransitionImageLayout(tex->m_image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		// Copiar la imagen al buffer de staging
		CopyImageToBuffer(tex->m_image, stagingBuffer.m_buffer, layout, width, height);

		// Transicionar la imagen de vuelta al layout original
		TransitionImageLayout(tex->m_image, VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// Mapear la memoria y copiar al buffer de destino
		void* pMappedMemory = nullptr;
		VkResult res = vkMapMemory(m_device, stagingBuffer.m_mem, 0, layout.size, 0, &pMappedMemory);
		if (res != VK_SUCCESS) {
			printf("Error mapping memory: %d\n", res);
			stagingBuffer.Destroy(m_device);
			return 0;
		}

		memcpy(buffer, pMappedMemory, layout.size);
		vkUnmapMemory(m_device, stagingBuffer.m_mem);

		// Limpiar el buffer de staging
		stagingBuffer.Destroy(m_device);

		return (size_t)layout.size;
	}

	// M�todo auxiliar para copiar imagen a buffer
	void VulkanCore::CopyImageToBuffer(VkImage srcImage, VkBuffer dstBuffer, const VkSubresourceLayout& layout, int width, int height) {
		BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };

		// Necesitamos conocer las dimensiones de la imagen
		// Esto podr�a requerir almacenar width/height en VulkanTexture
		// Por ahora, asumimos que podemos obtenerlas de alguna manera
		region.imageExtent = { (uint32_t)layout.rowPitch / 4, // Asumiendo 4 bytes por pixel
							  (uint32_t)(layout.size / layout.rowPitch),
							  1 };

		vkCmdCopyImageToBuffer(m_copyCmdBuf, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			dstBuffer, 1, &region);

		SubmitCopyCommand();
	}

	/*
		OFFSCREEN RENDERING
	*/

	void VulkanCore::CreateOffscreenImages() {
		// Solo necesitamos 1 imagen para offscreen
		m_images.resize(1);
		m_imageViews.resize(1);

		// Crear imagen offscreen
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = m_offscreenWidth;
		imageInfo.extent.height = m_offscreenHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM; // Formato fijo
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult res = vkCreateImage(m_device, &imageInfo, nullptr, &m_images[0]);
		CHECK_VK_RESULT(res, "vkCreateImage offscreen");

		// Allocar memoria
		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(m_device, m_images[0], &memReqs);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		res = vkAllocateMemory(m_device, &allocInfo, nullptr, &m_offscreenImageMemory);
		CHECK_VK_RESULT(res, "vkAllocateMemory offscreen");

		res = vkBindImageMemory(m_device, m_images[0], m_offscreenImageMemory, 0);
		CHECK_VK_RESULT(res, "vkBindImageMemory offscreen");

		// Crear image view
		m_imageViews[0] = CreateImageView(m_device, m_images[0], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

		// Establecer formato para render pass
		m_swapChainSurfaceFormat.format = VK_FORMAT_R8G8B8A8_UNORM;
		m_swapChainSurfaceFormat.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

		printf("Offscreen image created\n");
	}
	// 5. NUEVA FUNCI�N: Guardar imagen offscreen a archivo
	void VulkanCore::SaveOffscreenImage(const char* filename) {
		VkDeviceSize imageSize = m_offscreenWidth * m_offscreenHeight * 4; // 4 bytes per pixel

		// Crear staging buffer
		BufferMemory stagingBuffer = CreateBuffer(imageSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// Transition image layout para transfer
		TransitionImageLayout(m_images[0], VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		// Copiar imagen a buffer
		BeginCommandBuffer(m_copyCmdBuf, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { m_offscreenWidth, m_offscreenHeight, 1 };

		vkCmdCopyImageToBuffer(m_copyCmdBuf, m_images[0], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			stagingBuffer.m_buffer, 1, &region);

		SubmitCopyCommand();

		// Leer datos
		void* data;
		VkResult res = vkMapMemory(m_device, stagingBuffer.m_mem, 0, imageSize, 0, &data);
		CHECK_VK_RESULT(res, "vkMapMemory");

		// Aqu� puedes guardar a archivo usando stb_image_write o similar
		stbi_write_png(filename, m_offscreenWidth, m_offscreenHeight, 4, data, m_offscreenWidth * 4);

		vkUnmapMemory(m_device, stagingBuffer.m_mem);
		stagingBuffer.Destroy(m_device);

		printf("Offscreen image saved to %s\n", filename);
	}

}