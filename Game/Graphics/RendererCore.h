#pragma once
// =============================================================================
// Game/Graphics/RendererCore.h
//
// Núcleo de frame Vulkan durante a migração do Renderer monolítico.
// Não conhece Player, Level, UI ou editor.
// =============================================================================

#include "Graphics/VulkanFrameCapture.h"

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace gfx {

class VulkanContext;
class Swapchain;
class RenderPass;
class Pipeline;

class RendererCore {
public:
    using DeviceWaitIdleFunction = VkResult (*)(VkDevice);

    enum class FrameStatus {
        Ready,
        SwapchainNeedsRecreate,
        Fatal,
    };

    explicit RendererCore(DeviceWaitIdleFunction waitIdle = &vkDeviceWaitIdle)
        : m_waitIdle(waitIdle) {}
    ~RendererCore() { cleanup(); }

    RendererCore(const RendererCore&) = delete;
    RendererCore& operator=(const RendererCore&) = delete;

    static FrameStatus classifyVulkanResult(VkResult result);

    bool init(VulkanContext* ctx, Swapchain* swapchain,
              RenderPass* renderPass, Pipeline* pipeline);
    void cleanup();

    FrameStatus beginFrame(VkCommandBuffer& commandBuffer, uint32_t& imageIndex);
    bool beginRenderPass(VkCommandBuffer commandBuffer, uint32_t imageIndex,
                         float clearR, float clearG, float clearB);
    bool endRenderPass(VkCommandBuffer commandBuffer);
    FrameStatus submitFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    // Rebuilds swapchain-dependent resources after OUT_OF_DATE/SUBOPTIMAL.
    // On failure after wait-idle succeeds, the core is left explicitly
    // uninitialized so callers cannot accidentally reuse destroyed
    // synchronization or frame resources.
    //
    // If vkDeviceWaitIdle() itself fails, the old resources/state are left
    // intact and false is returned. The consumer must fail closed.
    bool recreateSwapchain();

    VkExtent2D swapchainExtent() const;
    bool isInitialized() const { return m_initialized; }

private:
    bool createFramebuffers();
    bool createCommandPool();
    bool allocateCommandBuffers();
    bool createSyncObjects();
    void destroyFrameResources();

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

    VulkanFrameCapture m_capture;
    std::string m_capturePath;

    DeviceWaitIdleFunction m_waitIdle = &vkDeviceWaitIdle;
    bool m_initialized = false;
};

} // namespace gfx
