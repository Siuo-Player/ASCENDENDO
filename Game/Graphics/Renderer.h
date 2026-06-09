#pragma once
// =============================================================================
//  Game/Graphics/Renderer.h
//
//  @version 5.3
// =============================================================================
#include <vulkan/vulkan.h>
#include <vector>

// Forward declarations para não termos de incluir os .h gigantes aqui
namespace logic { class Player; }
namespace gfx { class Camera; }

namespace gfx {

class VulkanContext;
class Swapchain;
class RenderPass;
class Pipeline;

class Renderer {
public:
    Renderer()  = default;
    ~Renderer() { cleanup(); }

    Renderer(const Renderer&)            = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool init(VulkanContext* ctx, Swapchain* swapchain, RenderPass* renderPass, Pipeline* pipeline);
    void cleanup();
    
    // MUDANÇA AQUI: Recebe o jogador e a câmara!
    bool drawFrame(const logic::Player& player, const gfx::Camera& camera);
    
    bool isInitialized() const { return m_initialized; }

private:
    bool createFramebuffers();
    bool createCommandPool();
    bool allocateCommandBuffers();
    bool createSyncObjects();
    
    // MUDANÇA AQUI: Assinatura atualizada
    bool recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex, const logic::Player& player, const gfx::Camera& camera);

    VulkanContext* m_ctx        = nullptr;
    Swapchain* m_swapchain  = nullptr;
    RenderPass* m_renderPass = nullptr;
    Pipeline* m_pipeline   = nullptr;

    std::vector<VkFramebuffer>   m_framebuffers;
    VkCommandPool                m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;

    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence     m_inFlightFence           = VK_NULL_HANDLE;

    bool m_initialized = false;
};

} // namespace gfx