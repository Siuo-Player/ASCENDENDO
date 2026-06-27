#pragma once
// =============================================================================
//  Game/Graphics/Renderer.h
//
//  @version 7.5
//  @history
//    v5.3  — drawFrame recebe Player + Camera
//    v6.2b — drawFrame aceita Level* opcional; plataformas antes do jogador
//    v7.5  — GameState (PLAYING/CREDITS/MENU), FLAG visual, ecrã de créditos
// =============================================================================
#include <vulkan/vulkan.h>
#include <vector>

namespace logic { class Player; class Level; }
namespace gfx   { class Camera; }

namespace gfx {

// ─── Estado do Jogo ───────────────────────────────────────────────────────────
// Partilhado entre Renderer e main.cpp (inclui Renderer.h).
enum class GameState { PLAYING, CREDITS, MENU };

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

    // state e menuSel têm defaults → retrocompativel com testes existentes
    bool drawFrame(const logic::Player& player,
                   const gfx::Camera&  camera,
                   const logic::Level* level    = nullptr,
                   GameState           state    = GameState::PLAYING,
                   int                 menuSel  = 0);

    bool isInitialized() const { return m_initialized; }

private:
    bool createFramebuffers();
    bool createCommandPool();
    bool allocateCommandBuffers();
    bool createSyncObjects();

    bool recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex,
                             const logic::Player& player,
                             const gfx::Camera&  camera,
                             const logic::Level* level,
                             GameState state, int menuSel);

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
