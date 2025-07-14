#pragma once
#include <memory>
#include <glm/glm.hpp>

class VulkanRenderApp {
public:
    VulkanRenderApp(int WindowWidth, int WindowHeight);
    ~VulkanRenderApp();

    // Disable copy constructor and assignment operator
    VulkanRenderApp(const VulkanRenderApp&) = delete;
    VulkanRenderApp& operator=(const VulkanRenderApp&) = delete;

    // Enable move constructor and assignment operator
    VulkanRenderApp(VulkanRenderApp&&) noexcept;
    VulkanRenderApp& operator=(VulkanRenderApp&&) noexcept;

    void initRt();
    void loop();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};