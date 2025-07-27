#include "core/core_rt.h"
#include "core/utils.h"
#include "core/core_shader.h"
#include <array>

namespace core {
    //--------------------------------------------------------------------------------------------------
    // Initialize Vulkan ray tracing
    // #VKRay

    Raytracer::Raytracer() {}

    Raytracer::~Raytracer() {
    }

    /*
    * Metodo para pasar todos los componentes necesarios del core aqui
    * Quiero mantener encapsulado de cierta manera el raytracing
    *
    * Componentes necesarios por ahora: Core, MEshes, tama�o textura de salida
    */
    void Raytracer::setup(VkCommandPool pool, core::VulkanCore* core) {

        m_cmdBufPool = pool;
        m_vkcore = core;
        loadRayTracingFunctions();
        m_outTexture = new core::VulkanTexture();
        createOutImage(800, 800, m_outTexture);
    }


    void Raytracer::initRayTracing(core::PhysicalDevice physdev, VkDevice* dev)
    {
        m_device = dev;
        // Requesting ray tracing properties
        VkPhysicalDeviceProperties2 prop2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
        prop2.pNext = &m_rtProperties;
        vkGetPhysicalDeviceProperties2(physdev.m_physDevice, &prop2);
    }

#pragma region GeometryBuild



    //--------------------------------------------------------------------------------------------------
// Convert an OBJ model into the ray tracing geometry used to build the BLAS
//
    auto Raytracer::objectToVkGeometryKHR(const core::SimpleMesh& model)
    {

        core::BlasInput input;

        // BLAS builder requires raw device addresses.
        VkDeviceAddress vertexAddress = GetBufferDeviceAddress(*m_device, model.m_vb.m_buffer);
        VkDeviceAddress indexAddress = GetBufferDeviceAddress(*m_device, model.m_indexbuffer.m_buffer);

        uint32_t maxPrimitiveCount = (uint32_t)(model.vertexcount / 3);

        // Describe buffer as array of VertexObj.
        VkAccelerationStructureGeometryTrianglesDataKHR triangles{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR };
        triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;  // vec3 vertex position data.
        triangles.vertexData.deviceAddress = vertexAddress;
        triangles.vertexStride = sizeof(core::VertexObj);
        // Describe index data (32-bit unsigned int)
        triangles.indexType = VK_INDEX_TYPE_UINT32;
        triangles.indexData.deviceAddress = indexAddress;
        // Indicate identity transform by setting transformData to null device pointer.
        triangles.transformData = {};
        triangles.maxVertex = (uint32_t)(model.m_vertexBufferSize - 1);
        /*if (model.m_transMat != glm::mat4(1.0f)) {
            // Handle transformation matrix
            VkDeviceAddress transformAddress = 0;
            BufferMemory transformBuffer;
            // Vulkan expects a 3x4 matrix(transpose of upper 3x4 of the 4x4 matrix)
            // Format: [m00, m10, m20, m01, m11, m21, m02, m12, m22, m03, m13, m23]
            float transformMatrix[12] = {
                model.m_transMat[0][0], model.m_transMat[1][0], model.m_transMat[2][0], // First column
                model.m_transMat[0][1], model.m_transMat[1][1], model.m_transMat[2][1], // Second column  
                model.m_transMat[0][2], model.m_transMat[1][2], model.m_transMat[2][2], // Third column
                model.m_transMat[0][3], model.m_transMat[1][3], model.m_transMat[2][3]  // Fourth column (translation)
            };
            
            // Create buffer for transformation matrix
            VkBufferUsageFlags usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

            transformBuffer = m_vkcore->CreateBufferACC(sizeof(transformMatrix), usage, memProps);
            // Map and copy transform data
            void* pMem = nullptr;
            VkResult res = vkMapMemory(*m_device, transformBuffer.m_mem, 0, sizeof(transformMatrix), 0, &pMem);
            CHECK_VK_RESULT(res, "vkMapMemory transform\n");
            memcpy(pMem, transformMatrix, sizeof(transformMatrix));
            vkUnmapMemory(*m_device, transformBuffer.m_mem);

            // Step 1: Create staging buffer
            VkBufferUsageFlags stagingUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            VkMemoryPropertyFlags stagingMemProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            BufferMemory stagingBuffer = m_vkcore->CreateBufferACC(sizeof(transformMatrix), stagingUsage, stagingMemProps);

            // Step 2: Map the memory of the staging buffer
            void* pMem = nullptr;
            VkDeviceSize offset = 0;
            VkMemoryMapFlags flags = 0;
            VkResult res = vkMapMemory(*m_device, stagingBuffer.m_mem, offset,
                stagingBuffer.m_allocationSize, flags, &pMem);
            CHECK_VK_RESULT(res, "vkMapMemory transform staging\n");

            // Step 3: Copy the transformation matrix to the staging buffer
            memcpy(pMem, transformMatrix, sizeof(transformMatrix));

            // Step 4: Unmap/release the mapped memory
            vkUnmapMemory(*m_device, stagingBuffer.m_mem);

            // Step 5: Create the final buffer
            VkBufferUsageFlags finalUsage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
            VkMemoryPropertyFlags finalMemProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            transformBuffer = m_vkcore->CreateBufferACC(sizeof(transformMatrix), finalUsage, finalMemProps);

            // Step 6: Copy the staging buffer to the final buffer
            m_vkcore->CopyBufferToBuffer(transformBuffer.m_buffer, stagingBuffer.m_buffer, sizeof(transformMatrix));

            // Step 7: Release the resources of the staging buffer
            stagingBuffer.Destroy(*m_device);

            transformAddress = GetBufferDeviceAddress(*m_device, transformBuffer.m_buffer);
            triangles.transformData.deviceAddress = transformAddress;
            input.m_transBuffer = transformBuffer;
            printf("%zd", transformAddress);
        }*/

        // Identify the above data as containing opaque triangles.
        VkAccelerationStructureGeometryKHR asGeom{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR };
        asGeom.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        asGeom.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        asGeom.geometry.triangles = triangles;

        // The entire array will be used to build the BLAS.
        VkAccelerationStructureBuildRangeInfoKHR offset;
        offset.firstVertex = 0;
        offset.primitiveCount = maxPrimitiveCount;
        offset.primitiveOffset = 0;
        offset.transformOffset = 0;

        // Our blas is made from only one geometry, but could be made of many geometries
  
        input.asGeometry.emplace_back(asGeom);
        input.asBuildOffsetInfo.emplace_back(offset);
        
        printf("BLAS created for %d triangles\n", maxPrimitiveCount);

        return input;
    }

