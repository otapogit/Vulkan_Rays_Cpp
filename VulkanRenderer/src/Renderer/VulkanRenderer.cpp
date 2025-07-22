#pragma once


#ifdef WIN32
#include <windows.h>  // Necesario en Windows antes de OpenGL
#endif
#include "core/core.h"
//#include <GLFW/glfw3.h> DEPRECATED
#include <core/core_shader.h>

#include <core/core_simple_mesh.h>
#include <core/core_rt.h>
#include <core/core_vertex.h>
#include "core/utils.h"
#include <iostream>

#include <glm/glm.hpp>


#include <GL/glew.h>
#include <GL/wglew.h>

#include "Renderer/VulkanRenderer.h"
#ifdef _WIN32
#include <GLFW/glfw3native.h>  // Importante: debes incluir esto
#endif
#include <GLFW/glfw3.h>
#include <GL/gl.h>      // Para funciones básicas de OpenGL
#include <GL/glu.h>     // Para funciones de utilidad (opcional)



#include <stdexcept>


// Extensiones OpenGL necesarias para memory objects
#ifndef GL_EXT_memory_object
#define GL_EXT_memory_object 1
#endif

#ifndef GL_EXT_memory_object_fd
#define GL_EXT_memory_object_fd 1
#endif

#ifndef GL_EXT_memory_object_win32
#define GL_EXT_memory_object_win32 1
#endif


// Si usas extensiones modernas, también:
//#include <GL/glext.h>   // Para extensiones


class VulkanRenderer::Impl {
    public:
        Impl() {
           
    }

