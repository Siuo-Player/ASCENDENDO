#pragma once
// =============================================================================
// Game/Graphics/RendererFacade.h
// =============================================================================

#include "Graphics/RenderState.h"
#include "Graphics/RenderSnapshot.h"
#include "Logic/CampaignEditorSnapshot.h"
#include "Logic/CampaignValidation.h"
#include "Logic/EditorRenderSnapshot.h"

#include <vulkan/vulkan.h>
#include <memory>
#include <cstddef>

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
    void attachCampaignEditorSnapshot(const logic::CampaignEditorRenderSnapshot* snapshot);
    void attachCampaignValidationSnapshot(const logic::CampaignValidationSnapshot* snapshot);

    bool drawFrame(const RenderSnapshot& snapshot,
                   const Camera& camera,
                   RenderState state,
                   int menuSelection = 0,
                   float elapsedSeconds = 0.0f,
                   bool adminValidationVisible = false,
                   std::size_t currentCampaignLevelIndex = 0);

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
    logic::EditorRenderSnapshot m_editorSnapshot{};
    const logic::EditorRenderSnapshot* m_editorSnapshotPtr = nullptr;
    logic::CampaignEditorRenderSnapshot m_campaignEditorSnapshot{};
    const logic::CampaignEditorRenderSnapshot* m_campaignEditorSnapshotPtr = nullptr;
    logic::CampaignValidationSnapshot m_campaignValidationSnapshot{};
    const logic::CampaignValidationSnapshot* m_campaignValidationSnapshotPtr = nullptr;

    bool m_initialized = false;
};

} // namespace gfx
