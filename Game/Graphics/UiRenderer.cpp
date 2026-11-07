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
#include <filesystem>
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
    shapes.drawRect(cmd, shapePipeline, 44.0f, 30.0f,
                    552.0f, 298.0f, 0.04f, 0.04f, 0.08f, 0.94f);
    shapes.drawRect(cmd, shapePipeline, 44.0f, 326.0f,
                    552.0f, 2.0f, 0.45f, 0.45f, 0.60f);

    if (!textPipeline || !font || !textPipeline->isInitialized()) return;
    bindText(cmd, textPipeline, font);

    centerText(cmd, textPipeline, font, "CONTROLOS",
               centerX, 304.0f, 1.15f, 0.95f, 0.80f, 0.10f);

    const char* lines[] = {
        "A / D ou SETAS        mover / navegar menu",
        "W / S ou TAB          alternar campanha no menu",
        "ESPACO                confirmar / saltar (segurar para carregar)",
        "G                     alternar fantasma speedrun / modo editor",
        "ESC                   voltar / pausa",
        "Q                     sair / voltar ao menu",
        "E                     editor de nivel",
        "C                     editor de campanha",
        "0                     ver controlos",
        "[ / ]                 tamanho menor / maior",
        "DELETE / BACKSPACE    apagar selecao",
        "1                     guardar",
        "2                     testar (editor de nivel)",
        "3                     validar (editor de nivel)",
    };

    float y = 280.0f;
    for (const char* line : lines) {
        font->drawText(cmd, textPipeline->layout(), line,
                       56.0f, y, 0.40f, 0.82f, 0.84f, 0.90f, 1.0f);
        y -= 17.0f;
    }

    centerText(cmd, textPipeline, font,
               "ESC OU 0 PARA VOLTAR",
               centerX, 42.0f, 0.42f, 0.55f, 0.60f, 0.72f);
}

std::string formatTimerHundredths(float seconds) {
    if (seconds < 0.0f) seconds = 0.0f;
    const int totalSecs = static_cast<int>(seconds);
    const int minutes = totalSecs / 60;
    const int secs = totalSecs % 60;
    const int centis = static_cast<int>((seconds - static_cast<float>(totalSecs)) * 100.0f) % 100;
    char buffer[24]{};
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d.%02d", minutes, secs, centis);
    return buffer;
}

} // namespace

void UiRenderer::drawTimer(VkCommandBuffer cmd, TextPipeline* textPipeline,
                           FontRenderer* font, float elapsedSeconds) const {
    if (!textPipeline || !font || !textPipeline->isInitialized()) return;
    bindText(cmd, textPipeline, font);
    const std::string timer = formatTimerHundredths(elapsedSeconds);
    const float scale = 0.7f;
    font->drawText(cmd, textPipeline->layout(), timer.c_str(),
                   config::LOGICAL_WIDTH - font->textWidth(timer.c_str(), scale) - 16.0f,
                   config::LOGICAL_HEIGHT - 28.0f,
                   scale, 0.90f, 0.90f, 0.90f, 0.85f);
}

