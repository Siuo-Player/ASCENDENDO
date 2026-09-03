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

    // The Level Editor is a single logical 640x360 screen. Geometry is already
    // in logical coordinates, so no camera transform is permitted here.
    const Camera fixedCamera{};

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

    for (float x = 0.0f; x <= config::LOGICAL_WIDTH; x += spacing) {
        const bool major = std::fmod(x, majorSpacing) == 0.0f;
        shapes.drawRect(cmd, shapePipeline,
                        x - thickness * 0.5f, 0.0f,
                        thickness, config::LOGICAL_HEIGHT,
                        major ? majorR : minorR,
                        major ? majorG : minorG,
                        major ? majorB : minorB,
                        1.0f, &fixedCamera);
    }

    for (float y = 0.0f; y <= config::LOGICAL_HEIGHT; y += spacing) {
        const bool major = std::fmod(y, majorSpacing) == 0.0f;
        shapes.drawRect(cmd, shapePipeline,
                        0.0f, y - thickness * 0.5f,
                        config::LOGICAL_WIDTH, thickness,
                        major ? majorR : minorR,
                        major ? majorG : minorG,
                        major ? majorB : minorB,
                        1.0f, &fixedCamera);
    }

    constexpr float border = 3.0f;
    const float borderR = 0.75f;
    const float borderG = 0.75f;
    const float borderB = 0.82f;
    shapes.drawRect(cmd, shapePipeline, 0.0f, 0.0f,
                    config::LOGICAL_WIDTH, border,
                    borderR, borderG, borderB, 1.0f, &fixedCamera);
    shapes.drawRect(cmd, shapePipeline, 0.0f, config::LOGICAL_HEIGHT - border,
                    config::LOGICAL_WIDTH, border,
                    borderR, borderG, borderB, 1.0f, &fixedCamera);
    shapes.drawRect(cmd, shapePipeline, 0.0f, 0.0f,
                    border, config::LOGICAL_HEIGHT,
                    borderR, borderG, borderB, 1.0f, &fixedCamera);
    shapes.drawRect(cmd, shapePipeline, config::LOGICAL_WIDTH - border, 0.0f,
                    border, config::LOGICAL_HEIGHT,
                    borderR, borderG, borderB, 1.0f, &fixedCamera);

    for (std::size_t i = 0; i < snapshot.platforms.size(); ++i) {
        const logic::AABB& platform = snapshot.platforms[i];
        const bool selected = snapshot.hasSelection && snapshot.selectedIndex == i;

        const float r = selected ? 0.95f : presentation::COLOR_PLATFORM_R;
        const float g = selected ? 0.78f : presentation::COLOR_PLATFORM_G;
        const float b = selected ? 0.15f : presentation::COLOR_PLATFORM_B;

        shapes.drawRect(cmd, shapePipeline,
                        platform.min.x, platform.min.y,
                        platform.width(), platform.height(),
                        r, g, b, selected ? 1.0f : 0.92f, &fixedCamera);

        if (selected) {
            constexpr float selectionBorder = 2.0f;
            shapes.drawRect(cmd, shapePipeline,
                            platform.min.x, platform.max.y - selectionBorder,
                            platform.width(), selectionBorder,
                            1.0f, 0.88f, 0.12f, 1.0f, &fixedCamera);
            shapes.drawRect(cmd, shapePipeline,
                            platform.min.x, platform.min.y,
                            platform.width(), selectionBorder,
                            1.0f, 0.88f, 0.12f, 1.0f, &fixedCamera);
        }
    }

    // Spawn and goal are explicit editor entities, rendered independently from
    // platform geometry so editing them never implies a gameplay rewrite.
    constexpr float spawnHalf = 8.0f;
    constexpr float markerThickness = 2.0f;
    shapes.drawRect(cmd, shapePipeline,
                    snapshot.spawnPosition.x - markerThickness * 0.5f,
                    snapshot.spawnPosition.y,
                    markerThickness, spawnHalf,
                    0.25f, 1.0f, 0.55f, 0.95f, &fixedCamera);
    shapes.drawRect(cmd, shapePipeline,
                    snapshot.spawnPosition.x - spawnHalf,
                    snapshot.spawnPosition.y + spawnHalf - markerThickness,
                    spawnHalf * 2.0f, markerThickness,
                    0.25f, 1.0f, 0.55f, 0.95f, &fixedCamera);

    if (snapshot.hasFlag) {
        const logic::AABB& flag = snapshot.flagBounds;
        shapes.drawRect(cmd, shapePipeline,
                        flag.min.x, flag.min.y,
                        flag.width(), flag.height(),
                        1.0f, 0.72f, 0.18f, 0.90f, &fixedCamera);
        shapes.drawRect(cmd, shapePipeline,
                        flag.min.x, flag.max.y - markerThickness,
                        flag.width(), markerThickness,
                        1.0f, 0.92f, 0.30f, 1.0f, &fixedCamera);
    }

    if (snapshot.previewVisible) {
        const logic::AABB& preview = snapshot.previewBounds;
        shapes.drawRect(cmd, shapePipeline,
                        preview.min.x, preview.min.y,
                        preview.width(), preview.height(),
                        0.35f, 0.85f, 1.0f, 0.32f, &fixedCamera);
    }

    constexpr float cursorHalf = 5.0f;
    constexpr float cursorThickness = 1.0f;
    shapes.drawRect(cmd, shapePipeline,
                    snapshot.cursorWorld.x - cursorHalf,
                    snapshot.cursorWorld.y - cursorThickness * 0.5f,
                    cursorHalf * 2.0f, cursorThickness,
                    0.85f, 0.90f, 1.0f, 0.75f, &fixedCamera);
    shapes.drawRect(cmd, shapePipeline,
                    snapshot.cursorWorld.x - cursorThickness * 0.5f,
                    snapshot.cursorWorld.y - cursorHalf,
                    cursorThickness, cursorHalf * 2.0f,
                    0.85f, 0.90f, 1.0f, 0.75f, &fixedCamera);

    shapes.bind(cmd, shapePipeline);

    if (textPipeline && font) {
        const char* tool = snapshot.tool == logic::EditorToolMode::STAMP ? "STAMP" : "DRAG";
        const char* size = "MEDIUM";
        if (snapshot.sizePreset == logic::EditorSizePreset::SMALL) size = "SMALL";
        else if (snapshot.sizePreset == logic::EditorSizePreset::LARGE) size = "LARGE";
        const char* entity = "PLATFORM";
        if (snapshot.entityTool == logic::EditorEntityTool::SPAWN) entity = "SPAWN";
        else if (snapshot.entityTool == logic::EditorEntityTool::FLAG) entity = "FLAG";

        char hud[160];
        std::snprintf(hud, sizeof(hud),
                      "%s %s | %s | P PLAT S SPAWN F FLAG | DEL APAGAR | ESC SAIR",
                      entity, tool, size);
        drawEditorText(cmd, textPipeline, font, hud,
                       10.0f, config::LOGICAL_HEIGHT - 24.0f,
                       0.38f, 0.86f, 0.90f, 0.95f, 0.95f);
        shapes.bind(cmd, shapePipeline);
    }
}

} // namespace gfx
