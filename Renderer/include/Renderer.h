#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <glm/ext.hpp>

class Renderer {
public:
    using MeshId = uint32_t;
    using TextureId = uint32_t;
    using LightId = uint32_t;

    Renderer() {}
    virtual ~Renderer() {}
    /**
     * @brief Initializes the underlying graphics library (GL, Vulkan...)
     * @return true, if succeeded
     */
    virtual bool init() = 0;
    /**
     * @brief Defines a mesh (it won't be rendered until it is added to
the scene with Renderer::addMesh
      * @param vtcs vertices
      * @param nrmls normals
      * @param uv texture coordinates (optional)
      * @param inds indices
      * @return the MeshId used to add a copy of this mesh to the scene
      */
    virtual MeshId defineMesh(
        const std::vector<glm::vec3>& vtcs,
        const std::vector<glm::vec3>& nrmls,
        const std::vector<glm::vec2>& uv,
        const std::vector<uint32_t> inds) = 0;


    /**
    @brief This method reads the obj file and calls the previous method to return the MeshIds (a obj file may have several meshes)
    */
    std::vector<MeshId> defineMesh(const std::string& objfilename);

    /**
     * @brief Add a previously defined mesh to the scene
     * @param modelMatrix transformation applied to the mesh
     * @param id the mesh id
     * @param inspectable true, if this mesh is part of the part under inspection, false if it should not be inspected, but
     * it should be taken into account for occlusions (for example, scaffolding or camera or lighting fixtures)
     * @return false if the mesh id does not exists
     */
    virtual bool addMesh(const glm::mat4& modelMatrix, const glm::vec3
        & color, MeshId id, bool inspectable = true) = 0;


    virtual TextureId addTexture(uint8_t* texels, uint32_t width,
        uint32_t height, uint32_t bpp) = 0;

    virtual void deleteTexture(TextureId tid) = 0;

    virtual bool addLight(const glm::mat4& modelMatrix, MeshId id,
        const glm::vec3& color, LightId lid, TextureId tid = 0) = 0;

    /**
     * @brief Removes the indicated mesh from memory (and all its
instances in the scene)
      * @param id
      * @return false if the mesh id does not exists
      */
    virtual bool removeMesh(MeshId id) = 0;

    /**
     * @brief removes all the meshes from the scene
     */
    virtual void clearScene() = 0;

    /**
     * @brief Defines the view camera
     * @param viewMatrix
     * @param projMatrix
     */
    virtual void setViewCamera(const glm::mat4& viewMatrix, const
        glm::mat4& projMatrix) = 0;

    /**
     * @brief Defines the inspection camera
     * @param viewMatrix
     * @param projMatrix
     */
    virtual void setInspCamera(const glm::mat4& viewMatrix, const
        glm::mat4& projMatrix) = 0;

    /**
     * @brief Defines the size of the result image
     * @param width
     * @param height
     */
    virtual void setOutputResolution(uint32_t width, uint32_t height) = 0;

    /**
     * @brief Renders the scene. The result can be obtained with
Renderer::copyResultBytes or Renderer::getResultTextureId
      * @return
      */
    virtual bool render() = 0;

    /**
     * @brief  Copies the final image into buffer
     * @param buffer destination
     * @param bufferSize size of buffer
     * @return the number of bytes written to buffer
     */
    virtual size_t copyResultBytes(uint8_t* buffer, size_t bufferSize) = 0;

    /**
     * @brief Returns the texture object id with the result image
     * @return the GL? object Id (0 if there is not a texture, or it is
not compatible with GL)
      */
    virtual uint32_t getResultTextureId() = 0;

    virtual bool saveResultToFile(const std::string& filename) = 0;
};