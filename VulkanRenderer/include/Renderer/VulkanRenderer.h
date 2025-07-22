#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Renderer.h"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

class VulkanRenderer : public Renderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();

    // Disable copy constructor and assignment operator
    VulkanRenderer(const VulkanRenderer&) = delete;
    VulkanRenderer& operator=(const VulkanRenderer&) = delete;

    // Enable move constructor and assignment operator
    VulkanRenderer(VulkanRenderer&&) noexcept;
    VulkanRenderer& operator=(VulkanRenderer&&) noexcept;

    /**
     * @brief Initializes the underlying graphics library (GL, Vulkan...)
     * @return true, if succeeded
     */
    virtual bool init() override;

    /**
     * @brief Defines a mesh (it won't be rendered until it is added to
     * the scene with Renderer::addMesh
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
        const std::vector<uint32_t> inds) override;

    /**
     * @brief Add a previously defined mesh to the scene
     * @param modelMatrix transformation applied to the mesh
     * @param color color of the mesh
     * @param id the mesh id
     * @return false if the mesh id does not exists
     */
    bool addMesh(const glm::mat4& modelMatrix, const glm::vec3& color, MeshId id) override;

    /**
     * @brief Add a texture to the renderer
     * @param texels texture data
     * @param width texture width
     * @param height texture height
     * @param bpp bytes per pixel
     * @return texture id
     */
    TextureId addTexture(uint8_t* texels, uint32_t width, uint32_t height, uint32_t bpp) override;

    /**
     * @brief Delete a texture from the renderer
     * @param tid texture id to delete
     */
    void deleteTexture(TextureId tid) override;

    /**
     * @brief Add a light to the scene
     * @param modelMatrix transformation applied to the light
     * @param id mesh id for the light
     * @param color light color
     * @param lid light id
     * @param tid texture id (optional)
     * @return false if the mesh id does not exist
     */
    bool addLight(const glm::mat4& modelMatrix, MeshId id, const glm::vec3& color, LightId lid, TextureId tid = 0) override;

    /**
     * @brief Removes the indicated mesh from memory (and all its instances in the scene)
     * @param id mesh id to remove
     * @return false if the mesh id does not exists
     */
    bool removeMesh(MeshId id) override;

    /**
     * @brief removes all the meshes from the scene
     */
    void clearScene() override;

    /**
     * @brief Defines the camera
     * @param viewMatrix view matrix
     * @param projMatrix projection matrix
     */
    void setCamera(const glm::mat4& viewMatrix, const glm::mat4& projMatrix) override;

    /**
     * @brief Defines the size of the result image
     * @param width image width
     * @param height image height
     */
    void setOutputResolution(uint32_t width, uint32_t height) override;

    /**
     * @brief Renders the scene. The result can be obtained with Renderer::copyResultBytes or Renderer::getResultTextureId
     * @return true if render succeeded
     */
    bool render() override;

    /**
     * @brief Copies the final image into buffer
     * @param buffer destination buffer
     * @param bufferSize size of buffer
     * @return the number of bytes written to buffer
     */
    size_t copyResultBytes(uint8_t* buffer, size_t bufferSize) override;

    /**
     * @brief Returns the texture object id with the result image
     * @return the GL object Id (0 if there is not a texture, or it is not compatible with GL)
     */
    uint32_t getResultTextureId() override;

    /**
     * @brief Enable/disable saving rendered images
     * @param s save flag
     */
    void save(bool s, GLFWwindow* window);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};