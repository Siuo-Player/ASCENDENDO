#pragma once
// =============================================================================
// Game/Graphics/UiRenderer.h
//
// UI/presentation pass: menus, pause, credits, campaign editor and gameplay HUD.
// Não conhece Player/Level internamente; recebe apenas os dados necessários.
// =============================================================================

#include "Graphics/RenderSnapshot.h"
#include "Logic/CampaignEditorSnapshot.h"
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

    void drawGameplayHud(VkCommandBuffer cmd,
                         const Pipeline& shapePipeline,
                         const ShapeRenderer& shapes,
                         TextPipeline* textPipeline,
                         FontRenderer* font,
                         const RenderHudSnapshot& hud) const;

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
                  int menuSelection,
                  const MenuRenderSnapshot* snapshot = nullptr) const;

    void drawCredits(VkCommandBuffer cmd,
                     const Pipeline& shapePipeline,
                     const ShapeRenderer& shapes,
                     TextPipeline* textPipeline,
                     FontRenderer* font) const;

    void drawCampaignEditor(VkCommandBuffer cmd,
                            const Pipeline& shapePipeline,
                            const ShapeRenderer& shapes,
                            TextPipeline* textPipeline,
                            FontRenderer* font,
                            const logic::CampaignEditorRenderSnapshot& snapshot) const;
};

} // namespace gfx
