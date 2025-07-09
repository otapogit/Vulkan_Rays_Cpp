#pragma once
#include "Renderer.h"
#include "core/core.h"
//#include <GLFW/glfw3.h> DEPRECATED
#include <core/core_shader.h>

#include <core/core_simple_mesh.h>
#include <core/core_glfw.h>
#include <core/core_rt.h>
#include <core/core_vertex.h>
#include "core/utils.h"
#include <iostream>

#include <glm/glm.hpp>

class VulkanRenderer : public Renderer {
    public:
    VulkanRenderer() {

    }

     ~VulkanRenderer()  {
        vkDestroyShaderModule(m_vkcore.GetDevice(), rgen, nullptr);
        vkDestroyShaderModule(m_vkcore.GetDevice(), rmiss, nullptr);
        vkDestroyShaderModule(m_vkcore.GetDevice(), rchit, nullptr);

        for (int i = 0; i < meshesC.size(); i++) {
            meshesC[i].Destroy(m_vkcore.GetDevice());
        }

        for (int i = 0; i < texturesC.size(); i++) {
            texturesC[i]->Destroy(m_vkcore.GetDevice());
        }


        m_raytracer.cleanup();
        vkDestroyRenderPass(m_vkcore.GetDevice(), m_renderPass, NULL);
        m_outTexture->Destroy(m_vkcore.GetDevice());
    }

    /**
     * @brief Initializes the underlying graphics library (GL, Vulkan...)
     * @return true, if succeeded
     */
    virtual bool init() override {
        //Esto primero tiene que irse
        //m_pWindow = core::glwf_vulkan_init(1080, 1080, "AppName");
        m_vkcore.Init("AppName", 1920,1080);

        m_device = m_vkcore.GetDevice();
        m_numImages = 1;

        m_pQueue = m_vkcore.GetQueue();

        m_renderPass = m_vkcore.CreateSimpleRenderPass();
        m_frameBuffers = m_vkcore.CreateFrameBuffers(m_renderPass);

        //Creo que los framebuffers no hacen falta tampoco xq voy a hacer offscreen rendering??? nse la verda

        m_outTexture = new core::VulkanTexture();

        rgen = core::CreateShaderModuleFromText(m_vkcore.GetDevice(), "../VulkanRenderer/Shaders/raytrace.rgen");
        rmiss = core::CreateShaderModuleFromText(m_vkcore.GetDevice(), "../VulkanRenderer/Shaders/raytrace.rmiss");
        rchit = core::CreateShaderModuleFromText(m_vkcore.GetDevice(), "../VulkanRenderer/Shaders/raytrace.rchit");

        m_raytracer.initRayTracing(m_vkcore.GetSelectedPhysicalDevice(), &m_device);
        m_raytracer.setup(m_vkcore.GetCommandPool(), &m_vkcore);

        m_raytracer.createRtDescriptorSet();
        m_raytracer.createMvpDescriptorSet();

        //std::vector<core::SimpleMesh> emptyMeshes;
        //m_raytracer.createBottomLevelAS(emptyMeshes);
        //m_raytracer.createTopLevelAS();
        //m_raytracer.UpdateAccStructure();

        m_raytracer.createRtPipeline(rgen, rmiss, rchit);
        m_raytracer.createRtShaderBindingTable();


        return true;
    }


    /**
     * @brief Defines a mesh (it won't be rendered until it is added to
the scene with Renderer::addMesh
      * @param vtcs vertices
      * @param nrmls normals
      * @param uv texture coordinates (optional)
      * @param inds indices
      * @return the MeshId used to add a copy of this mesh to the scene
      */
     MeshId defineMesh(
        const std::vector<glm::vec3>& vtcs,
        const std::vector<glm::vec3>& nrmls,
        const std::vector<glm::vec2>& uv,
        const std::vector<uint32_t> inds) override {

        core::SimpleMesh mesh;
        mesh.m_vertexBufferSize = sizeof(vtcs[0]) * vtcs.size();
        mesh.m_vb = m_vkcore.CreateVertexBuffer(vtcs.data(), mesh.m_vertexBufferSize, true);

        mesh.m_indexBufferSize = sizeof(inds[0]) * inds.size();
        mesh.m_indexbuffer = m_vkcore.CreateIndexBuffer(inds.data(), mesh.m_indexBufferSize, true);

        mesh.m_indexType = VK_INDEX_TYPE_UINT32;

        //Faltan crear buffer de normales e uvs
        mesh.m_normalbuffer = m_vkcore.CreateNormalBuffer(nrmls, true);

        mesh.m_uvbuffer = m_vkcore.CreateUVBuffer(uv, true);

        mesh.vertexcount = inds.size();

        mesh.id = m_baseId++;

        meshesC.push_back(mesh);

        return mesh.id;
    }

    /*
       Plan para las meshes
       Add mesh y demás simplemente las crea y las mete en la lista de meshes a raytracear
       Deja update ccomo sucio, a updatear
       Una vez vaya a renderizar reconstruir las blas y tlas
    */


