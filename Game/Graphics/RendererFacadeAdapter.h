#pragma once
// =============================================================================
// Game/Graphics/RendererFacadeAdapter.h
//
// Adapter de migração: expõe a API usada atualmente por main.cpp enquanto
// delega para o novo RendererFacade. Mantém a sessão do editor como fonte da
// verdade e atualiza o snapshot por cópia no frame em que EDITOR é desenhado.
// =============================================================================

#include "Graphics/Renderer.h"
#include "Graphics/RendererFacade.h"
#include "Logic/EditorRenderSnapshot.h"

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
    const logic::EditorSession* m_editorSession = nullptr; // não possuído
    logic::EditorRenderSnapshot m_editorSnapshot{};
    bool m_hasEditorSnapshot = false;
};

} // namespace gfx
