#pragma once
// =============================================================================
//  Game/Graphics/Renderer.h
//
//  @version 2.6
//  @history
//    v2.6 — criado (Framebuffers, CommandPool, SyncObjects, drawFrame)
//
//  Orquestrador do render loop: une Swapchain + RenderPass para produzir frames.
//
//  Ciclo de vida:
//    Renderer r;
//    r.init(&ctx, &swapchain, &renderPass);
//    while (!win.shouldClose()) {
//        win.pollEvents();
//        r.drawFrame(0.05f, 0.05f, 0.2f);  // azul escuro
//    }
//    vkDeviceWaitIdle(ctx.device());
//    r.cleanup();
//
//  Internamente, drawFrame executa:
//    1. Espera pela fence do frame anterior (CPU/GPU sync)
//    2. Adquire imagem do Swapchain
//    3. Grava CommandBuffer (begin RenderPass → clear → end RenderPass)
//    4. Submete para a fila grafica
//    5. Apresenta no ecra (vkQueuePresentKHR)
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

    // ── Ciclo de Vida ─────────────────────────────────────────────────────────
    bool init(VulkanContext* ctx, Swapchain* swapchain, RenderPass* renderPass);
    void cleanup();

    // ── Render ────────────────────────────────────────────────────────────────
    // Limpa o ecra com a cor (r, g, b) e apresenta o frame.
    // Retorna false em caso de erro irrecuperavel.
    bool drawFrame(float r, float g, float b);

    bool isInitialized() const { return m_initialized; }

private:
    bool createFramebuffers();
    bool createCommandPool();
    bool allocateCommandBuffers();
    bool createSyncObjects();
    bool recordCommandBuffer(VkCommandBuffer cmd, uint32_t imageIndex,
                             float r, float g, float b);

    // ── Dependencias (nao possui — nao destroi) ───────────────────────────────
    VulkanContext* m_ctx        = nullptr;
    Swapchain*     m_swapchain  = nullptr;
    RenderPass*    m_renderPass = nullptr;

    // ── Recursos proprios ─────────────────────────────────────────────────────
    std::vector<VkFramebuffer>   m_framebuffers;
    VkCommandPool                m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;

    // Sincronizacao (1 conjunto por simplicidade — sem frames-in-flight multiplos)
    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence     m_inFlightFence           = VK_NULL_HANDLE;

    bool m_initialized = false;
};

} // namespace gfx