    // Tambi�n necesitar�s actualizar tu m�todo createBottomLevelAS:
    void Raytracer::createBottomLevelAS(std::vector<core::SimpleMesh> meshes) {
        // BLAS - Storing each primitive in a geometry
        m_blas.clear();
        allBlas.clear();
        allBlas.reserve(meshes.size());

        printf("\n");

        for (const core::SimpleMesh& obj : meshes) {
            //Da error aqui
            auto blas = objectToVkGeometryKHR(obj);
            allBlas.emplace_back(blas);
            printf("color en createBLAS: %f %f %f\n", obj.color.r, obj.color.g, obj.color.b);
        }

        //printf("size of allblas: %d\n", allBlas.size());
        // Ahora puedes llamar a tu implementaci�n
        buildBlas(allBlas, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
    }

    void Raytracer::buildBlas(std::vector<core::BlasInput>& input, VkBuildAccelerationStructureFlagsKHR flags) {
        uint32_t nbBlas = static_cast<uint32_t>(input.size());
        VkDeviceSize maxScratchSize{ 0 };

        // Preparar la informaci�n para los comandos de construcci�n de acceleration structures
        std::vector<core::AccelerationStructureBuildData> buildAs(nbBlas);
        std::vector<VkAccelerationStructureBuildSizesInfoKHR> buildSizes(nbBlas);

        // 1. Preparar informaci�n de construcci�n para cada BLAS
        for (uint32_t idx = 0; idx < nbBlas; idx++) {
            buildAs[idx].asType = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
            buildAs[idx].asGeometry = input[idx].asGeometry;
            buildAs[idx].rangeInfo = input[idx].asBuildOffsetInfo;

            // Finalizar geometr�a y obtener informaci�n de tama�os
            buildSizes[idx] = buildAs[idx].finalizeGeometry(*m_device, input[idx].flags | flags, vkGetAccelerationStructureBuildSizesKHR);
            maxScratchSize = std::max(maxScratchSize, buildSizes[idx].buildScratchSize);
        }

        // 2. Crear buffer de scratch


        core::BufferMemory blasScratchBuffer = m_vkcore[0].CreateBufferBlas(maxScratchSize, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);

        // Obtener la direcci�n del device del buffer de scratch
        VkDeviceAddress scratchAddress = GetBufferDeviceAddress(*m_device, blasScratchBuffer.m_buffer);
        m_blas.clear();
        // 3. Crear y construir cada BLAS
        m_blas.resize(nbBlas);

        for (uint32_t idx = 0; idx < nbBlas; idx++) {
            // Crear buffer para almacenar la acceleration structure


            core::BufferMemory asBuffer = m_vkcore[0].CreateBufferBlas(buildSizes[idx].accelerationStructureSize,
                VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            // Crear la acceleration structure
            VkAccelerationStructureCreateInfoKHR createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
            createInfo.buffer = asBuffer.m_buffer;
            createInfo.size = buildSizes[idx].accelerationStructureSize;
            createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;


            VkAccelerationStructureKHR accelerationStructure;
            VkResult result = vkCreateAccelerationStructureKHR(*m_device, &createInfo, nullptr, &accelerationStructure);
            if (result != VK_SUCCESS) {
                throw std::runtime_error("Failed to create acceleration structure");
            }

            // Guardar en tu estructura de datos (asumiendo que tienes una estructura para almacenar BLAS)
            core::AccelerationStructure blas;
            blas.handle = accelerationStructure;
            blas.buffer = asBuffer;

            // Obtener la direcci�n de la acceleration structure
            VkAccelerationStructureDeviceAddressInfoKHR addressInfo = {};
            addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
            addressInfo.accelerationStructure = accelerationStructure;
            blas.address = vkGetAccelerationStructureDeviceAddressKHR(*m_device, &addressInfo);

            m_blas[idx] = blas;

            // 4. Construir la acceleration structure
            VkCommandBufferAllocateInfo allocInfo = {};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool = m_cmdBufPool;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            vkAllocateCommandBuffers(*m_device, &allocInfo, &commandBuffer);

            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            // Configurar la informaci�n de construcci�n
            buildAs[idx].buildInfo.dstAccelerationStructure = accelerationStructure;
            buildAs[idx].buildInfo.scratchData.deviceAddress = scratchAddress;

            // Preparar punteros a la informaci�n de rangos
            std::vector<VkAccelerationStructureBuildRangeInfoKHR*> buildRangeInfos(buildAs[idx].rangeInfo.size());
            for (size_t i = 0; i < buildAs[idx].rangeInfo.size(); i++) {
                buildRangeInfos[i] = &buildAs[idx].rangeInfo[i];
            }

            // Construir la acceleration structure
            vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildAs[idx].buildInfo, buildRangeInfos.data());

            // A�adir barrier para asegurar que la construcci�n termine antes de la siguiente
            VkMemoryBarrier barrier = {};
            barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
            barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
            barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                0, 1, &barrier, 0, nullptr, 0, nullptr);

            vkEndCommandBuffer(commandBuffer);

            // Submit y esperar
            VkSubmitInfo submitInfo = {};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &commandBuffer;

  
            core::VulkanQueue* m_pQueue = m_vkcore->GetQueue();

            m_pQueue->SubmitSync(commandBuffer);
            m_pQueue->WaitIdle();

            // Liberar command buffer
            vkFreeCommandBuffers(*m_device, m_cmdBufPool, 1, &commandBuffer);
        }

        printf("Tama�o m_blas: %d", m_blas.size());

        // 5. Limpiar buffer de scratch
        vkDestroyBuffer(*m_device, blasScratchBuffer.m_buffer, NULL);

    }

    static VkTransformMatrixKHR toTransformMatrixKHR(glm::mat4 matrix)
    {
        // VkTransformMatrixKHR uses a row-major memory layout, while glm::mat4
        // uses a column-major memory layout. We transpose the matrix so we can
        // memcpy the matrix's data directly.
        glm::mat4            temp = glm::transpose(matrix);
        VkTransformMatrixKHR out_matrix;
        memcpy(&out_matrix, &temp, sizeof(VkTransformMatrixKHR));
        return out_matrix;
    }

    void Raytracer::createTopLevelAS()
    {
        std::vector<VkAccelerationStructureInstanceKHR> instances;

        printf("\n=== DEBUG TLAS CREATION ===\n");
        printf("Number of BLAS: %zu\n", m_blas.size());
        // Crear instancia para cada BLAS
        for (size_t i = 0; i < m_blas.size(); i++) {
            VkAccelerationStructureInstanceKHR instance{};
            memset(&instance, 0, sizeof(VkAccelerationStructureInstanceKHR));
            instance.transform = toTransformMatrixKHR(glm::mat4(1.0f));// Matriz de transformaci�n (identidad por defecto)
            instance.instanceCustomIndex = static_cast<uint32_t>(i);// �ndice personalizado de la instancia (accessible en shaders como gl_InstanceCustomIndexEXT)
            instance.accelerationStructureReference = m_blas[i].address;// Referencia a la BLAS
            instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR; // Flags de la instancia
            instance.mask = 0xFF;// M�scara para ray culling (0xFF significa que todos los rays pueden intersectar)
            instance.instanceShaderBindingTableRecordOffset = 0; // Offset en la shader binding table para hit shaders
            instances.push_back(instance);

            printf("Instance %zu:\n", i);
            printf("  instanceCustomIndex = %u\n", instance.instanceCustomIndex);
            printf("  accelerationStructureReference = 0x%llx\n", instance.accelerationStructureReference);
            printf("  mask = 0x%02x\n", instance.mask);
            printf("  flags = 0x%08x\n", instance.flags);
            printf("\n");
        }
        printf("\n%zd instances pre build\n", instances.size());

        buildTlas(instances, VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR);
    }

