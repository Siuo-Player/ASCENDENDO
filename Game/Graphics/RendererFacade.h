#pragma once
// =============================================================================
// Game/Graphics/RendererFacade.h
// =============================================================================

#include "Graphics/RenderState.h"
#include "Core/RenderSnapshot.h"
#include "Logic/EditorRenderSnapshot.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace logic { class EditorSession; }
namespace gfx {

class VulkanContext;
class Swapchain;
class RenderPass;
class Pipeline;
class TextPipeline;
class FontRenderer;
class SpritePipeline;
class SpriteRenderer;
class RendererCore;
class ShapeRenderer;
class WorldRenderer;
class UiRenderer;
class EditorRenderer;
class Camera;

class RendererFacade {
public:
    RendererFacade();
    ~RendererFacade();

    RendererFacade(const RendererFacade&) = delete;
    RendererFacade& operator=(const RendererFacade&) = delete;

    bool init(VulkanContext* ctx, Swapchain* swapchain,
              RenderPass* renderPass, Pipeline* pipeline);
    void cleanup();

    void attachText(TextPipeline* textPipeline, FontRenderer* font);
    void attachSprite(SpritePipeline* spritePipeline, SpriteRenderer* sprite);
    void attachEditorSnapshot(const logic::EditorRenderSnapshot* snapshot);
    void attachEditorSession(const logic::EditorSession* session);

    bool drawFrame(const core::RenderSnapshot& snapshot,
                   const Camera& camera,
                   RenderState state,
                   int menuSelection = 0,
                   float elapsedSeconds = 0.0f);

    bool isInitialized() const { return m_initialized; }

private:
    std::unique_ptr<RendererCore> m_core;
    std::unique_ptr<ShapeRenderer> m_shapes;
    std::unique_ptr<WorldRenderer> m_world;
    std::unique_ptr<UiRenderer> m_ui;
    std::unique_ptr<EditorRenderer> m_editor;
    Pipeline* m_shapePipeline = nullptr;

    TextPipeline* m_textPipeline = nullptr;
    FontRenderer* m_font = nullptr;
    SpritePipeline* m_spritePipeline = nullptr;
    SpriteRenderer* m_sprite = nullptr;
    const logic::EditorSession* m_editorSession = nullptr; // não possuído
    logic::EditorRenderSnapshot m_editorSnapshot{};
    const logic::EditorRenderSnapshot* m_editorSnapshotPtr = nullptr;

    bool m_initialized = false;
};

} // namespace gfx
