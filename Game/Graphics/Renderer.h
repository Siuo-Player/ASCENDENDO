#pragma once
// =============================================================================
//  Game/Graphics/Renderer.h
//
//  @version 8.2
//  @history
//    v5.3  — drawFrame recebe Player + Camera
//    v6.2b — drawFrame aceita Level* opcional; plataformas antes do jogador
//    v7.5  — GameState (PLAYING/CREDITS/MENU), FLAG visual, ecrã de créditos
//    v7.6  — attachText(): liga TextPipeline+FontRenderer (TTF real em
//             CREDITS/MENU). Opcional — sem isto, cai em BitmapFont.
//    v8.1  — GameState::PAUSED (ESC durante o jogo, distinto do menu de
//             fim-de-run). PLAYING ganha timer HUD. PAUSED desenha o mundo
//             congelado (dimmed) por baixo do menu de pausa.
//    v8.2  — attachSprite(): liga SpritePipeline+SpriteRenderer (jogador
//             desenhado como pixel-art em vez de rectangulo solido).
//             Opcional — sem isto, mantem o rectangulo (fallback).
// =============================================================================
#include <vulkan/vulkan.h>
#include <vector>

namespace logic { class Player; class Level; }
namespace gfx   { class Camera; class TextPipeline; class FontRenderer;
                  class SpritePipeline; class SpriteRenderer; }

namespace gfx {

// ─── Estado do Jogo ───────────────────────────────────────────────────────────
// Partilhado entre Renderer e main.cpp (inclui Renderer.h).
// PLAYING: gameplay activo, timer a contar.
// PAUSED:  ESC durante PLAYING. Mundo continua visivel (congelado, dimmed)
//          por baixo do menu. NAO termina a run. ESC de novo retoma.
// CREDITS: FLAG tocada (fim de campanha) OU "Creditos" a partir de
//          MENU/PAUSED. Regressa ao estado que o chamou.
// MENU:    ecra de fim-de-run, mostrado apos os creditos de vitoria.
enum class GameState { PLAYING, PAUSED, CREDITS, MENU };

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

    // Liga o texto TTF real (opcional). Chamar apos TextPipeline e
    // FontRenderer estarem inicializados. Sem isto, CREDITS/MENU usam
    // o fallback BitmapFont (pixelado, mas sempre funcional).
    void attachText(TextPipeline* textPipeline, FontRenderer* font);

    // Liga o sprite do jogador (opcional). Sem isto, o jogador continua a
    // ser desenhado como um rectangulo solido (fallback, sempre funcional).
    void attachSprite(SpritePipeline* spritePipeline, SpriteRenderer* sprite);

    // state, menuSel, elapsedSeconds têm defaults → retrocompativel com
    // testes existentes (test_renderer.cpp chama drawFrame(player,camera)).
    // elapsedSeconds: tempo de jogo activo acumulado (para o timer HUD em
    // PLAYING). Ignorado nos outros estados.
    bool drawFrame(const logic::Player& player,
                   const gfx::Camera&  camera,
                   const logic::Level* level          = nullptr,
                   GameState           state          = GameState::PLAYING,
                   int                 menuSel        = 0,
                   float               elapsedSeconds = 0.0f);

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
                             GameState state, int menuSel, float elapsedSeconds);

    VulkanContext* m_ctx        = nullptr;
    Swapchain*     m_swapchain  = nullptr;
    RenderPass*    m_renderPass = nullptr;
    Pipeline*      m_pipeline   = nullptr;

    TextPipeline*  m_textPipeline = nullptr; // nao possuido — opcional
    FontRenderer*  m_font         = nullptr; // nao possuido — opcional

    SpritePipeline* m_spritePipeline = nullptr; // nao possuido — opcional
    SpriteRenderer* m_sprite         = nullptr; // nao possuido — opcional

    std::vector<VkFramebuffer>   m_framebuffers;
    VkCommandPool                m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;

    VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore m_renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence     m_inFlightFence           = VK_NULL_HANDLE;

    bool m_initialized = false;
};

} // namespace gfx