    void Raytracer::buildTlas(const std::vector<VkAccelerationStructureInstanceKHR>& instances,
        VkBuildAccelerationStructureFlagsKHR flags)
    {
        // 1. Crear buffer para las instancias
        VkDeviceSize instanceBufferSize = instances.size() * sizeof(VkAccelerationStructureInstanceKHR);

        m_instBuffer = m_vkcore[0].CreateBufferBlas(
            instanceBufferSize,
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
            VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
        );

        // 2. Copiar datos de instancias al buffer
        void* data;
        vkMapMemory(*m_device, m_instBuffer.m_mem, 0, instanceBufferSize, 0, &data);
        memcpy(data, instances.data(), instanceBufferSize);
        vkUnmapMemory(*m_device, m_instBuffer.m_mem);

        // 3. Configurar la geometr�a de instancias
        VkAccelerationStructureGeometryInstancesDataKHR instancesVk{
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR
        };
        instancesVk.arrayOfPointers = VK_FALSE;
        instancesVk.data.deviceAddress = GetBufferDeviceAddress(*m_device, m_instBuffer.m_buffer);

        printf("Building TLAS\n");

        // 4. Configurar la geometr�a
        VkAccelerationStructureGeometryKHR topASGeometry{
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR
        };
        topASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        topASGeometry.geometry.instances = instancesVk;

        // 5. Preparar informaci�n de construcci�n
        core::AccelerationStructureBuildData buildData;
        buildData.asType = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        buildData.asGeometry.push_back(topASGeometry);

        VkAccelerationStructureBuildRangeInfoKHR buildOffsetInfo{};
        buildOffsetInfo.primitiveCount = static_cast<uint32_t>(instances.size());
        buildOffsetInfo.primitiveOffset = 0;
        buildOffsetInfo.firstVertex = 0;
        buildOffsetInfo.transformOffset = 0;
        buildData.rangeInfo.push_back(buildOffsetInfo);

        // 6. Obtener informaci�n de tama�os
        VkAccelerationStructureBuildSizesInfoKHR sizeInfo = buildData.finalizeGeometry(
            *m_device, flags, vkGetAccelerationStructureBuildSizesKHR
        );

        // 7. Crear buffer de scratch
        core::BufferMemory scratchBuffer = m_vkcore[0].CreateBufferBlas(
            sizeInfo.buildScratchSize,
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
        );
        VkDeviceAddress scratchAddress = GetBufferDeviceAddress(*m_device, scratchBuffer.m_buffer);

        // 8. Crear buffer para la TLAS
        core::BufferMemory tlasBuffer = m_vkcore[0].CreateBufferBlas(
            sizeInfo.accelerationStructureSize,
            VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        // 9. Crear la acceleration structure
        VkAccelerationStructureCreateInfoKHR createInfo{
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR
        };
        createInfo.buffer = tlasBuffer.m_buffer;
        createInfo.size = sizeInfo.accelerationStructureSize;
        createInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

        VkResult result = vkCreateAccelerationStructureKHR(*m_device, &createInfo, nullptr, &m_tlas.handle);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create top level acceleration structure");
        }

        m_tlas.buffer = tlasBuffer;

        // 10. Obtener la direcci�n de la TLAS
        VkAccelerationStructureDeviceAddressInfoKHR addressInfo{
            VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR
        };
        addressInfo.accelerationStructure = m_tlas.handle;
        m_tlas.address = vkGetAccelerationStructureDeviceAddressKHR(*m_device, &addressInfo);

        // 11. Construir la TLAS

        VkCommandBuffer commandBuffer;
        m_vkcore->CreateCommandBuffer(1, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
        };
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        // Configurar la informaci�n de construcci�n
        buildData.buildInfo.dstAccelerationStructure = m_tlas.handle;
        buildData.buildInfo.scratchData.deviceAddress = scratchAddress;

        // Preparar punteros a la informaci�n de rangos
        VkAccelerationStructureBuildRangeInfoKHR* pBuildRangeInfo = &buildData.rangeInfo[0];

        // Construir la acceleration structure
        vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildData.buildInfo, &pBuildRangeInfo);

        // A�adir barrier
        VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
        barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
        barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
            0, 1, &barrier, 0, nullptr, 0, nullptr);

        vkEndCommandBuffer(commandBuffer);

        // Submit y esperar
        core::VulkanQueue* pQueue = m_vkcore[0].GetQueue();
        pQueue->SubmitSync(commandBuffer);
        pQueue->WaitIdle();

        // Limpiar
        vkFreeCommandBuffers(*m_device, m_cmdBufPool, 1, &commandBuffer);
        vkDestroyBuffer(*m_device, scratchBuffer.m_buffer, nullptr);
        vkFreeMemory(*m_device, scratchBuffer.m_mem, nullptr);

        printf("TLAS created with %zd instances\n", instances.size());
    }

    VkAccelerationStructureBuildSizesInfoKHR AccelerationStructureBuildData::finalizeGeometry(VkDevice device, VkBuildAccelerationStructureFlagsKHR flags, PFN_vkGetAccelerationStructureBuildSizesKHR pfnGetBuildSizes)
    {
        assert(asGeometry.size() > 0 && "No geometry added to Build Structure");
        assert(asType != VK_ACCELERATION_STRUCTURE_TYPE_MAX_ENUM_KHR && "Acceleration Structure Type not set");

        buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        buildInfo.type = asType;
        buildInfo.flags = flags;
        buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;
        buildInfo.dstAccelerationStructure = VK_NULL_HANDLE;
        buildInfo.geometryCount = static_cast<uint32_t>(asGeometry.size());
        buildInfo.pGeometries = asGeometry.data();
        buildInfo.ppGeometries = nullptr;
        buildInfo.scratchData.deviceAddress = 0;

        std::vector<uint32_t> maxPrimCount(rangeInfo.size());
        for (size_t i = 0; i < rangeInfo.size(); ++i)
        {
            maxPrimCount[i] = rangeInfo[i].primitiveCount;
        }

        pfnGetBuildSizes(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo,
            maxPrimCount.data(), &sizeInfo);

        return sizeInfo;
    }
#pragma endregion

