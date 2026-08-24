#pragma once
// =============================================================================
// Game/Graphics/RendererCore.h
//
// Núcleo de frame Vulkan durante a migração do Renderer monolítico.
// Não conhece Player, Level, UI ou editor.
// =============================================================================

#include <vulkan/vulkan.h>
#include <vector>

namespace gfx {

class VulkanContext;
class Swapchain;
class RenderPass;
class Pipeline;

class RendererCore {
public:
    RendererCore() = default;
    ~RendererCore() { cleanup(); }

    RendererCore(const RendererCore&) = delete;
    RendererCore& operator=(const RendererCore&) = delete;

    bool init(VulkanContext* ctx, Swapchain* swapchain,
              RenderPass* renderPass, Pipeline* pipeline);
    void cleanup();

    bool beginFrame(VkCommandBuffer& commandBuffer, uint32_t& imageIndex);
    bool beginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
                         float clearR, float clearG, float clearB);
    bool endRenderPass(VkCommandBuffer commandBuffer);
    bool submitFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    VkExtent2D swapchainExtent() const;
    bool isInitialized() const { return m_initialized; }

private:
    bool createFramebuffers();
    bool createCommandPool();
    bool allocateCommandBuffers();
    bool createSyncObjects();

    VulkanContext* m_ctx = nullptr;
    Swapchain* m_swapchain = nullptr;
    RenderPass* m_renderPass = nullptr;
    Pipeline* m_pipeline = nullptr;

    std::vector<VkFramebuffer> m_framebuffers;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;

    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence m_inFlightFence = VK_NULL_HANDLE;

    bool m_initialized = false;
};

} // namespace gfx
