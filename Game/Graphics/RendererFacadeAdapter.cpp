// =============================================================================
// Game/Graphics/RendererFacadeAdapter.cpp
// =============================================================================
#include "Graphics/RendererFacadeAdapter.h"
#include "Graphics/Camera.h"
#include "Logic/EditorSession.h"

namespace gfx {

bool RendererFacadeAdapter::init(VulkanContext* ctx, Swapchain* swapchain,
                                 RenderPass* renderPass, Pipeline* pipeline) {
    return m_facade.init(ctx, swapchain, renderPass, pipeline);
}

void RendererFacadeAdapter::cleanup() {
    m_facade.cleanup();
    m_editorSession = nullptr;
    m_hasEditorSnapshot = false;
    m_editorSnapshot = {};
}

void RendererFacadeAdapter::attachText(TextPipeline* textPipeline, FontRenderer* font) {
    m_facade.attachText(textPipeline, font);
}

void RendererFacadeAdapter::attachSprite(SpritePipeline* spritePipeline, SpriteRenderer* sprite) {
    m_facade.attachSprite(spritePipeline, sprite);
}

void RendererFacadeAdapter::attachEditorSession(const logic::EditorSession* session) {
    m_editorSession = session;
    if (!session) {
        m_hasEditorSnapshot = false;
        m_editorSnapshot = {};
        m_facade.attachEditorSnapshot(nullptr);
        return;
    }

    m_editorSnapshot = session->renderSnapshot();
    m_hasEditorSnapshot = true;
    m_facade.attachEditorSnapshot(&m_editorSnapshot);
}

bool RendererFacadeAdapter::drawFrame(const logic::Player& player,
                                      const Camera& camera,
                                      const logic::Level* level,
                                      GameState state,
                                      int menuSel,
                                      float elapsedSeconds) {
    RenderState renderState = RenderState::PLAYING;
    switch (state) {
        case GameState::PLAYING: renderState = RenderState::PLAYING; break;
        case GameState::PAUSED:  renderState = RenderState::PAUSED;  break;
        case GameState::CREDITS: renderState = RenderState::CREDITS; break;
        case GameState::MENU:    renderState = RenderState::MENU;    break;
        case GameState::EDITOR:
            renderState = RenderState::EDITOR;
            if (m_editorSession) {
                m_editorSnapshot = m_editorSession->renderSnapshot();
                m_hasEditorSnapshot = true;
                m_facade.attachEditorSnapshot(&m_editorSnapshot);
            }
            break;
    }

    return m_facade.drawFrame(player, camera, level,
                              renderState, menuSel, elapsedSeconds);
}

bool RendererFacadeAdapter::isInitialized() const {
    return m_facade.isInitialized();
}

} // namespace gfx