#pragma region RtDescsets



    void Raytracer::createRtDescriptorSet() {
        CreateRtDescriptorPool(1);
        printf("Creating RT descriptor set layout\n");
        CreateRtDescriptorSetLayout();
        //IMPORTANTE, A ESTE DESCRIPTOR SET SE LE DEBER� A�ADIR EL OTRO DESCRIPTOR SET DE INFO GENERAL DE LA ESCENA PARA QUE VAYA OK :)
        printf("Allocating RT Descriptor set\n");
        AllocateRtDescriptorSet();
        printf("Writing RT Descriptor set\n");
        WriteAccStructure();
    }

    void Raytracer::AllocateRtDescriptorSet() {
        VkDescriptorSetAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocateInfo.descriptorPool = m_rtDescPool;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts = &m_rtDescSetLayout;
        vkAllocateDescriptorSets(*m_device, &allocateInfo, &m_rtDescSet);
    }


    void Raytracer::CreateRtDescriptorPool(int NumImages) {

        std::vector<VkDescriptorPoolSize> poolSizes;

        // Pool para uniform buffers
        VkDescriptorPoolSize uniformPoolSize = {};
        uniformPoolSize.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        uniformPoolSize.descriptorCount = (uint32_t)NumImages;
        poolSizes.push_back(uniformPoolSize);

        // Pool para texturas
        VkDescriptorPoolSize samplerPoolSize = {};
        samplerPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        samplerPoolSize.descriptorCount = (uint32_t)NumImages;
        poolSizes.push_back(samplerPoolSize);

        VkDescriptorPoolCreateInfo PoolInfo = {};
        PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        //Para alocar cosas en el otro metodo cambiar flags
        PoolInfo.flags = 0;
        //Esto puede ser cambiado para pasar mas cosas
        PoolInfo.maxSets = (uint32_t)NumImages;
        //A lo mejor hay que cambiar las pools para poner las cosas en especifico tipo uniforms y tal
        PoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
        PoolInfo.pPoolSizes = poolSizes.data();

        VkResult res = vkCreateDescriptorPool(*m_device, &PoolInfo, NULL, &m_rtDescPool);
        CHECK_VK_RESULT(res, "vkCreateDescriptorPool");
        printf("Descriptor pool created\n");

    }

    void Raytracer::CreateRtDescriptorSetLayout() {

        std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;

        VkDescriptorSetLayoutBinding AccStructureLayoutBinding_Uniform = {};
        AccStructureLayoutBinding_Uniform.binding = 1;
        AccStructureLayoutBinding_Uniform.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        AccStructureLayoutBinding_Uniform.descriptorCount = 1;
        //Obviamente si es necesario ampliar esto
        AccStructureLayoutBinding_Uniform.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;


        LayoutBindings.push_back(AccStructureLayoutBinding_Uniform);

        // CORREGIDO: Usar la variable correcta
        VkDescriptorSetLayoutBinding FragmentShaderLayoutBinding = {};
        FragmentShaderLayoutBinding.binding = 2;
        FragmentShaderLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        FragmentShaderLayoutBinding.descriptorCount = 1;
        FragmentShaderLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

        LayoutBindings.push_back(FragmentShaderLayoutBinding);


        VkDescriptorSetLayoutCreateInfo LayoutInfo = {};
        LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        LayoutInfo.pNext = NULL;
        LayoutInfo.flags = 0;
        LayoutInfo.bindingCount = (uint32_t)LayoutBindings.size();
        LayoutInfo.pBindings = LayoutBindings.data();

        VkResult res = vkCreateDescriptorSetLayout(*m_device, &LayoutInfo, NULL, &m_rtDescSetLayout);
        CHECK_VK_RESULT(res, "vkCreateDescriptorSetLayout\n");

    }

    void Raytracer::UpdateAccStructure(){
        std::vector<VkWriteDescriptorSet> WriteDescriptorSet;
        //solo hay un m_rtDescSet
        VkAccelerationStructureKHR tlas = m_tlas.handle;
        VkWriteDescriptorSetAccelerationStructureKHR descASInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR };
        descASInfo.accelerationStructureCount = 1;
        descASInfo.pAccelerationStructures = &tlas;
        VkWriteDescriptorSet wds_t = {};
        wds_t.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds_t.dstSet = m_rtDescSet;
        wds_t.dstBinding = 1;
        wds_t.dstArrayElement = 0;
        wds_t.descriptorCount = 1;
        wds_t.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        wds_t.pNext = &descASInfo;
        WriteDescriptorSet.push_back(wds_t);

        vkUpdateDescriptorSets(*m_device, (uint32_t)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
    }

    void Raytracer::WriteAccStructure() {
        std::vector<VkWriteDescriptorSet> WriteDescriptorSet;
        //solo hay un m_rtDescSet
       /* VkAccelerationStructureKHR tlas = m_tlas.handle;
        VkWriteDescriptorSetAccelerationStructureKHR descASInfo{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR };
        descASInfo.accelerationStructureCount = 1;
        descASInfo.pAccelerationStructures = &tlas;
        VkWriteDescriptorSet wds_t = {};
        wds_t.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds_t.dstSet = m_rtDescSet;
        wds_t.dstBinding = 1;
        wds_t.dstArrayElement = 0;
        wds_t.descriptorCount = 1;
        wds_t.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        wds_t.pNext = &descASInfo;
        WriteDescriptorSet.push_back(wds_t);*/

        //Esto a lo mejor hay que cambiarlo xq no voy a samplear nada, es de output
        VkDescriptorImageInfo ImageInfo = {};
        ImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        ImageInfo.imageView = m_outTexture->m_view;
        ImageInfo.sampler = NULL;
        VkWriteDescriptorSet wds_i = {};
        wds_i.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        wds_i.dstSet = m_rtDescSet;
        wds_i.dstBinding = 2;
        wds_i.dstArrayElement = 0;
        wds_i.descriptorCount = 1;
        wds_i.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        wds_i.pImageInfo = &ImageInfo;
        WriteDescriptorSet.push_back(wds_i);
       
        vkUpdateDescriptorSets(*m_device, (uint32_t)WriteDescriptorSet.size(), WriteDescriptorSet.data(), 0, NULL);
    }
#pragma endregion

