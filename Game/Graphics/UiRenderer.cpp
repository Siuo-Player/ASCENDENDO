// =============================================================================
// Game/Graphics/UiRenderer.cpp
// =============================================================================
#include "Graphics/UiRenderer.h"
#include "Graphics/ShapeRenderer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/TextPipeline.h"
#include "Graphics/FontRenderer.h"
#include "Core/Config.h"
#include "Core/Viewport.h"

#include <cstdio>
#include <string>

namespace gfx {

namespace {

void bindText(VkCommandBuffer cmd, TextPipeline* pipeline, FontRenderer* font) {
    if (!pipeline || !font || !pipeline->isInitialized()) return;
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle());
    font->bind(cmd, pipeline->layout());
}

void centerText(VkCommandBuffer cmd, TextPipeline* pipeline, FontRenderer* font,
                const char* text, float x, float y, float scale,
                float r, float g, float b, float a = 1.0f) {
    if (!pipeline || !font || !pipeline->isInitialized()) return;
    font->drawTextCentered(cmd, pipeline->layout(), text, x, y, scale, r, g, b, a);
}

void drawMenuBoxes(VkCommandBuffer cmd, const Pipeline& shapePipeline,
                   const ShapeRenderer& shapes, int count, int menuSelection) {
    const float boxWidth = core::MenuBoxLayout::boxWidth(count, config::LOGICAL_WIDTH);
    const float boxHeight = core::MenuBoxLayout::BOX_H;
    const float boxY = core::MenuBoxLayout::BOX_Y;

    for (int i = 0; i < count; ++i) {
        const bool selected = menuSelection == i;
        const float r = selected ? 1.0f : 0.22f;
        const float g = selected ? 0.85f : 0.22f;
        const float b = selected ? 0.10f : 0.28f;
        const float x = core::MenuBoxLayout::boxX(i, count, config::LOGICAL_WIDTH);

        shapes.drawRect(cmd, shapePipeline, x, boxY, boxWidth, boxHeight,
                        r * 0.15f, g * 0.15f, b * 0.12f, 0.92f);
        shapes.drawRect(cmd, shapePipeline, x, boxY, boxWidth, 2.0f, r, g, b);
        shapes.drawRect(cmd, shapePipeline, x, boxY + boxHeight - 2.0f,
                        boxWidth, 2.0f, r, g, b);
        shapes.drawRect(cmd, shapePipeline, x, boxY, 2.0f, boxHeight, r, g, b);
        shapes.drawRect(cmd, shapePipeline, x + boxWidth - 2.0f, boxY,
                        2.0f, boxHeight, r, g, b);
    }
}

void drawControlsReference(VkCommandBuffer cmd,
                           const Pipeline& shapePipeline,
                           const ShapeRenderer& shapes,
                           TextPipeline* textPipeline,
                           FontRenderer* font) {
    const float centerX = config::LOGICAL_WIDTH / 2.0f;
    shapes.drawRect(cmd, shapePipeline, 44.0f, 42.0f,
                    552.0f, 274.0f, 0.04f, 0.04f, 0.08f, 0.92f);
    shapes.drawRect(cmd, shapePipeline, 44.0f, 314.0f,
                    552.0f, 2.0f, 0.45f, 0.45f, 0.60f);

    if (!textPipeline || !font || !textPipeline->isInitialized()) return;
    bindText(cmd, textPipeline, font);

    centerText(cmd, textPipeline, font, "CONTROLOS",
               centerX, 286.0f, 1.15f, 0.95f, 0.80f, 0.10f);

    const char* lines[] = {
        "A / D ou SETAS      mover / navegar",
        "ESPACO              confirmar / saltar",
        "ESC                 voltar / pausa",
        "Q                   sair / voltar ao menu",
        "E                   editor de nivel",
        "C                   editor de campanha",
        "0                   ver controlos",
        "G                   STAMP / DRAG",
        "[ / ]               tamanho menor / maior",
        "DELETE / BACKSPACE   apagar selecao",
        "1                   guardar (editor)",
        "2                   testar (editor)",
        "3                   validar (editor)",
    };

    float y = 255.0f;
    for (const char* line : lines) {
        font->drawText(cmd, textPipeline->layout(), line,
                       66.0f, y, 0.43f, 0.82f, 0.84f, 0.90f, 1.0f);
        y -= 18.0f;
    }

    centerText(cmd, textPipeline, font,
               "ESC OU 0 PARA VOLTAR",
               centerX, 55.0f, 0.42f, 0.55f, 0.60f, 0.72f);
}

std::string formatTimer(float seconds) {
    if (seconds < 0.0f) seconds = 0.0f;
    const int total = static_cast<int>(seconds);
    const int minutes = total / 60;
    const int secs = total % 60;
    char buffer[16]{};
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, secs);
    return buffer;
}

} // namespace

