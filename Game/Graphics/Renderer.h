#pragma once
// =============================================================================
//  Game/Graphics/Renderer.h
//
//  @version 6.2b
//  @history
//    v5.3  — drawFrame recebe Player + Camera
//    v6.2b — drawFrame aceita Level* opcional; plataformas desenhadas antes
//             do jogador (retrocompativel: nullptr = comportamento anterior)
// =============================================================================
#include <vulkan/vulkan.h>
#include <vector>

namespace logic { class Player; class Level; }
namespace gfx   { class Camera; }

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

    bool init(VulkanContext* ctx, Swapchain* swapchain,
              RenderPass* renderPass, Pipeline* pipeline);
    void cleanup();

    // Level* e opcional: nullptr = so o jogador (testes de integracao existentes)
    bool drawFrame(const logic::Player& player, const gfx::Camera& camera,
                   const logic::Level* level = nullptr);

    bool isInitialized() const { return m_initialized; }

private:
    bool createFramebuffers();
    bool createCommandPool();
    bool allocateCommandBuffers();
    bool createSyncObjects();

    bool recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex,
                             const logic::Player& player,
                             const gfx::Camera& camera,
                             const logic::Level* level);

    VulkanContext* m_ctx        = nullptr;
    Swapchain*     m_swapchain  = nullptr;
    RenderPass*    m_renderPass = nullptr;
    Pipeline*      m_pipeline   = nullptr;

    std::vector<VkFramebuffer>   m_framebuffers;
    VkCommandPool                m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;

    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence     m_inFlightFence           = VK_NULL_HANDLE;

    bool m_initialized = false;
};

} // namespace gfx
