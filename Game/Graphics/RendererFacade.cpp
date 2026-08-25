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
#include "Logic/Player.h"
#include "Logic/Level.h"
#include "Logic/EditorSession.h"
#include "Core/Config.h"

#include <algorithm>

namespace gfx {

RendererFacade::~RendererFacade() {
    cleanup();
}

bool RendererFacade::init(VulkanContext* ctx, Swapchain* swapchain,
                          RenderPass* renderPass, Pipeline* pipeline) {
    if (m_initialized) return true;
    if (!ctx || !swapchain || !renderPass || !pipeline) return false;

    m_core = new RendererCore();
    m_shapes = new ShapeRenderer();
    m_world = new WorldRenderer();
    m_ui = new UiRenderer();
    m_editor = new EditorRenderer();
    m_shapePipeline = pipeline;

    if (!m_core->init(ctx, swapchain, renderPass, pipeline)) {
        cleanup();
        return false;
    }

    m_initialized = true;
    return true;
}

void RendererFacade::cleanup() {
    delete m_editor;
    delete m_ui;
    delete m_world;
    delete m_shapes;
    delete m_core;

    m_editor = nullptr;
    m_ui = nullptr;
    m_world = nullptr;
    m_shapes = nullptr;
    m_core = nullptr;
    m_shapePipeline = nullptr;
    m_editorSnapshotPtr = nullptr;
    m_editorSnapshot = {};
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
        m_editorSnapshotPtr = nullptr;
        return;
    }
    m_editorSnapshot = *snapshot;
    m_editorSnapshotPtr = &m_editorSnapshot;
}

void RendererFacade::attachEditorSession(const logic::EditorSession* session) {
    if (!session) {
        m_editorSnapshotPtr = nullptr;
        return;
    }
    m_editorSnapshot = session->renderSnapshot();
    m_editorSnapshotPtr = &m_editorSnapshot;
}

bool RendererFacade::drawFrame(const logic::Player& player,
                               const Camera& camera,
                               const logic::Level* level,
                               GameState state,
                               int menuSelection,
                               float elapsedSeconds) {
    RenderState renderState = RenderState::PLAYING;
    switch (state) {
        case GameState::PLAYING: renderState = RenderState::PLAYING; break;
        case GameState::PAUSED:  renderState = RenderState::PAUSED;  break;
        case GameState::CREDITS: renderState = RenderState::CREDITS; break;
        case GameState::MENU:    renderState = RenderState::MENU;    break;
        case GameState::EDITOR:  renderState = RenderState::EDITOR; break;
    }
    return drawFrame(player, camera, level, renderState,
                     menuSelection, elapsedSeconds);
}

bool RendererFacade::drawFrame(const logic::Player& player,
                               const Camera& camera,
                               const logic::Level* level,
                               RenderState state,
                               int menuSelection,
                               float elapsedSeconds) {
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
            clearR = config::CLEAR_CREDITS_R;
            clearG = config::CLEAR_CREDITS_G;
            clearB = config::CLEAR_CREDITS_B;
            break;
        case RenderState::MENU:
            clearR = config::CLEAR_MENU_R;
            clearG = config::CLEAR_MENU_G;
            clearB = config::CLEAR_MENU_B;
            break;
        case RenderState::EDITOR:
            clearR = config::CLEAR_EDITOR_R;
            clearG = config::CLEAR_EDITOR_G;
            clearB = config::CLEAR_EDITOR_B;
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
                          player, camera, level, m_spritePipeline, m_sprite);
            m_ui->drawTimer(commandBuffer, m_textPipeline, m_font, elapsedSeconds);
            break;

        case RenderState::PAUSED:
            m_world->draw(commandBuffer, *m_shapePipeline, *m_shapes,
                          player, camera, level, m_spritePipeline, m_sprite);
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
            if (m_editorSnapshotPtr) {
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
