#pragma once
// =============================================================================
//  Game/Graphics/Renderer.h
//
//  @version 4.2
//  @history
//    v4.2 — Reestruturação para Viewport Letterboxing direto no Swapchain
// =============================================================================
#include <vulkan/vulkan.h>
#include <vector>

namespace gfx {

class VulkanContext;
class Swapchain;
class RenderPass;

class Renderer {
public:
    Renderer()  = default;
    ~Renderer() { cleanup(); }

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool init(VulkanContext* ctx, Swapchain* swapchain, RenderPass* renderPass);
    void cleanup();
    bool drawFrame(float r, float g, float b);
    bool isInitialized() const { return m_initialized; }

private:
    bool createFramebuffers();
    bool createCommandPool();
    bool allocateCommandBuffers();
    bool createSyncObjects();
    bool recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex, float r, float g, float b);

    VulkanContext* m_ctx        = nullptr;
    Swapchain* m_swapchain  = nullptr;
    RenderPass* m_renderPass = nullptr;

    std::vector<VkFramebuffer>   m_framebuffers;
    VkCommandPool                m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;

    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence     m_inFlightFence           = VK_NULL_HANDLE;

    bool m_initialized = false;
};

} // namespace gfx