#pragma region MVPDescsets



    void Raytracer::createMvpDescriptorSet() {
        CreateMvpDescriptorPool(1);
        printf("Creating MVP descriptor set layout\n");
        CreateMvpDescriptorSetLayout();
        printf("Creating MVP Buffer\n");
        CreateMvpBuffer();
        printf("Allocating MVP Descriptor set\n");
        AllocateMvpDescriptorSet();
        printf("Writing MVP Descriptor set\n");
        WriteMvpBuffer();
    }
    // Crear el pool de descriptores para MVP
    void Raytracer::CreateMvpDescriptorPool(int NumImages) {
        std::vector<VkDescriptorPoolSize> poolSizes;

        // Pool para uniform buffer (MVP matrix)
        VkDescriptorPoolSize uniformPoolSize = {};
        uniformPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformPoolSize.descriptorCount = (uint32_t)NumImages;
        poolSizes.push_back(uniformPoolSize);

        VkDescriptorPoolCreateInfo PoolInfo = {};
        PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        PoolInfo.flags = 0;
        PoolInfo.maxSets = (uint32_t)NumImages;
        PoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
        PoolInfo.pPoolSizes = poolSizes.data();

        VkResult res = vkCreateDescriptorPool(*m_device, &PoolInfo, NULL, &m_mvpDescPool);
        CHECK_VK_RESULT(res, "vkCreateDescriptorPool MVP");
        printf("MVP Descriptor pool created\n");
    }

    // Crear el layout del descriptor set para MVP
    void Raytracer::CreateMvpDescriptorSetLayout() {
        std::vector<VkDescriptorSetLayoutBinding> LayoutBindings;

        VkDescriptorSetLayoutBinding MvpLayoutBinding = {};
        MvpLayoutBinding.binding = 1; // Binding 0 para la matriz MVP
        MvpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        MvpLayoutBinding.descriptorCount = 1;
        // Puedes usar VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_RAYGEN_BIT_KHR dependiendo de d�nde lo uses
        MvpLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

        LayoutBindings.push_back(MvpLayoutBinding);

        VkDescriptorSetLayoutCreateInfo LayoutInfo = {};
        LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        LayoutInfo.pNext = NULL;
        LayoutInfo.flags = 0;
        LayoutInfo.bindingCount = (uint32_t)LayoutBindings.size();
        LayoutInfo.pBindings = LayoutBindings.data();

        VkResult res = vkCreateDescriptorSetLayout(*m_device, &LayoutInfo, NULL, &m_mvpDescSetLayout);
        CHECK_VK_RESULT(res, "vkCreateDescriptorSetLayout MVP");
    }

    // Alocar el descriptor set para MVP
    void Raytracer::AllocateMvpDescriptorSet() {
        VkDescriptorSetAllocateInfo allocateInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        allocateInfo.descriptorPool = m_mvpDescPool;
        allocateInfo.descriptorSetCount = 1;
        allocateInfo.pSetLayouts = &m_mvpDescSetLayout;

        VkResult res = vkAllocateDescriptorSets(*m_device, &allocateInfo, &m_mvpDescSet);
        CHECK_VK_RESULT(res, "vkAllocateDescriptorSets MVP");
    }

    // Crear el buffer para la matriz MVP
    void Raytracer::CreateMvpBuffer() {
        VkDeviceSize bufferSize = sizeof(glm::mat4); // Asumiendo que usas glm::mat4

        m_mvpBufferMemory = m_vkcore->CreateBufferACC(
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        printf("MVP Buffer created successfully\n");
    }

    // Escribir/actualizar el buffer MVP
    void Raytracer::WriteMvpBuffer() {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_mvpBufferMemory.m_buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(glm::mat4);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_mvpDescSet;
        descriptorWrite.dstBinding = 1;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(*m_device, 1, &descriptorWrite, 0, nullptr);
    }

    // M�todo para actualizar la matriz MVP en runtime
    void Raytracer::UpdateMvpMatrix(const glm::mat4& mvpMatrix) {
        m_mvpBufferMemory.Update(*m_device, &mvpMatrix, sizeof(glm::mat4));
    }
#pragma endregion

#pragma region GeometryDescsets

    void Raytracer::createGeometryDescriptorSet( int maxsize) {
        m_maxsize = maxsize;
        CreateGeometryDescriptorPool(maxsize);
        printf("Creating Geometry layout\n");
        CreateGeometryDescriptorSetLayout(maxsize);
        printf("Allocating layout\n");
        AllocateGeometryDescriptorSet();

    }
    void Raytracer::CreateGeometryDescriptorPool(int numMeshes) {
        // Calculamos el n�mero de descriptors necesarios
        // Por cada mesh: vertex buffer + index buffer + normal buffer + texture index buffer + color + texture sampler
        std::vector<VkDescriptorPoolSize> poolSizes = {
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, static_cast<uint32_t>(numMeshes * 5)}, // vertex, index, normal, texture index, color
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(numMeshes)} // texturas
        };

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();

        if (vkCreateDescriptorPool(*m_device, &poolInfo, nullptr, &m_geometryDescPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create geometry descriptor pool!");
        }
    }

    void Raytracer::CreateGeometryDescriptorSetLayout(int maxsize) {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        // Binding 0: Array de vertex buffers
        VkDescriptorSetLayoutBinding vertexBinding{};
        vertexBinding.binding = 0;
        vertexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        vertexBinding.descriptorCount = maxsize;
        vertexBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        vertexBinding.pImmutableSamplers = nullptr;
        bindings.push_back(vertexBinding);

        // Binding 1: Array de index buffers
        VkDescriptorSetLayoutBinding indexBinding{};
        indexBinding.binding = 1;
        indexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        indexBinding.descriptorCount = maxsize;
        indexBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        indexBinding.pImmutableSamplers = nullptr;
        bindings.push_back(indexBinding);

        // Binding 2: Array de normal buffers
        VkDescriptorSetLayoutBinding normalBinding{};
        normalBinding.binding = 2;
        normalBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        normalBinding.descriptorCount = maxsize;
        normalBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        normalBinding.pImmutableSamplers = nullptr;
        bindings.push_back(normalBinding);

        // Binding 3: Array de texture index buffers
        VkDescriptorSetLayoutBinding textureIndexBinding{};
        textureIndexBinding.binding = 3;
        textureIndexBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        textureIndexBinding.descriptorCount = 1;
        textureIndexBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        textureIndexBinding.pImmutableSamplers = nullptr;
        bindings.push_back(textureIndexBinding);

        VkDescriptorSetLayoutBinding colorBinding{};
        colorBinding.binding = 4;
        colorBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        colorBinding.descriptorCount = 1;
        colorBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        colorBinding.pImmutableSamplers = nullptr;
        bindings.push_back(colorBinding);

        // Binding 4: Array de texturas
        VkDescriptorSetLayoutBinding textureBinding{};
        textureBinding.binding = 5;
        textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureBinding.descriptorCount = maxsize;
        textureBinding.stageFlags = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        textureBinding.pImmutableSamplers = nullptr;
        bindings.push_back(textureBinding);

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        if (vkCreateDescriptorSetLayout(*m_device, &layoutInfo, nullptr, &m_geometryDescSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create geometry descriptor set layout!");
        }
    }

    void Raytracer::AllocateGeometryDescriptorSet() {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_geometryDescPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_geometryDescSetLayout;

        if (vkAllocateDescriptorSets(*m_device, &allocInfo, &m_geometryDescSet) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate geometry descriptor set!");
        }
    }

    void Raytracer::CreateGeometryBuffers(std::vector<core::SimpleMesh> meshes) {
        // Limpiar buffers existentes
        for (auto& buffer : m_vertexBuffers) {
            buffer.Destroy(*m_device);
        }
        for (auto& buffer : m_indexBuffers) {
            buffer.Destroy(*m_device);
        }
        for (auto& buffer : m_normalBuffers) {
            buffer.Destroy(*m_device);
        }
        
        m_textureIndexBuffer.Destroy(*m_device);
        m_colorBuffer.Destroy(*m_device);


        m_vertexBuffers.clear();
        m_indexBuffers.clear();
        m_normalBuffers.clear();
        m_textureIndexBuffer = {};
        m_colorBuffer = {};

        std::vector<int> texindexes = {};
        std::vector<glm::vec3> colors = {};


        for (const core::SimpleMesh& mesh : meshes) {
            m_vertexBuffers.push_back(mesh.m_vb);
            m_indexBuffers.push_back(mesh.m_indexbuffer);
            m_normalBuffers.push_back(mesh.m_normalbuffer);
            texindexes.push_back(mesh.texIndex);
            colors.push_back(mesh.color);
            printf("Mesh color: %d %d %d\n", mesh.color.r, mesh.color.g, mesh.color.b);
            printf("Mesh texindex: %d\n", mesh.texIndex);

        }


        if (!colors.empty()) {
            printf("Colors was empty\n");
        }
        else {
            printf("Size of colors: %d", colors.size());
        }

        m_textureIndexBuffer = m_vkcore->CreateBufferBlas(sizeof(int) * texindexes.size(), 
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

        // Mapear y escribir el texture index
        void* data;
        vkMapMemory(*m_device, m_textureIndexBuffer.m_mem, 0, sizeof(int) * texindexes.size(), 0, &data);
        memcpy(data, &texindexes, sizeof(int)*texindexes.size());
        vkUnmapMemory(*m_device, m_textureIndexBuffer.m_mem);


        m_colorBuffer = m_vkcore->CreateBufferBlas(sizeof(glm::vec3) * colors.size(),
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        // Mapear y escribir el color
        void* colorData;
        vkMapMemory(*m_device, m_colorBuffer.m_mem, 0, sizeof(glm::vec3) * colors.size(), 0, &colorData);
        memcpy(colorData, &colors, sizeof(glm::vec3) * colors.size());
        vkUnmapMemory(*m_device, m_colorBuffer.m_mem);

    }

    void Raytracer::WriteGeometryDescriptorSet() {


        printf("Writing Geometry descriptor set\n");

        std::vector<VkWriteDescriptorSet> descriptorWrites;

        // Preparar buffer infos para cada tipo de buffer
        std::vector<VkDescriptorBufferInfo> vertexBufferInfos;
        std::vector<VkDescriptorBufferInfo> indexBufferInfos;
        std::vector<VkDescriptorBufferInfo> normalBufferInfos;
        std::vector<VkDescriptorBufferInfo> textureIndexBufferInfos;
        std::vector<VkDescriptorBufferInfo> colorBufferInfos;
        std::vector<VkDescriptorImageInfo> imageInfos;

        // Vertex buffers
        for (const auto& buffer : m_vertexBuffers) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = buffer.m_buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;
            vertexBufferInfos.push_back(bufferInfo);
        }

        // Index buffers
        for (const auto& buffer : m_indexBuffers) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = buffer.m_buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;
            indexBufferInfos.push_back(bufferInfo);
        }

        // Normal buffers
        for (const auto& buffer : m_normalBuffers) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = buffer.m_buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = VK_WHOLE_SIZE;
            normalBufferInfos.push_back(bufferInfo);
        }

        // Texture index buffers
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_textureIndexBuffer.m_buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;
        textureIndexBufferInfos.push_back(bufferInfo);

        // Texture index buffers
        VkDescriptorBufferInfo c_bufferInfo{};
        bufferInfo.buffer = m_colorBuffer.m_buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;
        colorBufferInfos.push_back(c_bufferInfo);
        

        // Texture samplers
        for (const auto& texture : m_textures) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture->m_view;
            imageInfo.sampler = texture->m_sampler;
            imageInfos.push_back(imageInfo);
        }

        // Write descriptor sets
        VkWriteDescriptorSet vertexWrite{};
        vertexWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        vertexWrite.dstSet = m_geometryDescSet;
        vertexWrite.dstBinding = 0;
        vertexWrite.dstArrayElement = 0;
        vertexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        vertexWrite.descriptorCount = static_cast<uint32_t>(vertexBufferInfos.size());
        vertexWrite.pBufferInfo = vertexBufferInfos.data();
        descriptorWrites.push_back(vertexWrite);

        VkWriteDescriptorSet indexWrite{};
        indexWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        indexWrite.dstSet = m_geometryDescSet;
        indexWrite.dstBinding = 1;
        indexWrite.dstArrayElement = 0;
        indexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        indexWrite.descriptorCount = static_cast<uint32_t>(indexBufferInfos.size());
        indexWrite.pBufferInfo = indexBufferInfos.data();
        descriptorWrites.push_back(indexWrite);

        VkWriteDescriptorSet normalWrite{};
        normalWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        normalWrite.dstSet = m_geometryDescSet;
        normalWrite.dstBinding = 2;
        normalWrite.dstArrayElement = 0;
        normalWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        normalWrite.descriptorCount = static_cast<uint32_t>(normalBufferInfos.size());
        normalWrite.pBufferInfo = normalBufferInfos.data();
        descriptorWrites.push_back(normalWrite);

        VkWriteDescriptorSet textureIndexWrite{};
        textureIndexWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        textureIndexWrite.dstSet = m_geometryDescSet;
        textureIndexWrite.dstBinding = 3;
        textureIndexWrite.dstArrayElement = 0;
        textureIndexWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        textureIndexWrite.descriptorCount = static_cast<uint32_t>(textureIndexBufferInfos.size());
        textureIndexWrite.pBufferInfo = textureIndexBufferInfos.data();
        descriptorWrites.push_back(textureIndexWrite);

        VkWriteDescriptorSet colorWrite{};
        colorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        colorWrite.dstSet = m_geometryDescSet;
        colorWrite.dstBinding = 4;
        colorWrite.dstArrayElement = 0;
        colorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        colorWrite.descriptorCount = static_cast<uint32_t>(colorBufferInfos.size());
        colorWrite.pBufferInfo = colorBufferInfos.data();
        descriptorWrites.push_back(colorWrite);

        VkWriteDescriptorSet textureWrite{};
        textureWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        textureWrite.dstSet = m_geometryDescSet;
        textureWrite.dstBinding = 5;
        textureWrite.dstArrayElement = 0;
        textureWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        textureWrite.descriptorCount = static_cast<uint32_t>(imageInfos.size());
        textureWrite.pImageInfo = imageInfos.data();
        //descriptorWrites.push_back(textureWrite);

        vkUpdateDescriptorSets(*m_device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }


    void Raytracer::CleanupGeometryDescriptorSet() {
        // Limpiar buffers
        /*for (auto& buffer : m_vertexBuffers) {
            buffer.Destroy(*m_device);
        }
        for (auto& buffer : m_indexBuffers) {
            buffer.Destroy(*m_device);
        }
        for (auto& buffer : m_normalBuffers) {
            buffer.Destroy(*m_device);
        }
        for (auto& buffer : m_textureIndexBuffers) {
            buffer.Destroy(*m_device);
        }*/

        m_vertexBuffers.clear();
        m_indexBuffers.clear();
        m_normalBuffers.clear();
        m_textureIndexBuffer.Destroy(*m_device);
        m_colorBuffer.Destroy(*m_device);

        // Limpiar descriptor set
        if (m_geometryDescSetLayout != VK_NULL_HANDLE) {
            vkDestroyDescriptorSetLayout(*m_device, m_geometryDescSetLayout, nullptr);
            m_geometryDescSetLayout = VK_NULL_HANDLE;
        }
        if (m_geometryDescPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(*m_device, m_geometryDescPool, nullptr);
            m_geometryDescPool = VK_NULL_HANDLE;
        }
        printf("");
    }

    void Raytracer::updateGeometryDescriptorSet(const std::vector<core::SimpleMesh>& meshes) {
        if (meshes.size() > m_maxsize) {
            printf("\nMax size of meshes exceeded, stopping program\nFor more info consult Renderer initRT: method createGeometryDescriptorSet\nCurrent maximum size is %d\n", m_maxsize);
            exit(1);
        }

        CreateGeometryBuffers(meshes);
        WriteGeometryDescriptorSet();
    }
