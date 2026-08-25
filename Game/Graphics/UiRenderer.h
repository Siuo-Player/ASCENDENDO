#pragma once
// =============================================================================
// Game/Graphics/UiRenderer.h
//
// UI/presentation pass: menus, pause, credits and gameplay HUD.
// Não conhece Player/Level internamente; recebe apenas os dados necessários.
// =============================================================================

#include <vulkan/vulkan.h>

namespace gfx {

class Pipeline;
class ShapeRenderer;
class TextPipeline;
class FontRenderer;

class UiRenderer {
public:
    void drawTimer(VkCommandBuffer cmd, TextPipeline* textPipeline,
                   FontRenderer* font, float elapsedSeconds) const;

    void drawPaused(VkCommandBuffer cmd,
                    const Pipeline& shapePipeline,
                    const ShapeRenderer& shapes,
                    TextPipeline* textPipeline,
                    FontRenderer* font,
                    int menuSelection) const;

    void drawMenu(VkCommandBuffer cmd,
                  const Pipeline& shapePipeline,
                  const ShapeRenderer& shapes,
                  TextPipeline* textPipeline,
                  FontRenderer* font,
                  int menuSelection) const;

    void drawCredits(VkCommandBuffer cmd,
                     const Pipeline& shapePipeline,
                     const ShapeRenderer& shapes,
                     TextPipeline* textPipeline,
                     FontRenderer* font) const;
};

} // namespace gfx