void UiRenderer::drawGameplayHud(VkCommandBuffer cmd,
                                 const Pipeline& shapePipeline,
                                 const ShapeRenderer& shapes,
                                 TextPipeline* textPipeline,
                                 FontRenderer* font,
                                 const RenderHudSnapshot& hud) const {
    // Fundo discreto para o HUD superior direito
    shapes.drawRect(cmd, shapePipeline,
                    config::LOGICAL_WIDTH - 180.0f, config::LOGICAL_HEIGHT - 48.0f,
                    170.0f, 42.0f, 0.02f, 0.03f, 0.05f, 0.65f);
    shapes.drawRect(cmd, shapePipeline,
                    config::LOGICAL_WIDTH - 180.0f, config::LOGICAL_HEIGHT - 48.0f,
                    170.0f, 1.0f, 0.25f, 0.28f, 0.38f, 0.70f);

    // Fundo discreto para o HUD superior esquerdo
    shapes.drawRect(cmd, shapePipeline,
                    10.0f, config::LOGICAL_HEIGHT - 48.0f,
                    190.0f, 42.0f, 0.02f, 0.03f, 0.05f, 0.65f);
    shapes.drawRect(cmd, shapePipeline,
                    10.0f, config::LOGICAL_HEIGHT - 48.0f,
                    190.0f, 1.0f, 0.25f, 0.28f, 0.38f, 0.70f);

    if (!textPipeline || !font || !textPipeline->isInitialized()) return;
    bindText(cmd, textPipeline, font);

    // 1. Cronómetro principal (direita)
    const std::string timer = formatTimerHundredths(hud.elapsedTime);
    const float timerScale = 0.65f;
    const float timerW = font->textWidth(timer.c_str(), timerScale);
    font->drawText(cmd, textPipeline->layout(), timer.c_str(),
                   config::LOGICAL_WIDTH - timerW - 18.0f,
                   config::LOGICAL_HEIGHT - 24.0f,
                   timerScale, 0.98f, 0.98f, 0.98f, 1.0f);

    // 2. Personal Best (PB) e Ghost status
    if (hud.personalBestTime > 0.0f) {
        char pbBuf[32]{};
        std::snprintf(pbBuf, sizeof(pbBuf), "PB %s",
                      formatTimerHundredths(hud.personalBestTime).c_str());
        font->drawText(cmd, textPipeline->layout(), pbBuf,
                       config::LOGICAL_WIDTH - 172.0f,
                       config::LOGICAL_HEIGHT - 42.0f,
                       0.38f, 0.95f, 0.82f, 0.20f, 0.95f);
    }

    if (hud.ghostAvailable) {
        if (hud.ghostVisible) {
            // Delta de altitude face ao fantasma
            char deltaBuf[32]{};
            float deltaR = 0.75f, deltaG = 0.75f, deltaB = 0.75f;
            if (hud.ghostDeltaMeters > 0.5f) {
                std::snprintf(deltaBuf, sizeof(deltaBuf), "+%.0fm", hud.ghostDeltaMeters);
                deltaR = 0.20f; deltaG = 1.0f; deltaB = 0.40f;
            } else if (hud.ghostDeltaMeters < -0.5f) {
                std::snprintf(deltaBuf, sizeof(deltaBuf), "%.0fm", hud.ghostDeltaMeters);
                deltaR = 1.0f; deltaG = 0.35f; deltaB = 0.35f;
            } else {
                std::snprintf(deltaBuf, sizeof(deltaBuf), "= 0m");
            }
            font->drawText(cmd, textPipeline->layout(), deltaBuf,
                           config::LOGICAL_WIDTH - 65.0f,
                           config::LOGICAL_HEIGHT - 42.0f,
                           0.38f, deltaR, deltaG, deltaB, 1.0f);
        } else {
            font->drawText(cmd, textPipeline->layout(), "[G] OFF",
                           config::LOGICAL_WIDTH - 65.0f,
                           config::LOGICAL_HEIGHT - 42.0f,
                           0.35f, 0.50f, 0.52f, 0.58f, 0.80f);
        }
    }

    // 3. Campanha e Altitude (esquerda)
    if (hud.campaignName[0] != '\0') {
        font->drawText(cmd, textPipeline->layout(), hud.campaignName,
                       18.0f, config::LOGICAL_HEIGHT - 23.0f,
                       0.44f, 0.85f, 0.88f, 0.95f, 1.0f);
    }
    char altBuf[32]{};
    std::snprintf(altBuf, sizeof(altBuf), "ALTURA: %.0fm", hud.altitudeMeters);
    font->drawText(cmd, textPipeline->layout(), altBuf,
                   18.0f, config::LOGICAL_HEIGHT - 42.0f,
                   0.40f, 0.60f, 0.88f, 0.95f, 0.90f);
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
                          int menuSelection,
                          const MenuRenderSnapshot* snapshot) const {
    const float centerX = config::LOGICAL_WIDTH / 2.0f;

    if (menuSelection == 4) {
        drawControlsReference(cmd, shapePipeline, shapes, textPipeline, font);
        return;
    }

    // 1. Selector de Campanha Interactivo (se catalogo presente)
    if (snapshot && !snapshot->campaigns.empty()) {
        const std::size_t activeIdx = snapshot->activeCampaignIndex < snapshot->campaigns.size()
                                          ? snapshot->activeCampaignIndex
                                          : 0;
        const auto& activeCamp = snapshot->campaigns[activeIdx];

        // Caixa do selector de campanha
        shapes.drawRect(cmd, shapePipeline, 44.0f, 222.0f, 552.0f, 54.0f,
                        0.035f, 0.045f, 0.08f, 0.92f);
        shapes.drawRect(cmd, shapePipeline, 44.0f, 274.0f, 552.0f, 2.0f,
                        0.85f, 0.75f, 0.20f, 0.90f);
        shapes.drawRect(cmd, shapePipeline, 44.0f, 222.0f, 552.0f, 1.5f,
                        0.35f, 0.38f, 0.50f, 0.60f);

        if (textPipeline && font && textPipeline->isInitialized()) {
            bindText(cmd, textPipeline, font);

            // Setas decorativas de navegacao
            font->drawText(cmd, textPipeline->layout(), "<", 58.0f, 248.0f,
                           0.70f, 0.95f, 0.80f, 0.10f, 1.0f);
            font->drawText(cmd, textPipeline->layout(), ">", 572.0f, 248.0f,
                           0.70f, 0.95f, 0.80f, 0.10f, 1.0f);

            // Titulo e indice da campanha
            char titleBuf[128]{};
            std::snprintf(titleBuf, sizeof(titleBuf), "CAMPANHA [%02zu/%02zu]: %s",
                          activeIdx + 1, snapshot->campaigns.size(), activeCamp.name.c_str());
            centerText(cmd, textPipeline, font, titleBuf, centerX, 258.0f,
                       0.48f, 1.0f, 0.92f, 0.45f, 1.0f);

            // Estatisticas / informacoes
            char statsBuf[160]{};
            if (activeCamp.bestTimeSeconds > 0.0f) {
                std::snprintf(statsBuf, sizeof(statsBuf),
                              "%zu NIVEIS   |   RECORDE: %s   |   VITORIAS: %u",
                              activeCamp.levelCount,
                              formatTimerHundredths(activeCamp.bestTimeSeconds).c_str(),
                              activeCamp.completions);
            } else {
                std::snprintf(statsBuf, sizeof(statsBuf),
                              "%zu NIVEIS   |   RECORDE: --:--   |   POR CONQUISTAR",
                              activeCamp.levelCount);
            }
            centerText(cmd, textPipeline, font, statsBuf, centerX, 240.0f,
                       0.38f, 0.82f, 0.86f, 0.95f, 1.0f);

            // Dica de atalho
            centerText(cmd, textPipeline, font,
                       "W/S ou TAB para alternar campanha",
                       centerX, 228.0f, 0.30f, 0.45f, 0.70f, 0.85f, 0.90f);
        }
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
               centerX, 302.0f, 1.55f, 0.95f, 0.80f, 0.10f);

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
               "A/D NAVEGAR   ESPACO CONFIRMAR   W/S CAMPANHA   0 CONTROLOS   Q SAIR",
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

void UiRenderer::drawCampaignEditor(
    VkCommandBuffer cmd,
    const Pipeline& shapePipeline,
    const ShapeRenderer& shapes,
    TextPipeline* textPipeline,
    FontRenderer* font,
    const logic::CampaignEditorRenderSnapshot& snapshot) const {
    const float centerX = config::LOGICAL_WIDTH / 2.0f;

    shapes.drawRect(cmd, shapePipeline, 28.0f, 24.0f,
                    584.0f, 312.0f, 0.025f, 0.035f, 0.065f, 0.96f);
    shapes.drawRect(cmd, shapePipeline, 28.0f, 309.0f,
                    584.0f, 2.0f, 0.45f, 0.48f, 0.64f);

    if (!textPipeline || !font || !textPipeline->isInitialized()) return;
    bindText(cmd, textPipeline, font);

    centerText(cmd, textPipeline, font, "EDITOR DE CAMPANHA",
               centerX, 324.0f, 0.92f, 0.95f, 0.80f, 0.10f);

    std::string header = std::to_string(snapshot.levels.size()) + " NIVEIS";
    if (snapshot.dirty) header += "   * NAO GUARDADO";
    font->drawText(cmd, textPipeline->layout(), header.c_str(),
                   48.0f, 292.0f, 0.44f, 0.72f, 0.75f, 0.86f, 1.0f);

    const std::size_t count = snapshot.levels.size();
    if (count > 0) {
        const std::size_t selected =
            snapshot.selectedIndex < count ? snapshot.selectedIndex : 0;
        constexpr std::size_t visibleRows = 8;
        std::size_t first = selected > visibleRows / 2 ? selected - visibleRows / 2 : 0;
        if (first + visibleRows > count && count > visibleRows)
            first = count - visibleRows;
        const std::size_t last = std::min(count, first + visibleRows);

        float y = 263.0f;
        for (std::size_t i = first; i < last; ++i) {
            const auto& level = snapshot.levels[i];
            const bool isSelected = i == selected;
            const float rowY = y;
            shapes.drawRect(cmd, shapePipeline, 44.0f, rowY - 6.0f,
                            552.0f, 27.0f,
                            isSelected ? 0.16f : 0.045f,
                            isSelected ? 0.14f : 0.050f,
                            isSelected ? 0.025f : 0.085f,
                            0.96f);
            shapes.drawRect(cmd, shapePipeline, 44.0f, rowY - 6.0f,
                            3.0f, 27.0f,
                            isSelected ? 1.0f : 0.25f,
                            isSelected ? 0.82f : 0.35f,
                            isSelected ? 0.10f : 0.42f,
                            1.0f);

            const std::string number = std::to_string(i + 1) + ".";
            const std::string filename = std::filesystem::path(level.path).filename().string();
            font->drawText(cmd, textPipeline->layout(), number.c_str(),
                           56.0f, rowY + 2.0f, 0.44f,
                           isSelected ? 1.0f : 0.66f,
                           isSelected ? 0.85f : 0.68f,
                           isSelected ? 0.10f : 0.74f, 1.0f);
            font->drawText(cmd, textPipeline->layout(), level.name.c_str(),
                           88.0f, rowY + 3.0f, 0.47f, 0.92f, 0.93f, 0.96f, 1.0f);
            font->drawText(cmd, textPipeline->layout(), filename.c_str(),
                           390.0f, rowY + 3.0f, 0.32f, 0.52f, 0.55f, 0.68f, 1.0f);
            y -= 31.0f;
        }
    } else {
        centerText(cmd, textPipeline, font, "CAMPANHA SEM NIVEIS",
                   centerX, 175.0f, 0.65f, 0.82f, 0.36f, 0.32f);
    }

    centerText(cmd, textPipeline, font,
               "UP/DOWN SELECIONAR   LEFT/RIGHT REORDENAR",
               centerX, 55.0f, 0.38f, 0.56f, 0.60f, 0.72f);
    centerText(cmd, textPipeline, font,
               "ESPACO ABRIR   1 GUARDAR   ESC VOLTAR",
               centerX, 38.0f, 0.38f, 0.56f, 0.60f, 0.72f);
}

} // namespace gfx