#pragma endregion

#pragma region Utils



    // En tu archivo .cpp, implementa el m�todo:
    void Raytracer::loadRayTracingFunctions() {
        // Cargar funciones de acceleration structure
        vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
            vkGetDeviceProcAddr(*m_device, "vkCreateAccelerationStructureKHR"));

        vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
            vkGetDeviceProcAddr(*m_device, "vkDestroyAccelerationStructureKHR"));

        vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
            vkGetDeviceProcAddr(*m_device, "vkGetAccelerationStructureBuildSizesKHR"));

        vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
            vkGetDeviceProcAddr(*m_device, "vkGetAccelerationStructureDeviceAddressKHR"));

        vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
            vkGetDeviceProcAddr(*m_device, "vkCmdBuildAccelerationStructuresKHR"));

        vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(
            vkGetDeviceProcAddr(*m_device, "vkBuildAccelerationStructuresKHR"));

        // Cargar funciones de ray tracing pipeline
        vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(
            vkGetDeviceProcAddr(*m_device, "vkCmdTraceRaysKHR"));

        vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(
            vkGetDeviceProcAddr(*m_device, "vkGetRayTracingShaderGroupHandlesKHR"));

        vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(
            vkGetDeviceProcAddr(*m_device, "vkCreateRayTracingPipelinesKHR"));

        // Verificar que todas las funciones se cargaron correctamente
        if (!vkCreateAccelerationStructureKHR || !vkDestroyAccelerationStructureKHR ||
            !vkGetAccelerationStructureBuildSizesKHR || !vkGetAccelerationStructureDeviceAddressKHR ||
            !vkCmdBuildAccelerationStructuresKHR || !vkBuildAccelerationStructuresKHR ||
            !vkCmdTraceRaysKHR || !vkGetRayTracingShaderGroupHandlesKHR ||
            !vkCreateRayTracingPipelinesKHR) {
            throw std::runtime_error("Failed to load ray tracing functions!");
        }
    }   