    /**
     * @brief Add a previously defined mesh to the scene
     * @param modelMatrix transformation applied to the mesh
     * @param id the mesh id
     * @return false if the mesh id does not exists
     */
     bool addMesh(const glm::mat4& modelMatrix, const glm::vec3
        & color, MeshId id) override {
        dirtyupdate = true;

        int tid = -1;
        //En principio actualizar las meshes es escribir de nuevo un uniforme
        for (int i = 0; i < meshesC.size(); i++) {
            if (id == meshesC[i].id) tid = i;
        }
        if (tid == -1) return false;

        meshesC[tid].m_transMat = modelMatrix;

        meshesC[tid].color = color;

        //m_meshesDraw contiene las meshes que se dibujarán
        m_meshesDraw.push_back(meshesC[tid]);

        return true;

    }


     TextureId addTexture(uint8_t* texels, uint32_t width,
        uint32_t height, uint32_t bpp) override {
        //Para esto usar los pushconstants??
        core::VulkanTexture* tex = new core::VulkanTexture();
        VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;
        m_vkcore.CreateTexture(texels, width, height, bpp, *tex);
        tex->id = m_baseTexId++;
        texturesC.push_back(tex);
        return tex->id;
    }

     void deleteTexture(TextureId tid) override {
        for(uint32_t i = 0; i < texturesC.size(); i++) {
            if (texturesC[i]->id == tid) {
                texturesC.erase(texturesC.begin() + i);
                return;
            }
        }
        return;
    }

     bool addLight(const glm::mat4& modelMatrix, MeshId id,
        const glm::vec3& color, LightId lid, TextureId tid = 0) override {
        //pasarle la textura la id y tal

        return addMesh(modelMatrix, color, id);
    }

    /**
     * @brief Removes the indicated mesh from memory (and all its
instances in the scene)
      * @param id
      * @return false if the mesh id does not exists
      */
     bool removeMesh(MeshId id) override{
        for (int i = 0; i < m_meshesDraw.size(); i++) {
            if (m_meshesDraw[i].id == id) {
                dirtyupdate = true;
                m_meshesDraw.erase(m_meshesDraw.begin() + i);
                return true;
            }
        }
        return false;

    }

    /**
     * @brief removes all the meshes from the scene
     */
     void clearScene() override{
        dirtyupdate = true;
        m_meshesDraw = {};
    }

    /**
     * @brief Defines the camera
     * @param viewMatrix
     * @param projMatrix
     */
     void setCamera(const glm::mat4& viewMatrix, const
        glm::mat4& projMatrix) override{
        VP = projMatrix * viewMatrix;
        VP = glm::affineInverse(VP);
        //Updatear el buffer que esta en el descriptor set
        m_raytracer.UpdateMvpMatrix(VP);
    }

    /**
     * @brief Defines the size of the result image
     * @param width
     * @param height
     */
     void setOutputResolution(uint32_t width, uint32_t height) override{
        windowwidth = width;
        windowheight = height;
        m_raytracer.createOutImage(windowwidth, windowheight, m_outTexture);
    }

   


    /**
     * @brief Renders the scene. The result can be obtained with Renderer::copyResultBytes or Renderer::getResultTextureId
      * @return
      */
     bool render() override{
        //Antes de entregar quitar ek guardar en png
        if (dirtyupdate) {
            updateMeshes();
            dirtyupdate = false;
        }

        m_raytracer.render(windowwidth, windowheight, true, "Test1.png");
        return true;
    }

    /**
     * @brief  Copies the final image into buffer
     * @param buffer destination
     * @param bufferSize size of buffer
     * @return the number of bytes written to buffer
     */
     size_t copyResultBytes(uint8_t* buffer, size_t bufferSize) override{
        return m_vkcore.copyResultBytes(buffer, bufferSize, m_outTexture, windowwidth, windowheight);
    }

    /*
     * @brief Returns the texture object id with the result image
     * @return the GL? object Id (0 if there is not a texture, or it is
     *   not compatible with GL)
     */
     uint32_t getResultTextureId() override{
        //Mirar el interop
        //No implementado
        return 0;
    }

    private:

        void updateMeshes() {
            m_raytracer.createBottomLevelAS(m_meshesDraw);
            m_raytracer.createTopLevelAS();
            m_raytracer.UpdateAccStructure();
            if (!pipelineCreated) {
                m_raytracer.createRtPipeline(rgen, rmiss, rchit);
                m_raytracer.createRtShaderBindingTable();
                pipelineCreated = true;
            }

        }

        core::VulkanQueue* m_pQueue;
        core::VulkanCore m_vkcore;
        VkDevice m_device = NULL;
        int m_numImages = 0;
        std::vector<VkCommandBuffer> m_cmdBufs;
        //GLFWwindow* m_pWindow;
        VkRenderPass m_renderPass;
        std::vector<VkFramebuffer> m_frameBuffers;

        VkShaderModule rgen, rmiss, rchit;

        
        core::VulkanTexture* m_outTexture;

        uint32_t windowwidth, windowheight;

        /////meshes
        bool dirtyupdate = false;
        std::vector<core::SimpleMesh> meshesC;
        std::vector<core::SimpleMesh> m_meshesDraw;

        uint32_t m_baseId = 0;

        glm::mat4 VP;
        bool pipelineCreated = false;

        ////Textures
        std::vector<core::VulkanTexture*> texturesC;
        uint32_t m_baseTexId = 1;

        ///RAYTRACING
        core::Raytracer m_raytracer;
};