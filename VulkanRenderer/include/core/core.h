#pragma once
#include <vulkan/vulkan_core.h>

#include "core/core_utils.h"
#include "core/physical_device.h"
#include "core/core_wrapper.h"
#include "core/core_queue.h"
#include "core/utils.h"
#include <glm/ext.hpp>

#include "3rdParty/stb_image.h"
#include "3rdParty/stb_image_write.h"
namespace core {
	class BufferMemory {
	public:
		BufferMemory() {}

		VkBuffer m_buffer = NULL;
		VkDeviceMemory m_mem = NULL;
		VkDeviceSize m_allocationSize = 0;

		void Destroy(VkDevice device);
		void Update(VkDevice Device, const void* pData, size_t Size);
	};

	class VulkanTexture {
	public:
		VulkanTexture() {}

		VkImage m_image = VK_NULL_HANDLE;
		VkDeviceMemory m_mem = VK_NULL_HANDLE;
		VkImageView m_view = VK_NULL_HANDLE;
		VkSampler m_sampler = VK_NULL_HANDLE;

		uint32_t id = 0;

		void Destroy(VkDevice Device); 
	};

	class VulkanCore {
	public:
		VulkanCore();
		~VulkanCore();
		void Init(const char* pAppName, uint32_t width, uint32_t height);

		void CreateCommandBuffer(uint32_t count, VkCommandBuffer* cmdBufs);
		void FreeCommandBuffers(uint32_t count, const VkCommandBuffer* pCmdBufs);
		int GetNumImages() const { return (int)m_images.size(); }
		const VkImage& GetImage(int Index) const;
		core::PhysicalDevice GetSelectedPhysicalDevice();
		VulkanQueue* GetQueue() { return &m_queue; }
		VkRenderPass CreateSimpleRenderPass();
		VkDevice& GetDevice() { return m_device; }
		VkCommandPool GetCommandPool() { return m_cmdBufPool; }
		std::vector<VkFramebuffer> CreateFrameBuffers(VkRenderPass RenderPass);
		BufferMemory CreateVertexBuffer(const void* pVertices, size_t Size, bool rt = false);
		BufferMemory CreateIndexBuffer(const void* pIndices, size_t Size, bool rt = false);
		BufferMemory CreateNormalBuffer(const std::vector<glm::vec3>& nrmls, bool rt = false);
		BufferMemory CreateUVBuffer(const std::vector<glm::vec2>& uv, bool rt = false);
		//BufferMemory CreateSimpleVertexBuffer(const void* pVertices, size_t Size);
		std::vector<BufferMemory> CreateUniformBuffers(size_t Size);
		void CreateTexture(const char* filename, VulkanTexture& Tex);
		void CreateTexture(uint8_t* texels, uint32_t width, uint32_t height, uint32_t bpp, VulkanTexture& Tex);
		void CreateTextureImage(VulkanTexture& Tex, uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TexFormat);
		BufferMemory CreateBufferBlas(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
		BufferMemory CreateBufferACC(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);

		void TransitionImageLayout(VkImage& Image, VkFormat Format, VkImageLayout OldLayout, VkImageLayout NewLayout);
		size_t copyResultBytes(uint8_t* buffer, size_t bufferSize, VulkanTexture* tex, int width, int height);
		void SaveOffscreenImage(const char* filename);
		void CopyBufferToBuffer(VkBuffer Dst, VkBuffer Src, VkDeviceSize Size);
		//GLFW DEPRECATED
		//void Init(const char* pAppName, GLFWwindow* pWindow);

	private:
		void CreateInstance(const char* pAppName);
		void CreateDebugCallback();
		//void CreateSurface(GLFWwindow* pWindow);
		void CreateDevice();
		void CreateSwapChain();
		void CreateCommandBufferPool();
		
		BufferMemory CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, bool rt=false);
		uint32_t GetMemoryTypeIndex(uint32_t MemTypeBitsMask, VkMemoryPropertyFlags ReqMemPropFlags);

		BufferMemory CreateUniformBuffer(size_t Size);
		void CreateTextureFromData(const void* pPixels, int ImageWidth, int ImageHeight, VulkanTexture& Tex);
		void CreateTextureImageFromData(VulkanTexture& Tex, const void* pPixels, uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TexFormat, uint32_t bpp = 100000);
		void UpdateTextureImage(VulkanTexture& Tex, uint32_t ImageWidth, uint32_t ImageHeight,VkFormat TexFormat, const void* pPixels, uint32_t bpp = 1000000);
		void CreateImage(VulkanTexture& Tex, uint32_t ImageWidth, uint32_t ImageHeight, VkFormat TexFormat,VkImageUsageFlags UsageFlags, VkMemoryPropertyFlagBits PropertyFlags);
		
		
		void CopyBufferToImage(VkImage Dst, VkBuffer Src, uint32_t ImageWidth, uint32_t ImageHeight);
		void SubmitCopyCommand();
		void CreateDepthResources();
		void CopyImageToBuffer(VkImage srcImage, VkBuffer dstBuffer, const VkSubresourceLayout& layout, int width, int height);

		void CreateOffscreenImages();
		

		VkInstance m_instance = NULL;
		VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
		
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VulkanPhysicalDevices m_physDevices;
		uint32_t m_queueFamily = 0;
		VkDevice m_device = VK_NULL_HANDLE;
		VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
		VkSurfaceFormatKHR m_swapChainSurfaceFormat;
		//ImageView -> acceso
		//Image -> Imagen real
		std::vector<VkImage> m_images;
		std::vector<VkImageView> m_imageViews;
	
		VkCommandPool m_cmdBufPool = VK_NULL_HANDLE;
		VulkanQueue m_queue;
		std::vector<VkFramebuffer> m_frameBuffers;
		VkCommandBuffer m_copyCmdBuf;
	
		//No necesita ni memoria ni sampler
		std::vector<core::VulkanTexture> m_depthImages;

		//Offscreen renderer
		uint32_t m_offscreenWidth, m_offscreenHeight;
		VkDeviceMemory m_offscreenImageMemory;
		uint32_t m_numImages = 0;

		//GLFW deprecated
		//GLFWwindow* m_pWindow = NULL;
	};


}