#pragma endregion

#pragma region Pipeline&Shaders

    void Raytracer::createOutImage(int windowwidth, int windowheight, VulkanTexture* tex) {
        VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;
        m_vkcore->CreateTextureImage(*tex, (uint32_t)windowwidth, (uint32_t)windowheight, Format);
    }

    void Raytracer::createRtPipeline(VkShaderModule rgenModule, VkShaderModule rmissModule, VkShaderModule rchitModule) {
        // 1. Cargar shaders
        enum StageIndices {
            eRaygen,
            eMiss,
            eClosestHit,
            eShaderGroupCount
        };

        std::array<VkPipelineShaderStageCreateInfo, eShaderGroupCount> stages;

        //A lo mejor es interesante pasar los shaders desde el ejecutable

        // Raygen shader
        VkShaderModule raygenModule = rgenModule;
        stages[eRaygen] = {};
        stages[eRaygen].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[eRaygen].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        stages[eRaygen].module = raygenModule;
        stages[eRaygen].pName = "main";
        stages[eRaygen].pNext = nullptr;
        stages[eRaygen].flags = 0;

        // Miss shader
        VkShaderModule missModule = rmissModule;
        stages[eMiss] = {};
        stages[eMiss].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[eMiss].stage = VK_SHADER_STAGE_MISS_BIT_KHR;
        stages[eMiss].module = missModule;
        stages[eMiss].pName = "main";
        stages[eMiss].pNext = nullptr;
        stages[eMiss].flags = 0;

        // Closest hit shader
        VkShaderModule chitModule = rchitModule;
        stages[eClosestHit] = {};
        stages[eClosestHit].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        stages[eClosestHit].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        stages[eClosestHit].module = chitModule;
        stages[eClosestHit].pName = "main";
        stages[eClosestHit].pNext = nullptr;
        stages[eClosestHit].flags = 0;

        // 2. Crear shader groups
        m_rtShaderGroups.resize(eShaderGroupCount);

        // Raygen group
        m_rtShaderGroups[eRaygen].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        m_rtShaderGroups[eRaygen].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        m_rtShaderGroups[eRaygen].generalShader = eRaygen;
        m_rtShaderGroups[eRaygen].closestHitShader = VK_SHADER_UNUSED_KHR;
        m_rtShaderGroups[eRaygen].anyHitShader = VK_SHADER_UNUSED_KHR;
        m_rtShaderGroups[eRaygen].intersectionShader = VK_SHADER_UNUSED_KHR;

        // Miss group
        m_rtShaderGroups[eMiss].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        m_rtShaderGroups[eMiss].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        m_rtShaderGroups[eMiss].generalShader = eMiss;
        m_rtShaderGroups[eMiss].closestHitShader = VK_SHADER_UNUSED_KHR;
        m_rtShaderGroups[eMiss].anyHitShader = VK_SHADER_UNUSED_KHR;
        m_rtShaderGroups[eMiss].intersectionShader = VK_SHADER_UNUSED_KHR;

        // Hit group
        m_rtShaderGroups[eClosestHit].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        m_rtShaderGroups[eClosestHit].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        m_rtShaderGroups[eClosestHit].generalShader = VK_SHADER_UNUSED_KHR;
        m_rtShaderGroups[eClosestHit].closestHitShader = eClosestHit;
        m_rtShaderGroups[eClosestHit].anyHitShader = VK_SHADER_UNUSED_KHR;
        m_rtShaderGroups[eClosestHit].intersectionShader = VK_SHADER_UNUSED_KHR;

        // 3. Crear pipeline layout
        //Incluir aqui m�s sets si necesario
        std::vector<VkDescriptorSetLayout> rtDescSetLayouts = { m_rtDescSetLayout, m_mvpDescSetLayout, m_geometryDescSetLayout };
        m_rtDescSets = { m_rtDescSet, m_mvpDescSet, m_geometryDescSet };

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(rtDescSetLayouts.size());
        pipelineLayoutCreateInfo.pSetLayouts = rtDescSetLayouts.data();

        printf("Creating rt pipeline layout\n");

        VkResult result = vkCreatePipelineLayout(*m_device, &pipelineLayoutCreateInfo, nullptr, &m_rtPipelineLayout);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create ray tracing pipeline layout");
        }

        // 4. Crear el pipeline de ray tracing
        VkRayTracingPipelineCreateInfoKHR rayPipelineInfo{};
        rayPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
        rayPipelineInfo.stageCount = static_cast<uint32_t>(stages.size());
        rayPipelineInfo.pStages = stages.data();
        rayPipelineInfo.groupCount = static_cast<uint32_t>(m_rtShaderGroups.size());
        rayPipelineInfo.pGroups = m_rtShaderGroups.data();
        rayPipelineInfo.maxPipelineRayRecursionDepth = 2; // Ajustar seg�n necesidades
        rayPipelineInfo.layout = m_rtPipelineLayout;

        printf("Preparing to create RT pipeline\n");

        result = vkCreateRayTracingPipelinesKHR(*m_device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayPipelineInfo, nullptr, &m_rtPipeline);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to create ray tracing pipeline");
        }

        // 5. Limpiar m�dulos de shader
        //vkDestroyShaderModule(*m_device, raygenModule, nullptr);
        //vkDestroyShaderModule(*m_device, missModule, nullptr);
        //vkDestroyShaderModule(*m_device, chitModule, nullptr);

        printf("Ray tracing pipeline created successfully\n");
    }


    void Raytracer::createRtShaderBindingTable() {
        // 1. Obtener el tama�o de handle de shader group
        uint32_t groupCount = static_cast<uint32_t>(m_rtShaderGroups.size());
        uint32_t groupHandleSize = m_rtProperties.shaderGroupHandleSize;
        uint32_t groupSizeAligned = (groupHandleSize + m_rtProperties.shaderGroupBaseAlignment - 1) &
            ~(m_rtProperties.shaderGroupBaseAlignment - 1);

        // 2. Obtener handles de shader groups
        uint32_t sbtDataSize = groupCount * groupHandleSize;
        std::vector<uint8_t> shaderHandleStorage(sbtDataSize);

        VkResult result = vkGetRayTracingShaderGroupHandlesKHR(*m_device, m_rtPipeline, 0, groupCount,
            sbtDataSize, shaderHandleStorage.data());
        if (result != VK_SUCCESS) {
            throw std::runtime_error("Failed to get ray tracing shader group handles");
        }

        // 3. Calcular tama�os de regiones
        VkDeviceSize sbtSize = groupCount * groupSizeAligned;

        // 4. Crear buffer SBT
        m_rtSBTBuffer = m_vkcore[0].CreateBufferBlas(
            sbtSize,
            VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT
        );

        // 5. Mapear y copiar datos al buffer
        void* data;
        vkMapMemory(*m_device, m_rtSBTBuffer.m_mem, 0, sbtSize, 0, &data);

        auto* pSBTBuffer = reinterpret_cast<uint8_t*>(data);
        for (uint32_t g = 0; g < groupCount; g++) {
            memcpy(pSBTBuffer, shaderHandleStorage.data() + g * groupHandleSize, groupHandleSize);
            pSBTBuffer += groupSizeAligned;
        }

        vkUnmapMemory(*m_device, m_rtSBTBuffer.m_mem);

        // 6. Configurar regiones de SBT
        VkDeviceAddress sbtAddress = GetBufferDeviceAddress(*m_device, m_rtSBTBuffer.m_buffer);

        m_rgenRegion.deviceAddress = sbtAddress;
        m_rgenRegion.stride = groupSizeAligned;
        m_rgenRegion.size = groupSizeAligned;

        m_missRegion.deviceAddress = sbtAddress + groupSizeAligned;
        m_missRegion.stride = groupSizeAligned;
        m_missRegion.size = groupSizeAligned;

        m_hitRegion.deviceAddress = sbtAddress + 2 * groupSizeAligned;
        m_hitRegion.stride = groupSizeAligned;
        m_hitRegion.size = groupSizeAligned;

        m_callRegion = {}; // No se usa en este ejemplo

        printf("Shader binding table created successfully\n");
    }

