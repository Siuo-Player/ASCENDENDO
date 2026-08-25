#pragma once
// =============================================================================
// Game/Graphics/RendererFacadeAdapter.h
//
// Adapter de migração: expõe a API usada atualmente por main.cpp enquanto
// delega para o novo RendererFacade. Permite um cut-over reversível.
// =============================================================================

#include "Graphics/Renderer.h"
#include "Graphics/RendererFacade.h"

namespace logic { class EditorSession; }

namespace gfx {

class RendererFacadeAdapter {
public:
    bool init(VulkanContext* ctx, Swapchain* swapchain,
              RenderPass* renderPass, Pipeline* pipeline);
    void cleanup();

    void attachText(TextPipeline* textPipeline, FontRenderer* font);
    void attachSprite(SpritePipeline* spritePipeline, SpriteRenderer* sprite);
    void attachEditorSession(const logic::EditorSession* session);

    bool drawFrame(const logic::Player& player,
                   const Camera& camera,
                   const logic::Level* level = nullptr,
                   GameState state = GameState::PLAYING,
                   int menuSel = 0,
                   float elapsedSeconds = 0.0f);

    bool isInitialized() const;

private:
    RendererFacade m_facade;
};

} // namespace gfx
