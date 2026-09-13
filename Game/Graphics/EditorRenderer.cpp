// =============================================================================
// Game/Graphics/EditorRenderer.cpp
// =============================================================================
#include "Graphics/EditorRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/ShapeRenderer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/TextPipeline.h"
#include "Graphics/FontRenderer.h"
#include "Graphics/PresentationConfig.h"
#include "Core/Config.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace gfx {

namespace {

void drawEditorText(VkCommandBuffer cmd, TextPipeline* pipeline, FontRenderer* font,
                    const char* text, float x, float y, float scale,
                    float r, float g, float b, float a) {
    if (!pipeline || !font || !pipeline->isInitialized()) return;
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());
    font->bind(cmd, pipeline->layout());
    font->drawText(cmd, pipeline->layout(), text, x, y, scale, r, g, b, a);
}

} // namespace

void EditorRenderer::draw(VkCommandBuffer cmd,
                          const Pipeline& shapePipeline,
                          const ShapeRenderer& shapes,
                          const logic::EditorRenderSnapshot& snapshot,
                          TextPipeline* textPipeline,
                          FontRenderer* font) const {
    if (cmd == VK_NULL_HANDLE || !shapePipeline.isInitialized()) return;

    const float viewportHeight = config::LOGICAL_HEIGHT;
    const float maxView = std::max(0.0f, snapshot.levelHeight - viewportHeight);
    const float viewBottomY = std::clamp(snapshot.viewBottomY, 0.0f, maxView);
    const float viewTopY = viewBottomY + viewportHeight;
    const Camera fixedCamera{};
    const bool validationReady =
        snapshot.validationState == logic::EditorValidationState::COMPLETE ||
        snapshot.validationState == logic::EditorValidationState::STALE;

    auto screenY = [viewBottomY](float worldY) { return worldY - viewBottomY; };

    shapes.bind(cmd, shapePipeline);

    constexpr float spacing = presentation::EDITOR_GRID_VISUAL_SPACING;
    constexpr float majorSpacing = presentation::EDITOR_GRID_MAJOR_SPACING;
    constexpr float minorR = 0.18f;
    constexpr float minorG = 0.18f;
    constexpr float minorB = 0.22f;
    constexpr float majorR = 0.28f;
    constexpr float majorG = 0.28f;
    constexpr float majorB = 0.34f;
    constexpr float thickness = 1.0f;

    for (float x = 0.0f; x <= snapshot.levelWidth; x += spacing) {
        const bool major = std::fmod(x, majorSpacing) == 0.0f;
        shapes.drawRect(cmd, shapePipeline,
                        x - thickness * 0.5f, 0.0f,
                        thickness, config::LOGICAL_HEIGHT,
                        major ? majorR : minorR,
                        major ? majorG : minorG,
                        major ? majorB : minorB,
                        1.0f, &fixedCamera);
    }

    for (float worldY = std::floor(viewBottomY / spacing) * spacing;
         worldY <= viewTopY + thickness; worldY += spacing) {
        const float y = screenY(worldY);
        if (y < -thickness || y > config::LOGICAL_HEIGHT + thickness) continue;
        const bool major = std::fmod(std::fabs(worldY), majorSpacing) < 0.0001f;
        shapes.drawRect(cmd, shapePipeline,
                        0.0f, y - thickness * 0.5f,
                        snapshot.levelWidth, thickness,
                        major ? majorR : minorR,
                        major ? majorG : minorG,
                        major ? majorB : minorB,
                        1.0f, &fixedCamera);
    }

    // Screen boundaries make the 640x360 decomposition visible without
    // imposing an authoring grid: they are presentation-only guides.
    for (std::size_t i = 0; i <= snapshot.screenCount; ++i) {
        const float worldY = static_cast<float>(i) * viewportHeight;
        const float y = screenY(worldY);
        if (y < -2.0f || y > config::LOGICAL_HEIGHT + 2.0f) continue;
        shapes.drawRect(cmd, shapePipeline,
                        0.0f, y - 1.0f,
                        snapshot.levelWidth, 2.0f,
                        0.45f, 0.45f, 0.60f, 0.55f, &fixedCamera);
    }

    constexpr float border = 3.0f;
    const float borderR = 0.75f;
    const float borderG = 0.75f;
    const float borderB = 0.82f;
    shapes.drawRect(cmd, shapePipeline, 0.0f, 0.0f,
                    snapshot.levelWidth, border,
                    borderR, borderG, borderB, 1.0f, &fixedCamera);
    shapes.drawRect(cmd, shapePipeline, 0.0f, config::LOGICAL_HEIGHT - border,
                    snapshot.levelWidth, border,
                    borderR, borderG, borderB, 1.0f, &fixedCamera);
    shapes.drawRect(cmd, shapePipeline, 0.0f, 0.0f,
                    border, config::LOGICAL_HEIGHT,
                    borderR, borderG, borderB, 1.0f, &fixedCamera);
    shapes.drawRect(cmd, shapePipeline, snapshot.levelWidth - border, 0.0f,
                    border, config::LOGICAL_HEIGHT,
                    borderR, borderG, borderB, 1.0f, &fixedCamera);

    // The ground is derived state, not an authored platform. Show it in the
    // editor so the canonical Y=0..16 starting surface is always visible.
    const logic::AABB implicitGround{{0.0f, 0.0f}, {snapshot.levelWidth, 16.0f}};
    if (implicitGround.max.y >= viewBottomY && implicitGround.min.y <= viewTopY) {
        shapes.drawRect(cmd, shapePipeline,
                        implicitGround.min.x, screenY(implicitGround.min.y),
                        implicitGround.width(), implicitGround.height(),
                        0.32f, 0.38f, 0.48f, 0.95f, &fixedCamera);
        shapes.drawRect(cmd, shapePipeline,
                        implicitGround.min.x, screenY(implicitGround.max.y) - 2.0f,
                        implicitGround.width(), 2.0f,
                        0.55f, 0.66f, 0.82f, 1.0f, &fixedCamera);
    }

    for (std::size_t i = 0; i < snapshot.platforms.size(); ++i) {
        const logic::AABB& platform = snapshot.platforms[i];
        if (platform.max.y < viewBottomY || platform.min.y > viewTopY) continue;

        const bool selected = snapshot.hasSelection && snapshot.selectedIndex == i;
        const bool unreachable = validationReady &&
            i < snapshot.platformReachable.size() &&
            !snapshot.platformReachable[i];

        float r = presentation::COLOR_PLATFORM_R;
        float g = presentation::COLOR_PLATFORM_G;
        float b = presentation::COLOR_PLATFORM_B;
        if (unreachable && !selected) {
            r = 0.72f;
            g = 0.26f;
            b = 0.26f;
        }
        if (selected) {
            r = 0.95f;
            g = 0.78f;
            b = 0.15f;
        }

        shapes.drawRect(cmd, shapePipeline,
                        platform.min.x, screenY(platform.min.y),
                        platform.width(), platform.height(),
                        r, g, b, selected ? 1.0f : 0.92f, &fixedCamera);

        if (selected) {
            constexpr float selectionBorder = 2.0f;
            shapes.drawRect(cmd, shapePipeline,
                            platform.min.x, screenY(platform.max.y) - selectionBorder,
                            platform.width(), selectionBorder,
                            1.0f, 0.88f, 0.12f, 1.0f, &fixedCamera);
            shapes.drawRect(cmd, shapePipeline,
                            platform.min.x, screenY(platform.min.y),
                            platform.width(), selectionBorder,
                            1.0f, 0.88f, 0.12f, 1.0f, &fixedCamera);
        }
    }

    constexpr float spawnHalf = 8.0f;
    constexpr float markerThickness = 2.0f;
    const float spawnY = screenY(snapshot.spawnPosition.y);
    if (snapshot.spawnPosition.y >= viewBottomY - spawnHalf &&
        snapshot.spawnPosition.y <= viewTopY + spawnHalf) {
        shapes.drawRect(cmd, shapePipeline,
                        snapshot.spawnPosition.x - markerThickness * 0.5f,
                        spawnY, markerThickness, spawnHalf,
                        0.25f, 1.0f, 0.55f, 0.95f, &fixedCamera);
        shapes.drawRect(cmd, shapePipeline,
                        snapshot.spawnPosition.x - spawnHalf,
                        spawnY + spawnHalf - markerThickness,
                        spawnHalf * 2.0f, markerThickness,
                        0.25f, 1.0f, 0.55f, 0.95f, &fixedCamera);
    }

    if (snapshot.hasFlag) {
        const logic::AABB& flag = snapshot.flagBounds;
        if (flag.max.y >= viewBottomY && flag.min.y <= viewTopY) {
            shapes.drawRect(cmd, shapePipeline,
                            flag.min.x, screenY(flag.min.y),
                            flag.width(), flag.height(),
                            1.0f, 0.72f, 0.18f, 0.90f, &fixedCamera);
            shapes.drawRect(cmd, shapePipeline,
                            flag.min.x, screenY(flag.max.y) - markerThickness,
                            flag.width(), markerThickness,
                            1.0f, 0.92f, 0.30f, 1.0f, &fixedCamera);
        }
    }

    if (snapshot.previewVisible) {
        const logic::AABB& preview = snapshot.previewBounds;
        if (preview.max.y >= viewBottomY && preview.min.y <= viewTopY) {
            shapes.drawRect(cmd, shapePipeline,
                            preview.min.x, screenY(preview.min.y),
                            preview.width(), preview.height(),
                            0.35f, 0.85f, 1.0f, 0.32f, &fixedCamera);
        }
    }

    if (snapshot.cursorWorld.y >= viewBottomY - 8.0f &&
        snapshot.cursorWorld.y <= viewTopY + 8.0f) {
        const float cursorY = screenY(snapshot.cursorWorld.y);
        constexpr float cursorHalf = 5.0f;
        constexpr float cursorThickness = 1.0f;
        shapes.drawRect(cmd, shapePipeline,
                        snapshot.cursorWorld.x - cursorHalf,
                        cursorY - cursorThickness * 0.5f,
                        cursorHalf * 2.0f, cursorThickness,
                        0.85f, 0.90f, 1.0f, 0.75f, &fixedCamera);
        shapes.drawRect(cmd, shapePipeline,
                        snapshot.cursorWorld.x - cursorThickness * 0.5f,
                        cursorY - cursorHalf,
                        cursorThickness, cursorHalf * 2.0f,
                        0.85f, 0.90f, 1.0f, 0.75f, &fixedCamera);
    }

    shapes.bind(cmd, shapePipeline);

    if (textPipeline && font) {
        const char* tool = snapshot.tool == logic::EditorToolMode::STAMP ? "STAMP" : "DRAG";
        const char* size = "MEDIUM";
        if (snapshot.sizePreset == logic::EditorSizePreset::SMALL) size = "SMALL";
        else if (snapshot.sizePreset == logic::EditorSizePreset::LARGE) size = "LARGE";

        const std::size_t currentScreen =
            std::min(snapshot.screenCount - 1,
                     static_cast<std::size_t>(snapshot.cursorWorld.y / viewportHeight));

        char hud[192];
        std::snprintf(hud, sizeof(hud),
                      "PLATFORM %s | %s | SCREEN %zu/%zu | VIEW %.0f-%.0f | DEL APAGAR | ESC SAIR",
                      tool, size,
                      currentScreen + 1, snapshot.screenCount,
                      viewBottomY, viewTopY);
        drawEditorText(cmd, textPipeline, font, hud,
                       10.0f, 12.0f,
                       0.34f, 0.86f, 0.90f, 0.95f, 0.95f);

        if (snapshot.validationState == logic::EditorValidationState::RUNNING) {
            drawEditorText(cmd, textPipeline, font,
                           "VALIDACAO: A EXECUTAR",
                           10.0f, config::LOGICAL_HEIGHT - 18.0f,
                           0.34f, 0.85f, 0.90f, 1.0f, 1.0f);
        } else if (snapshot.validationState == logic::EditorValidationState::COMPLETE) {
            char validationHud[192];
            std::snprintf(validationHud, sizeof(validationHud),
                          "VALIDACAO: %s | PLATAFORMAS %d/%d | OBJETIVO %s",
                          snapshot.validationValid ? "OK" : "FALHOU",
                          snapshot.reachablePlatforms,
                          snapshot.totalPlatforms,
                          snapshot.validationReachesGoal ? "OK" : "INALCANCAVEL");
            drawEditorText(cmd, textPipeline, font, validationHud,
                           10.0f, config::LOGICAL_HEIGHT - 18.0f,
                           0.34f,
                           snapshot.validationValid ? 0.35f : 1.0f,
                           snapshot.validationValid ? 1.0f : 0.35f,
                           snapshot.validationValid ? 0.55f : 0.35f,
                           1.0f);
        } else if (snapshot.validationState == logic::EditorValidationState::STALE) {
            drawEditorText(cmd, textPipeline, font,
                           "VALIDACAO: DESATUALIZADA | DOCUMENTO ALTERADO",
                           10.0f, config::LOGICAL_HEIGHT - 18.0f,
                           0.34f, 1.0f, 0.72f, 0.25f, 1.0f);
        }

        if (validationReady && !snapshot.validationMessage.empty()) {
            drawEditorText(cmd, textPipeline, font,
                           snapshot.validationMessage.c_str(),
                           10.0f, config::LOGICAL_HEIGHT - 36.0f,
                           0.31f, 0.82f, 0.84f, 0.90f, 0.95f);
        }
        shapes.bind(cmd, shapePipeline);
    }
}

} // namespace gfx
