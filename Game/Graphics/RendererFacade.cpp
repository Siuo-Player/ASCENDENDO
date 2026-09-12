// =============================================================================
// Game/Graphics/RendererFacade.cpp
// =============================================================================
#include "Graphics/RendererFacade.h"
#include "Graphics/RendererCore.h"
#include "Graphics/ShapeRenderer.h"
#include "Graphics/WorldRenderer.h"
#include "Graphics/UiRenderer.h"
#include "Graphics/EditorRenderer.h"
#include "Graphics/Swapchain.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Camera.h"
#include "Graphics/PresentationConfig.h"
#include "Core/Config.h"

#include <memory>
#include <string>

namespace gfx {
namespace {

void drawAdminValidationOverlay(VkCommandBuffer commandBuffer,
                                const Pipeline& shapePipeline,
                                const ShapeRenderer& shapes,
                                TextPipeline* textPipeline,
                                FontRenderer* font,
                                const logic::CampaignValidationSnapshot& snapshot,
                                std::size_t currentIndex) {
    if (!textPipeline || !font || !textPipeline->isInitialized()) return;

    const float panelX = 28.0f;
    const float panelY = 18.0f;
    const float panelW = 584.0f;
    const float panelH = 324.0f;
    shapes.drawRect(commandBuffer, shapePipeline, panelX, panelY,
                    panelW, panelH, 0.015f, 0.02f, 0.04f, 0.94f);
    shapes.drawRect(commandBuffer, shapePipeline, panelX, panelY + panelH - 3.0f,
                    panelW, 3.0f, 0.95f, 0.72f, 0.10f, 1.0f);

    font->drawText(commandBuffer, textPipeline->layout(),
                   "ADMIN VALIDATION  [F10 fechar]",
                   46.0f, 318.0f, 0.62f, 0.95f, 0.80f, 0.18f, 1.0f);

    const std::string summary =
        "Fisica: " + std::to_string(snapshot.validLevels) + "/" +
        std::to_string(snapshot.entries.size()) +
        (snapshot.allValid() ? " OK" : " ERRO");
    font->drawText(commandBuffer, textPipeline->layout(), summary.c_str(),
                   46.0f, 292.0f, 0.42f,
                   snapshot.allValid() ? 0.35f : 1.0f,
                   snapshot.allValid() ? 0.95f : 0.30f,
                   snapshot.allValid() ? 0.45f : 0.25f, 1.0f);

    constexpr std::size_t rowsPerColumn = 13;
    for (std::size_t i = 0; i < snapshot.entries.size(); ++i) {
        const auto& entry = snapshot.entries[i];
        const std::size_t column = i / rowsPerColumn;
        const std::size_t row = i % rowsPerColumn;
        const float x = column == 0 ? 48.0f : 318.0f;
        const float y = 262.0f - static_cast<float>(row) * 18.0f;
        const bool current = i == currentIndex;
        const bool valid = entry.valid;

        if (current) {
            shapes.drawRect(commandBuffer, shapePipeline,
                            x - 6.0f, y - 3.0f, 250.0f, 18.0f,
                            0.22f, 0.18f, 0.05f, 0.95f);
        }

        const std::string levelNumber = std::to_string(i + 1) + ". ";
        font->drawText(commandBuffer, textPipeline->layout(), levelNumber.c_str(),
                       x, y, 0.40f, 0.62f, 0.64f, 0.74f, 1.0f);

        const std::string status = valid ? "OK" : "ERRO";
        font->drawText(commandBuffer, textPipeline->layout(), status.c_str(),
                       x + 24.0f, y, 0.40f,
                       valid ? 0.35f : 1.0f,
                       valid ? 0.95f : 0.30f,
                       valid ? 0.45f : 0.25f, 1.0f);

        const std::string detail = " " + std::to_string(entry.reachablePlatforms) +
                                   "/" + std::to_string(entry.totalPlatforms);
        font->drawText(commandBuffer, textPipeline->layout(), detail.c_str(),
                       x + 58.0f, y, 0.38f, 0.55f, 0.58f, 0.68f, 1.0f);
    }

    font->drawText(commandBuffer, textPipeline->layout(),
                   "OK/ERRO = caminho de salto segundo o validador de campanha."
                   "  ADMIN MODE congela o jogo.",
                   46.0f, 34.0f, 0.34f, 0.50f, 0.54f, 0.64f, 1.0f);
}

} // namespace

RendererFacade::RendererFacade() = default;

RendererFacade::~RendererFacade() {
    cleanup();
}

bool RendererFacade::init(VulkanContext* ctx, Swapchain* swapchain,
                          RenderPass* renderPass, Pipeline* pipeline) {
    if (m_initialized) return true;
    if (!ctx || !swapchain || !renderPass || !pipeline) return false;

    m_core = std::make_unique<RendererCore>();
    m_shapes = std::make_unique<ShapeRenderer>();
    m_world = std::make_unique<WorldRenderer>();
    m_ui = std::make_unique<UiRenderer>();
    m_editor = std::make_unique<EditorRenderer>();
    m_shapePipeline = pipeline;

    if (!m_core->init(ctx, swapchain, renderPass, pipeline)) {
        cleanup();
        return false;
    }

    m_initialized = true;
    return true;
}

void RendererFacade::cleanup() {
    m_editor.reset();
    m_ui.reset();
    m_world.reset();
    m_shapes.reset();
    m_core.reset();

    m_shapePipeline = nullptr;
    m_editorSnapshotPtr = nullptr;
    m_editorSnapshot = {};
    m_campaignEditorSnapshotPtr = nullptr;
    m_campaignEditorSnapshot = {};
    m_campaignValidationSnapshotPtr = nullptr;
    m_campaignValidationSnapshot = {};
    m_textPipeline = nullptr;
    m_font = nullptr;
    m_spritePipeline = nullptr;
    m_sprite = nullptr;
    m_initialized = false;
}

void RendererFacade::attachText(TextPipeline* textPipeline, FontRenderer* font) {
    m_textPipeline = textPipeline;
    m_font = font;
}

void RendererFacade::attachSprite(SpritePipeline* spritePipeline, SpriteRenderer* sprite) {
    m_spritePipeline = spritePipeline;
    m_sprite = sprite;
}

void RendererFacade::attachEditorSnapshot(const logic::EditorRenderSnapshot* snapshot) {
    if (!snapshot) {
        m_editorSnapshot = {};
        m_editorSnapshotPtr = nullptr;
        return;
    }
    m_editorSnapshot = *snapshot;
    m_editorSnapshotPtr = &m_editorSnapshot;
    m_campaignEditorSnapshotPtr = nullptr;
}

void RendererFacade::attachCampaignEditorSnapshot(
    const logic::CampaignEditorRenderSnapshot* snapshot) {
    if (!snapshot) {
        m_campaignEditorSnapshot = {};
        m_campaignEditorSnapshotPtr = nullptr;
        return;
    }
    m_campaignEditorSnapshot = *snapshot;
    m_campaignEditorSnapshotPtr = &m_campaignEditorSnapshot;
    m_editorSnapshotPtr = nullptr;
}

void RendererFacade::attachCampaignValidationSnapshot(
    const logic::CampaignValidationSnapshot* snapshot) {
    if (!snapshot) {
        m_campaignValidationSnapshot = {};
        m_campaignValidationSnapshotPtr = nullptr;
        return;
    }
    m_campaignValidationSnapshot = *snapshot;
    m_campaignValidationSnapshotPtr = &m_campaignValidationSnapshot;
}

bool RendererFacade::drawFrame(const RenderSnapshot& snapshot,
                               const Camera& camera,
                               RenderState state,
                               int menuSelection,
                               float elapsedSeconds,
                               bool adminValidationVisible,
                               std::size_t currentCampaignLevelIndex) {
    if (!m_initialized || !m_core || !m_shapes || !m_shapePipeline) return false;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    uint32_t imageIndex = 0;
    const auto beginStatus = m_core->beginFrame(commandBuffer, imageIndex);
    if (beginStatus == RendererCore::FrameStatus::SwapchainNeedsRecreate) {
        return m_core->recreateSwapchain();
    }
    if (beginStatus == RendererCore::FrameStatus::Fatal) return false;

    float clearR = 0.05f;
    float clearG = 0.05f;
    float clearB = 0.15f;
    switch (state) {
        case RenderState::CREDITS:
            clearR = presentation::CLEAR_CREDITS_R;
            clearG = presentation::CLEAR_CREDITS_G;
            clearB = presentation::CLEAR_CREDITS_B;
            break;
        case RenderState::MENU:
            clearR = presentation::CLEAR_MENU_R;
            clearG = presentation::CLEAR_MENU_G;
            clearB = presentation::CLEAR_MENU_B;
            break;
        case RenderState::EDITOR:
            clearR = presentation::CLEAR_EDITOR_R;
            clearG = presentation::CLEAR_EDITOR_G;
            clearB = presentation::CLEAR_EDITOR_B;
            break;
        case RenderState::PLAYING:
        case RenderState::PAUSED:
            break;
    }

    if (!m_core->beginRenderPass(commandBuffer, imageIndex, clearR, clearG, clearB)) {
        return false;
    }

    const VkExtent2D extent = m_core->swapchainExtent();
    const float windowAspect = extent.height > 0
        ? static_cast<float>(extent.width) / static_cast<float>(extent.height)
        : config::TARGET_ASPECT;

    uint32_t viewportWidth = extent.width;
    uint32_t viewportHeight = extent.height;
    if (windowAspect > config::TARGET_ASPECT) {
        viewportWidth = static_cast<uint32_t>(extent.height * config::TARGET_ASPECT);
    } else if (config::TARGET_ASPECT > 0.0f) {
        viewportHeight = static_cast<uint32_t>(extent.width / config::TARGET_ASPECT);
    }

    const int32_t offsetX = static_cast<int32_t>((extent.width - viewportWidth) / 2);
    const int32_t offsetY = static_cast<int32_t>((extent.height - viewportHeight) / 2);

    VkViewport viewport{};
    viewport.x = static_cast<float>(offsetX);
    viewport.y = static_cast<float>(offsetY);
    viewport.width = static_cast<float>(viewportWidth);
    viewport.height = static_cast<float>(viewportHeight);
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {offsetX, offsetY};
    scissor.extent = {viewportWidth, viewportHeight};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    switch (state) {
        case RenderState::PLAYING:
            m_world->draw(commandBuffer, *m_shapePipeline, *m_shapes,
                          snapshot, camera, m_spritePipeline, m_sprite);
            m_ui->drawTimer(commandBuffer, m_textPipeline, m_font, elapsedSeconds);
            if (adminValidationVisible && m_campaignValidationSnapshotPtr) {
                drawAdminValidationOverlay(commandBuffer, *m_shapePipeline, *m_shapes,
                                           m_textPipeline, m_font,
                                           *m_campaignValidationSnapshotPtr,
                                           currentCampaignLevelIndex);
            }
            break;

        case RenderState::PAUSED:
            m_world->draw(commandBuffer, *m_shapePipeline, *m_shapes,
                          snapshot, camera, m_spritePipeline, m_sprite);
            m_ui->drawPaused(commandBuffer, *m_shapePipeline, *m_shapes,
                             m_textPipeline, m_font, menuSelection);
            break;

        case RenderState::CREDITS:
            m_ui->drawCredits(commandBuffer, *m_shapePipeline, *m_shapes,
                              m_textPipeline, m_font);
            break;

        case RenderState::MENU:
            m_ui->drawMenu(commandBuffer, *m_shapePipeline, *m_shapes,
                           m_textPipeline, m_font, menuSelection);
            break;

        case RenderState::EDITOR:
            if (m_campaignEditorSnapshotPtr) {
                m_ui->drawCampaignEditor(commandBuffer, *m_shapePipeline, *m_shapes,
                                         m_textPipeline, m_font,
                                         *m_campaignEditorSnapshotPtr);
            } else if (m_editorSnapshotPtr) {
                m_editor->draw(commandBuffer, *m_shapePipeline, *m_shapes,
                               *m_editorSnapshotPtr,
                               m_textPipeline, m_font);
            }
            break;
    }

    if (!m_core->endRenderPass(commandBuffer)) return false;

    const auto submitStatus = m_core->submitFrame(commandBuffer, imageIndex);
    if (submitStatus == RendererCore::FrameStatus::SwapchainNeedsRecreate) {
        return m_core->recreateSwapchain();
    }
    return submitStatus == RendererCore::FrameStatus::Ready;
}

} // namespace gfx