void UiRenderer::drawTimer(VkCommandBuffer cmd, TextPipeline* textPipeline,
                           FontRenderer* font, float elapsedSeconds) const {
    if (!textPipeline || !font || !textPipeline->isInitialized()) return;
    bindText(cmd, textPipeline, font);
    const std::string timer = formatTimer(elapsedSeconds);
    const float scale = 0.7f;
    font->drawText(cmd, textPipeline->layout(), timer.c_str(),
                   config::LOGICAL_WIDTH - font->textWidth(timer.c_str(), scale) - 16.0f,
                   config::LOGICAL_HEIGHT - 28.0f,
                   scale, 0.90f, 0.90f, 0.90f, 0.85f);
}

void UiRenderer::drawPaused(VkCommandBuffer cmd,
                            const Pipeline& shapePipeline,
                            const ShapeRenderer& shapes,
                            TextPipeline* textPipeline,
                            FontRenderer* font,
                            int menuSelection) const {
    const float centerX = config::LOGICAL_WIDTH / 2.0f;
    const int count = 3;
    const float boxWidth = core::MenuBoxLayout::boxWidth(count, config::LOGICAL_WIDTH);
    const float boxHeight = core::MenuBoxLayout::BOX_H;
    const float boxY = core::MenuBoxLayout::BOX_Y;
    const char* labels[] = {"CONTINUAR", "CREDITOS", "SAIR"};

    shapes.drawRect(cmd, shapePipeline, 0.0f, 0.0f,
                    config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT,
                    0.0f, 0.0f, 0.0f, 0.55f);
    shapes.drawRect(cmd, shapePipeline, 60.0f, 245.0f,
                    520.0f, 2.0f, 0.35f, 0.35f, 0.45f);
    drawMenuBoxes(cmd, shapePipeline, shapes, count, menuSelection);

    if (!textPipeline || !font || !textPipeline->isInitialized()) return;
    bindText(cmd, textPipeline, font);
    centerText(cmd, textPipeline, font, "PAUSA", centerX, 275.0f,
               1.3f, 1.0f, 0.85f, 0.10f);
    for (int i = 0; i < count; ++i) {
        const bool selected = menuSelection == i;
        const float r = selected ? 1.0f : 0.55f;
        const float g = selected ? 0.85f : 0.55f;
        const float b = selected ? 0.10f : 0.62f;
        centerText(cmd, textPipeline, font, labels[i],
                   core::MenuBoxLayout::boxX(i, count, config::LOGICAL_WIDTH) + boxWidth / 2.0f,
                   boxY + boxHeight / 2.0f - 6.5f, 0.5f, r, g, b);
    }
    centerText(cmd, textPipeline, font,
               "A/D NAVEGAR   ESPACO CONFIRMAR   ESC CONTINUAR",
               centerX, 100.0f, 0.42f, 0.60f, 0.60f, 0.70f);
}

void UiRenderer::drawMenu(VkCommandBuffer cmd,
                          const Pipeline& shapePipeline,
                          const ShapeRenderer& shapes,
                          TextPipeline* textPipeline,
                          FontRenderer* font,
                          int menuSelection) const {
    const float centerX = config::LOGICAL_WIDTH / 2.0f;

    if (menuSelection == 4) {
        drawControlsReference(cmd, shapePipeline, shapes, textPipeline, font);
        return;
    }

    const int count = 4;
    const float boxWidth = core::MenuBoxLayout::boxWidth(count, config::LOGICAL_WIDTH);
    const float boxHeight = core::MenuBoxLayout::BOX_H;
    const float boxY = core::MenuBoxLayout::BOX_Y;
    const char* labels[] = {"COMECAR", "EDITOR", "CREDITOS", "SAIR"};

    shapes.drawRect(cmd, shapePipeline, 60.0f, 287.0f,
                    520.0f, 2.0f, 0.28f, 0.28f, 0.38f);
    drawMenuBoxes(cmd, shapePipeline, shapes, count, menuSelection);

    if (!textPipeline || !font || !textPipeline->isInitialized()) return;
    bindText(cmd, textPipeline, font);
    centerText(cmd, textPipeline, font, "ASCENDENDO",
               centerX, 300.0f, 1.6f, 0.95f, 0.80f, 0.10f);

    for (int i = 0; i < count; ++i) {
        const bool selected = menuSelection == i;
        const float r = selected ? 1.0f : 0.55f;
        const float g = selected ? 0.85f : 0.55f;
        const float b = selected ? 0.10f : 0.62f;
        centerText(cmd, textPipeline, font, labels[i],
                   core::MenuBoxLayout::boxX(i, count, config::LOGICAL_WIDTH) + boxWidth / 2.0f,
                   boxY + boxHeight / 2.0f - 6.5f,
                   0.43f, r, g, b);
    }

    centerText(cmd, textPipeline, font,
               "A/D NAVEGAR   ESPACO CONFIRMAR   0 CONTROLOS   Q SAIR",
               centerX, 80.0f, 0.38f, 0.30f, 0.30f, 0.42f);
}

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
               centerX, 298.0f, 1.5f, 1.0f, 0.85f, 0.10f);
    centerText(cmd, textPipeline, font, "FIM DA CAMPANHA",
               centerX, 266.0f, 0.75f, 0.80f, 0.80f, 0.80f);
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
               centerX, 55.0f, 0.45f, 0.40f, 0.40f, 0.55f);
}

} // namespace gfx