     ~Impl()  {
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
     bool init()  {
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
        m_raytracer.createGeometryDescriptorSet(50);
  

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
        const std::vector<uint32_t> inds)  {

        core::SimpleMesh mesh;
        //mesh.m_vertexBufferSize = sizeof(vtcs[0]) * vtcs.size();
        //mesh.m_vb = m_vkcore.CreateVertexBuffer(vtcs.data(), mesh.m_vertexBufferSize, true);

        mesh.verts = vtcs;
        mesh.norms = nrmls;

        mesh.m_indexBufferSize = sizeof(inds[0]) * inds.size();
        mesh.m_indexbuffer = m_vkcore.CreateIndexBuffer(inds.data(), mesh.m_indexBufferSize, true);

        mesh.m_indexType = VK_INDEX_TYPE_UINT32;

        //Faltan crear buffer de normales e uvs
        //mesh.m_normalbuffer = m_vkcore.CreateNormalBuffer(nrmls, true);

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
        & color, MeshId id)  {
        dirtyupdate = true;

        int tid = -1;
        //En principio actualizar las meshes es escribir de nuevo un uniforme
        for (int i = 0; i < meshesC.size(); i++) {
            if (id == meshesC[i].id) tid = i;
        }
        if (tid == -1) return false;

        meshesC[tid].m_transMat = modelMatrix;

        meshesC[tid].color = color;

        std::vector<glm::vec3> modelverts = {};
        std::vector<glm::vec3> modelnorms = {};

        for (const glm::vec3& vert : meshesC[tid].verts) {
            glm::vec4 homogeneousVert = glm::vec4(vert, 1.0f);

            // Multiplicar por la matriz de transformación
            glm::vec4 transformedVert = modelMatrix * homogeneousVert;

            // Convertir de vuelta a vec3 (xyz)
            modelverts.push_back(glm::vec3(transformedVert));
        }

        for (const glm::vec3& norm : meshesC[tid].norms) {
            // Para normales usamos w=0.0f porque no queremos que se vean afectadas por la traslación
            glm::vec4 homogeneousNorm = glm::vec4(norm, 0.0f);

            // Multiplicar por la matriz de transformación
            glm::vec4 transformedNorm = modelMatrix * homogeneousNorm;

            // Convertir de vuelta a vec3 y normalizar
            modelnorms.push_back(glm::normalize(glm::vec3(transformedNorm)));
        }

        //meshesC[tid].
        meshesC[tid].m_vertexBufferSize = sizeof(modelverts[0]) * modelverts.size();
        meshesC[tid].m_vb = m_vkcore.CreateVertexBuffer(modelverts.data(), meshesC[tid].m_vertexBufferSize, true);
        meshesC[tid].m_normalbuffer = m_vkcore.CreateNormalBuffer(modelnorms, true);
     
        //m_meshesDraw contiene las meshes que se dibujarán
        m_meshesDraw.push_back(meshesC[tid]);

        return true;

    }


     TextureId addTexture(uint8_t* texels, uint32_t width,
        uint32_t height, uint32_t bpp)  {
        //Para esto usar los pushconstants??
        core::VulkanTexture* tex = new core::VulkanTexture();
        VkFormat Format = VK_FORMAT_R8G8B8A8_UNORM;
        m_vkcore.CreateTexture(texels, width, height, bpp, *tex);
        tex->id = m_baseTexId++;
        texturesC.push_back(tex);
        return tex->id;
    }

     void deleteTexture(TextureId tid)  {
        for(uint32_t i = 0; i < texturesC.size(); i++) {
            if (texturesC[i]->id == tid) {
                texturesC[i]->Destroy(m_vkcore.GetDevice());
                texturesC.erase(texturesC.begin() + i);
                return;
            }
        }
        return;
    }

     bool addLight(const glm::mat4& modelMatrix, MeshId id,
        const glm::vec3& color, LightId lid, TextureId tid = 0) {
        //pasarle la textura la id y tal

        return addMesh(modelMatrix, color, id);
    }

    /**
     * @brief Removes the indicated mesh from memory (and all its
instances in the scene)
      * @param id
      * @return false if the mesh id does not exists
      */
     bool removeMesh(MeshId id) {
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
     void clearScene() {
        dirtyupdate = true;
        m_meshesDraw = {};
    }

    /**
     * @brief Defines the camera
     * @param viewMatrix
     * @param projMatrix
     */
     void setCamera(const glm::mat4& viewMatrix, const
        glm::mat4& projMatrix) {
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
     void setOutputResolution(uint32_t width, uint32_t height) {
        windowwidth = width;
        windowheight = height;
        m_raytracer.createOutImage(windowwidth, windowheight, m_outTexture);
    }

   


    /**
     * @brief Renders the scene. The result can be obtained with Renderer::copyResultBytes or Renderer::getResultTextureId
      * @return
      */
     bool render() {
        //Antes de entregar quitar ek guardar en png
        if (dirtyupdate) {
            updateMeshes();
            dirtyupdate = false;
        }

        m_raytracer.render(windowwidth, windowheight, saving, "Test1.png");
        return true;
    }

    /**
     * @brief  Copies the final image into buffer
     * @param buffer destination
     * @param bufferSize size of buffer
     * @return the number of bytes written to buffer
     */
     size_t copyResultBytes(uint8_t* buffer, size_t bufferSize) {
        return m_vkcore.copyResultBytes(buffer, bufferSize, m_outTexture, windowwidth, windowheight);
    }

    /*
     * @brief Returns the texture object id with the result image
     * @return the GL? object Id (0 if there is not a texture, or it is
     *   not compatible with GL)
     */
     uint32_t getResultTextureId() {
        //Mirar el interop
         
         if (!m_outTexture || !m_outTexture->m_mem) {
             return 0;
         }


        // 
         try {
            #ifdef WIN32
             VkMemoryRequirements memReqs;
             vkGetImageMemoryRequirements(m_device, m_outTexture->m_image, &memReqs);
             // Windows - exportar handle
             
             VkMemoryGetWin32HandleInfoKHR handleInfo = {};
             handleInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_WIN32_HANDLE_INFO_KHR;
             handleInfo.memory = m_outTexture->m_mem;
             handleInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;

             

             PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR = (PFN_vkGetMemoryWin32HandleKHR)
                 vkGetDeviceProcAddr(m_device, "vkGetMemoryWin32HandleKHR");
             PFNGLCREATEMEMORYOBJECTSEXTPROC glCreateMemoryObjectsEXT = (PFNGLCREATEMEMORYOBJECTSEXTPROC)
                 wglGetProcAddress("glCreateMemoryObjectsEXT");
             PFNGLDELETEMEMORYOBJECTSEXTPROC glDeleteMemoryObjectsEXT = (PFNGLDELETEMEMORYOBJECTSEXTPROC)
                 wglGetProcAddress("glDeleteMemoryObjectsEXT");
             PFNGLTEXTURESTORAGEMEM2DEXTPROC glTextureStorageMem2DEXT = (PFNGLTEXTURESTORAGEMEM2DEXTPROC)
                 wglGetProcAddress("glTextureStorageMem2DEXT");
             PFNGLCREATETEXTURESPROC glCreateTextures = (PFNGLCREATETEXTURESPROC)
                 wglGetProcAddress("glCreateTextures");
             PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC glImportMemoryWin32HandleEXT = (PFNGLIMPORTMEMORYWIN32HANDLEEXTPROC)
                 wglGetProcAddress("glImportMemoryWin32HandleEXT");

             if (!glCreateMemoryObjectsEXT) {
                 std::cerr << "No se pudo cargar glCreateMemoryObjectsEXT" << std::endl;
                 return 0;
             }
             if (!vkGetMemoryWin32HandleKHR) {
                 std::cerr << "No se pudo cargar vkGetMemoryWin32HandleKHR" << std::endl;
                 return 0;
             }
             if (!glTextureStorageMem2DEXT) {
                 std::cerr << "No se pudo cargar glTextureStorageMem2DEXT" << std::endl;
                 return 0;
             }
             if (!glCreateTextures) {
                 std::cerr << "No se pudo cargar glCreateTextures" << std::endl;
                 return 0;
             }
             if (!glImportMemoryWin32HandleEXT) {
                 std::cerr << "No se pudo cargar glImportMemoryWin32HandleEXT" << std::endl;
                 return 0;
             }

             HANDLE memoryHandle;
             VkResult result = vkGetMemoryWin32HandleKHR(m_device, &handleInfo, &memoryHandle);
             if (result != VK_SUCCESS) {
                 return 0;
             }
             
             // En OpenGL - importar memoria
             GLuint memoryObject;
             glCreateMemoryObjectsEXT(1, &memoryObject);
             checkGLError("glCreateMemoryObjectsEXT");
             glImportMemoryWin32HandleEXT(memoryObject, memReqs.size, GL_HANDLE_TYPE_OPAQUE_WIN32_EXT, memoryHandle);
             checkGLError("glImportMemoryWin32HandleEXT");
             GLint dedicated = GL_TRUE;
             glMemoryObjectParameterivEXT(memoryObject, GL_DEDICATED_MEMORY_OBJECT_EXT, &dedicated);
            #else
             // Linux - exportar file descriptor
             VkMemoryGetFdInfoKHR fdInfo = {};
             fdInfo.sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR;
             fdInfo.memory = m_outTexture->m_mem;
             fdInfo.handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

             PFN_vkGetMemoryFdKHR vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)
                 vkGetDeviceProcAddr(m_device, "vkGetMemoryFdKHR");

             int memoryFd;
             VkResult result = vkGetMemoryFdKHR(m_device, &fdInfo, &memoryFd);
             if (result != VK_SUCCESS) {
                 return 0;
             }

             // En OpenGL - importar memoria
             GLuint memoryObject;
             glCreateMemoryObjectsEXT(1, &memoryObject);
             glImportMemoryFdEXT(memoryObject, memReqs.size, GL_HANDLE_TYPE_OPAQUE_FD_EXT, memoryFd);
            #endif

             glfwMakeContextCurrent(m_pWindow);
             // Crear textura OpenGL
             GLuint texture = 0;
             glGenTextures(1, &texture);
             glBindTexture(GL_TEXTURE_2D, texture);
             checkGLError("glCreateTextures");
             glTextureStorageMem2DEXT(texture, 1, GL_RGBA8, windowwidth, windowheight, memoryObject, 0);
             checkGLError("glTextureStorageMem2DEXT");

             return (uint32_t)texture;
         }
         catch (...) {
             return 0;
         }
         return 0;

    }

     void save(bool s, GLFWwindow* window) {
         saving = s;
         m_pWindow = window;
     }

    private:

        void updateMeshes() {
            m_raytracer.createBottomLevelAS(m_meshesDraw);
            m_raytracer.createTopLevelAS();
            m_raytracer.UpdateAccStructure();
            m_raytracer.updateGeometryDescriptorSet(m_meshesDraw);
            if (!pipelineCreated) {
                m_raytracer.createRtPipeline(rgen, rmiss, rchit);
                m_raytracer.createRtShaderBindingTable();
                pipelineCreated = true;
            }

        }

        void checkGLError(const char* operation) {
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cerr << "Error OpenGL en " << operation << ": " << error << std::endl;
            }
        }

        core::VulkanQueue* m_pQueue;
        core::VulkanCore m_vkcore;
        VkDevice m_device = NULL;
        int m_numImages = 0;
        std::vector<VkCommandBuffer> m_cmdBufs;
        GLFWwindow* m_pWindow;
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

        bool saving = false;

        ////Textures
        std::vector<core::VulkanTexture*> texturesC;
        uint32_t m_baseTexId = 1;

        ///RAYTRACING
        core::Raytracer m_raytracer;

        //PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR = nullptr;
        //PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR;
};
using MeshId = uint32_t;
using TextureId = uint32_t;
using LightId = uint32_t;

VulkanRenderer::VulkanRenderer() : pImpl(std::make_unique<Impl>()) {}

VulkanRenderer::~VulkanRenderer() = default;

VulkanRenderer::VulkanRenderer(VulkanRenderer&&) noexcept = default;
VulkanRenderer& VulkanRenderer::operator=(VulkanRenderer&&) noexcept = default;

bool VulkanRenderer::init() {
    return pImpl->init();
}

MeshId VulkanRenderer::defineMesh(const std::vector<glm::vec3>& vtcs,
    const std::vector<glm::vec3>& nrmls,
    const std::vector<glm::vec2>& uv,
    const std::vector<uint32_t> inds) {
    return pImpl->defineMesh(vtcs, nrmls, uv, inds);
}

bool VulkanRenderer::addMesh(const glm::mat4& modelMatrix, const glm::vec3& color, MeshId id) {
    return pImpl->addMesh(modelMatrix, color, id);
}

TextureId VulkanRenderer::addTexture(uint8_t* texels, uint32_t width, uint32_t height, uint32_t bpp) {
    return pImpl->addTexture(texels, width, height, bpp);
}

void VulkanRenderer::deleteTexture(TextureId tid) {
    pImpl->deleteTexture(tid);
}

bool VulkanRenderer::addLight(const glm::mat4& modelMatrix, MeshId id, const glm::vec3& color, LightId lid, TextureId tid) {
    return pImpl->addLight(modelMatrix, id, color, lid, tid);
}

bool VulkanRenderer::removeMesh(MeshId id) {
    return pImpl->removeMesh(id);
}

void VulkanRenderer::clearScene() {
    pImpl->clearScene();
}

void VulkanRenderer::setCamera(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) {
    pImpl->setCamera(viewMatrix, projMatrix);
}

void VulkanRenderer::setOutputResolution(uint32_t width, uint32_t height) {
    pImpl->setOutputResolution(width, height);
}

bool VulkanRenderer::render() {
    return pImpl->render();
}

size_t VulkanRenderer::copyResultBytes(uint8_t* buffer, size_t bufferSize) {
    return pImpl->copyResultBytes(buffer, bufferSize);
}

uint32_t VulkanRenderer::getResultTextureId() {
    return pImpl->getResultTextureId();
}

void VulkanRenderer::save(bool s, GLFWwindow* window) {
    pImpl->save(s,window);
}