#pragma endregion

#pragma region Rendering




    void Raytracer::raytrace(VkCommandBuffer cmdBuf, int width, int height) {
        // 1. Transici�n de imagen a layout correcto
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        imageMemoryBarrier.image = m_outTexture->m_image;
        imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

        vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

        // 2. Bind pipeline y descriptor sets
        vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipeline);
        vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, m_rtPipelineLayout,
            0,(uint32_t) m_rtDescSets.size(), m_rtDescSets.data(), 0, nullptr);

        // 3. Ejecutar ray tracing
        vkCmdTraceRaysKHR(cmdBuf, &m_rgenRegion, &m_missRegion, &m_hitRegion, &m_callRegion, width, height, 1);

        // 4. Barrier para asegurar que el ray tracing termine
        VkMemoryBarrier memoryBarrier{};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
            0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
    }



    void Raytracer::render(int width, int height, bool saveImage, const std::string& filename) {
        VkCommandBuffer cmdBuf;
        m_vkcore->CreateCommandBuffer(1, &cmdBuf);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmdBuf, &beginInfo);   

        // Ejecutar ray tracing
        raytrace(cmdBuf, width, height);

        vkEndCommandBuffer(cmdBuf);

        // Submit y esperar
        core::VulkanQueue* pQueue = m_vkcore->GetQueue();
        //Offscreen Render
        uint32_t ImageIndex = 0;

        pQueue->SubmitSync(cmdBuf);
        //pQueue->Present(ImageIndex);
        pQueue->WaitIdle();

        if (saveImage && !filename.empty()) {
            saveImageToPNG(filename, width, height);
        }

        // Limpiar command buffer
        vkFreeCommandBuffers(m_vkcore->GetDevice(), m_cmdBufPool, 1, &cmdBuf);
    }

    void Raytracer::saveImageToPNG(const std::string& filename, int width, int height) {
        VkDevice device = m_vkcore->GetDevice();

        // Crear command buffer temporal
        VkCommandBuffer cmdBuf;
        m_vkcore->CreateCommandBuffer(1, &cmdBuf);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmdBuf, &beginInfo);

        // Crear staging buffer
        VkDeviceSize imageSize = width * height * 4; // RGBA8
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createStagingBuffer(imageSize, stagingBuffer, stagingBufferMemory);
        


        // Transici�n de layout para transferencia
        m_vkcore->TransitionImageLayout((m_outTexture[0].m_image), VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        // Copiar imagen a buffer
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

        vkCmdCopyImageToBuffer(cmdBuf, (m_outTexture[0].m_image), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            stagingBuffer, 1, &region);



        // Restaurar layout original
        m_vkcore->TransitionImageLayout((m_outTexture[0].m_image), VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL);

        vkEndCommandBuffer(cmdBuf);

        // Submit y esperar
        core::VulkanQueue* pQueue = m_vkcore->GetQueue();
        pQueue->SubmitSync(cmdBuf);
        pQueue->WaitIdle();

        // Mapear memoria y leer datos
        void* data;
        vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);

        // Convertir de RGBA a RGB si es necesario (stb_image_write soporta ambos)
        // Para PNG con canal alpha, usar directamente RGBA
        int result = stbi_write_png(filename.c_str(), width, height, 4, data, width * 4);

        if (result == 0) {
            printf("Failed to write PNG file: %s\n", filename.c_str());
        }
        else {
            printf("Successfully saved image to: %s\n", filename.c_str());
        }

        // Limpiar recursos
        vkUnmapMemory(device, stagingBufferMemory);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
        vkFreeCommandBuffers(device, m_cmdBufPool, 1, &cmdBuf);
    }

    void Raytracer::createStagingBuffer(VkDeviceSize size, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_vkcore->GetDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create staging buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_vkcore->GetDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(m_vkcore->GetDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate staging buffer memory!");
        }

        vkBindBufferMemory(m_vkcore->GetDevice(), buffer, bufferMemory, 0);
    }

    uint32_t Raytracer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_vkcore->GetSelectedPhysicalDevice().m_physDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("Failed to find suitable memory type!");
    }

    size_t Raytracer::copyResultBytes(uint8_t* buffer, size_t bufferSize, VulkanTexture* tex, int width, int height) {
        if (!tex || !tex->m_image || !buffer) {
            return 0;
        }

        VkDevice device = *m_device;

        // Calcular el tama�o de la imagen (RGBA8)
        VkDeviceSize imageSize = width * height * 4;

        // Verificar si el buffer es suficientemente grande
        if (bufferSize < imageSize) {
            printf("Buffer size insufficient. Required: %zu, Available: %zu\n",
                (size_t)imageSize, bufferSize);
            return 0;
        }

        // Crear command buffer temporal
        VkCommandBuffer cmdBuf;
        m_vkcore->CreateCommandBuffer(1, &cmdBuf);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(cmdBuf, &beginInfo);

        // Crear staging buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createStagingBuffer(imageSize, stagingBuffer, stagingBufferMemory);

        // Transici�n de layout para transferencia
        // Nota: Ajusta el layout inicial seg�n el estado actual de tu textura
        // Podr�a ser VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL o VK_IMAGE_LAYOUT_GENERAL
        m_vkcore->TransitionImageLayout((m_outTexture[0].m_image), VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        // Copiar imagen a buffer
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

        vkCmdCopyImageToBuffer(cmdBuf, (m_outTexture[0].m_image), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            stagingBuffer, 1, &region);

        // Restaurar layout original
        m_vkcore->TransitionImageLayout((m_outTexture[0].m_image), VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkEndCommandBuffer(cmdBuf);

        // Submit y esperar
        VulkanQueue* pQueue = m_vkcore->GetQueue();
        pQueue->SubmitSync(cmdBuf);
        pQueue->WaitIdle();

        // Mapear memoria y copiar datos al buffer del usuario
        void* data;
        VkResult res = vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
        if (res != VK_SUCCESS) {
            printf("Error mapping memory: %d\n", res);
            // Limpiar recursos antes de retornar
            vkDestroyBuffer(device, stagingBuffer, nullptr);
            vkFreeMemory(device, stagingBufferMemory, nullptr);
            vkFreeCommandBuffers(device, m_cmdBufPool, 1, &cmdBuf);
            return 0;
        }

        // Copiar datos al buffer proporcionado por el usuario
        memcpy(buffer, data, imageSize);

        // Limpiar recursos
        vkUnmapMemory(device, stagingBufferMemory);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingBufferMemory, nullptr);
        vkFreeCommandBuffers(device, m_cmdBufPool, 1, &cmdBuf);

        return (size_t)imageSize;
    }
#pragma endregion
}