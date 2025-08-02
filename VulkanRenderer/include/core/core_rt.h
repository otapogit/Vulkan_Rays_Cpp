#pragma once
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan_core.h>

#include <vector>
#include "core/core.h"
#include "core/core_simple_mesh.h"
#include "core/core_vertex.h"
#include "3rdParty/stb_image_write.h"

#include <cassert>

namespace core {

	struct BlasInput
	{
		// Data used to build acceleration structure geometry
		std::vector<VkAccelerationStructureGeometryKHR>       asGeometry;
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> asBuildOffsetInfo;
		VkBuildAccelerationStructureFlagsKHR                  flags{ 0 };
		BufferMemory										m_transBuffer;
	};
	
	struct TlasInput {
		std::vector<VkAccelerationStructureInstanceKHR> instances;
		VkBuildAccelerationStructureFlagsKHR flags = 0;
	};


	struct AccelerationStructureBuildData
	{
		VkAccelerationStructureTypeKHR asType = VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR;  // Mandatory to set

		// Collection of geometries for the acceleration structure.
		std::vector<VkAccelerationStructureGeometryKHR> asGeometry;
		// Build range information corresponding to each geometry.
		std::vector<VkAccelerationStructureBuildRangeInfoKHR> rangeInfo;
		// Build information required for acceleration structure.
		VkAccelerationStructureBuildGeometryInfoKHR buildInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR };
		// Size information for acceleration structure build resources.
		VkAccelerationStructureBuildSizesInfoKHR sizeInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR };

		// Adds a geometry with its build range information to the acceleration structure.
		//void addGeometry(const VkAccelerationStructureGeometryKHR& asGeom, const VkAccelerationStructureBuildRangeInfoKHR& offset);
		//void addGeometry(const AccelerationStructureGeometryInfo& asGeom);
		// Configures the build information and calculates the necessary size information.
		VkAccelerationStructureBuildSizesInfoKHR finalizeGeometry(VkDevice device, VkBuildAccelerationStructureFlagsKHR flags, PFN_vkGetAccelerationStructureBuildSizesKHR pfnGetBuildSizes);
	};
	struct AccelerationStructure {
		VkAccelerationStructureKHR handle = VK_NULL_HANDLE;
		core::BufferMemory buffer;
		VkDeviceAddress address = 0;
	};

	// Single Geometry information, multiple can be used in a single BLAS
	struct AccelerationStructureGeometryInfo
	{
		VkAccelerationStructureGeometryKHR       geometry{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
		VkAccelerationStructureBuildRangeInfoKHR rangeInfo{};
	};
	class Raytracer {
	public:
		Raytracer();
		~Raytracer();
		// #VKRay
		void initRayTracing(core::PhysicalDevice physdev, VkDevice* dev);
		void setup( VkCommandPool pool, core::VulkanCore* core);
		void createBottomLevelAS(std::vector<core::SimpleMesh> meshes);
		void createTopLevelAS();




		// Método helper para limpiar recursos
		void cleanup() {
			if (m_tlas.handle != VK_NULL_HANDLE) {
				vkDestroyAccelerationStructureKHR(*m_device, m_tlas.handle, nullptr);
				vkDestroyBuffer(*m_device, m_tlas.buffer.m_buffer, nullptr);
				vkFreeMemory(*m_device, m_tlas.buffer.m_mem, nullptr);
			}

			if (m_instBuffer.m_buffer != VK_NULL_HANDLE) {
				vkDestroyBuffer(*m_device, m_instBuffer.m_buffer, nullptr);
				vkFreeMemory(*m_device, m_instBuffer.m_mem, nullptr);
			}
			for (auto& blas : m_blas) {
				if (blas.handle != VK_NULL_HANDLE) {
					vkDestroyAccelerationStructureKHR(m_device[0], blas.handle, nullptr);
				}
				//implementar destroybuffer o simplemente
				vkDestroyBuffer(m_device[0], blas.buffer.m_buffer, NULL);
				vkFreeMemory(*m_device, blas.buffer.m_mem, nullptr);
			}
			m_blas.clear();
			for (uint32_t i = 0; i < allBlas.size(); i++) {
				vkDestroyBuffer(*m_device, allBlas[i].m_transBuffer.m_buffer, NULL);
				vkFreeMemory(*m_device, allBlas[i].m_transBuffer.m_mem, nullptr);
			}
			allBlas.clear();
			CleanupMvpDescriptorSet();
			CleanupGeometryDescriptorSet();

			vkDestroyDescriptorPool(*m_device, m_rtDescPool, nullptr);
			vkDestroyDescriptorSetLayout(*m_device, m_rtDescSetLayout, nullptr);
			m_outTexture->Destroy(*m_device);
		}
		void createRtDescriptorSet();
		void createMvpDescriptorSet();
		void UpdateMvpMatrix(const glm::mat4& mvpMatrix);

		void createRtPipeline(VkShaderModule rgenModule, VkShaderModule rmissModule, VkShaderModule rchitModule);
		void createRtShaderBindingTable();
		void raytrace(VkCommandBuffer cmdBuf, int width, int height);
		void render(int width, int height, bool saveImage = false, const std::string& filename = "");

		void createOutImage(int windowwidth, int windowheight, VulkanTexture* tex);
		void UpdateAccStructure();

		void createGeometryDescriptorSet(int maxsize = 10);
		void updateGeometryDescriptorSet(std::vector<core::SimpleMesh> meshes);
		size_t copyResultBytes(uint8_t* buffer, size_t bufferSize, VulkanTexture* tex, int width, int height);

	private:



		void loadRayTracingFunctions();
		auto objectToVkGeometryKHR(const core::SimpleMesh& model);
		void buildBlas(std::vector<core::BlasInput>& input, VkBuildAccelerationStructureFlagsKHR flags);
		void buildTlas(const std::vector<VkAccelerationStructureInstanceKHR>& instances,
			VkBuildAccelerationStructureFlagsKHR flags);
		
		
		void CreateRtDescriptorPool(int NumImages);
		void CreateRtDescriptorSetLayout();
		void AllocateRtDescriptorSet();
		void WriteAccStructure();
		
		void CreateMvpDescriptorPool(int NumImages);
		void CreateMvpDescriptorSetLayout();
		void AllocateMvpDescriptorSet();
		void CreateMvpBuffer();
		void WriteMvpBuffer();

		void CreateGeometryDescriptorPool(int numMeshes);
		void CreateGeometryDescriptorSetLayout(int maxsize);
		void AllocateGeometryDescriptorSet();
		// Ya los tengo
		void CreateGeometryBuffers(std::vector<core::SimpleMesh> meshes);
		void WriteGeometryDescriptorSet();
		void CleanupGeometryDescriptorSet();
		

		void saveImageToPNG(const std::string& filename, int width, int height);
		void createStagingBuffer(VkDeviceSize size, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

		void CleanupMvpDescriptorSet() {
			m_mvpBufferMemory.Destroy(*m_device);

			if (m_mvpDescSetLayout != VK_NULL_HANDLE) {
				vkDestroyDescriptorSetLayout(*m_device, m_mvpDescSetLayout, nullptr);
			}
			if (m_mvpDescPool != VK_NULL_HANDLE) {
				vkDestroyDescriptorPool(*m_device, m_mvpDescPool, nullptr);
			}
		}

		VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rtProperties{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
		core::PhysicalDevice m_physicaldevice;
		VkDevice* m_device;
		VkCommandPool m_cmdBufPool;
		// Nuevos miembros para almacenar las BLAS creadas
		std::vector<core::BlasInput> allBlas;
		std::vector<AccelerationStructure> m_blas;

		core::AccelerationStructure m_tlas;
		core::BufferMemory m_instBuffer; // Buffer para las instancias

		VulkanCore* m_vkcore;

		core::VulkanTexture* m_outTexture;

		int windowwidth, windowheight;

		// Ray tracing function pointers
		PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
		PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
		PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
		PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
		PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
		PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR;
		PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
		PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR;
		PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR;

		//Descriptor Sets
		//nvvk::DescriptorSetBindings                     m_rtDescSetLayoutBind;
		VkDescriptorPool                                m_rtDescPool;
		VkDescriptorSetLayout                           m_rtDescSetLayout;
		VkDescriptorSet                                 m_rtDescSet;
		std::vector<VkDescriptorSet>					m_rtDescSets;

		VkDescriptorPool m_mvpDescPool;
		VkDescriptorSetLayout m_mvpDescSetLayout;
		VkDescriptorSet m_mvpDescSet;
		BufferMemory m_mvpBufferMemory;

		// Geometry descriptor set
		VkDescriptorPool m_geometryDescPool;
		VkDescriptorSetLayout m_geometryDescSetLayout;
		VkDescriptorSet m_geometryDescSet;

		// Buffers para geometry data
		std::vector<BufferMemory> m_vertexBuffers;
		std::vector<BufferMemory> m_indexBuffers;
		std::vector<BufferMemory> m_normalBuffers;
		BufferMemory m_textureIndexBuffer;
		BufferMemory m_colorBuffer;
		std::vector<VulkanTexture*> m_textures;

		int m_maxsize = 10;

		std::vector<glm::vec4> colors = {};

		VkPipeline m_rtPipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_rtPipelineLayout = VK_NULL_HANDLE;
		std::vector<VkRayTracingShaderGroupCreateInfoKHR> m_rtShaderGroups;

		// Shader Binding Table
		core::BufferMemory m_rtSBTBuffer;
		VkStridedDeviceAddressRegionKHR m_rgenRegion{};
		VkStridedDeviceAddressRegionKHR m_missRegion{};
		VkStridedDeviceAddressRegionKHR m_hitRegion{};
		VkStridedDeviceAddressRegionKHR m_callRegion{};

	};


}
