#pragma once
// =============================================================================
// Game/Graphics/RendererFacade.h
// =============================================================================

#include "Graphics/RenderState.h"
#include "Graphics/GameState.h"
#include "Logic/EditorRenderSnapshot.h"

#include <vulkan/vulkan.h>

namespace logic { class Player; class Level; class EditorSession; }
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
    RendererFacade() = default;
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

    bool drawFrame(const logic::Player& player,
                   const Camera& camera,
                   const logic::Level* level,
                   RenderState state,
                   int menuSelection = 0,
                   float elapsedSeconds = 0.0f);

    // Transitional overload for existing runtime callers. It converts the
    // application state at the presentation boundary rather than in an adapter.
    bool drawFrame(const logic::Player& player,
                   const Camera& camera,
                   const logic::Level* level,
                   GameState state,
                   int menuSelection = 0,
                   float elapsedSeconds = 0.0f);

    bool isInitialized() const { return m_initialized; }

private:
    RendererCore* m_core = nullptr;
    ShapeRenderer* m_shapes = nullptr;
    WorldRenderer* m_world = nullptr;
    UiRenderer* m_ui = nullptr;
    EditorRenderer* m_editor = nullptr;
    Pipeline* m_shapePipeline = nullptr;

    TextPipeline* m_textPipeline = nullptr;
    FontRenderer* m_font = nullptr;
    SpritePipeline* m_spritePipeline = nullptr;
    SpriteRenderer* m_sprite = nullptr;
    logic::EditorRenderSnapshot m_editorSnapshot{};
    const logic::EditorRenderSnapshot* m_editorSnapshotPtr = nullptr;

    bool m_initialized = false;
};

} // namespace gfx
