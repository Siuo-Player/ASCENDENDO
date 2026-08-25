// =============================================================================
//  Game/Graphics/UiRenderer.cpp
// =============================================================================
#include "Graphics/UiRenderer.h"

#include "Graphics/FontRenderer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/ShapeRenderer.h"
#include "Graphics/SpriteRenderer.h"
#include "Graphics/TextPipeline.h"
#include "Core/Config.h"
#include "Logic/Player.h"

namespace gfx {

namespace {
void bindText(VkCommandBuffer cmd, TextPipeline* textPipeline, FontRenderer* font) {
    if (!textPipeline || !font || !textPipeline->isInitialized()) return;
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, textPipeline->pipeline());
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(config::LOGICAL_WIDTH);
    viewport.height = static_cast<float>(config::LOGICAL_HEIGHT);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);
}

void centerText(VkCommandBuffer cmd, TextPipeline* textPipeline, FontRenderer* font,
                const char* text, float centerX, float baselineY, float scale,
                float r, float g, float b, float a) {
    if (!textPipeline || !font || !textPipeline->isInitialized()) return;
    const float width = font->measureText(text, scale);
    font->drawText(cmd, textPipeline->layout(), text,
                   centerX - width * 0.5f, baselineY, scale, r, g, b, a);
}
} // namespace

void UiRenderer::drawCredits(VkCommandBuffer cmd,
                             const Pipeline& shapePipeline,
                             const ShapeRenderer& shapes,
                             TextPipeline* textPipeline,
                             FontRenderer* font) const {
    const float centerX = config::LOGICAL_WIDTH / 2.0f;

    shapes.drawRect(cmd, shapePipeline, 60.0f, 250.0f,
                    520.0f, 2.0f, 0.30f, 0.30f, 0.40f);
    shapes.drawRect(cmd, shapePipeline, 60.0f, 82.0f,
                    520.0f, 2.0f, 0.25f, 0.25f, 0.35f);

    if (!textPipeline || !font || !textPipeline->isInitialized()) return;
    bindText(cmd, textPipeline, font);

    centerText(cmd, textPipeline, font, "ASCENDENDO",
               centerX, 298.0f, 1.5f, 1.0f, 0.85f, 0.10f, 1.0f);
    centerText(cmd, textPipeline, font, "FIM DA CAMPANHA",
               centerX, 266.0f, 0.75f, 0.80f, 0.80f, 0.80f, 1.0f);
    font->drawText(cmd, textPipeline->layout(), "AUTOR:",
                   80.0f, 225.0f, 0.45f, 0.50f, 0.50f, 0.60f, 1.0f);
    font->drawText(cmd, textPipeline->layout(), "RAFAEL GOMES BERNARDO",
                   80.0f, 193.0f, 0.70f, 0.95f, 0.95f, 0.95f, 1.0f);
    font->drawText(cmd, textPipeline->layout(), "AUXILIADO POR:",
                   80.0f, 160.0f, 0.45f, 0.50f, 0.50f, 0.60f, 1.0f);
    font->drawText(cmd, textPipeline->layout(), "CLAUDE  (ANTHROPIC)",
                   80.0f, 128.0f, 0.65f, 0.70f, 0.80f, 1.00f, 1.0f);
    font->drawText(cmd, textPipeline->layout(), "GEMINI  (GOOGLE)",
                   80.0f, 98.0f, 0.65f, 0.60f, 0.90f, 0.78f, 1.0f);
    centerText(cmd, textPipeline, font, "ESPACO PARA CONTINUAR",
               centerX, 55.0f, 0.45f, 0.40f, 0.40f, 0.55f, 1.0f);
}

} // namespace gfx