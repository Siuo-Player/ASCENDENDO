// =============================================================================
// Game/Graphics/WorldRenderer.cpp
// =============================================================================
#include "Graphics/WorldRenderer.h"
#include "Graphics/ShapeRenderer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/SpritePipeline.h"
#include "Graphics/SpriteRenderer.h"
#include "Graphics/Camera.h"
#include "Graphics/PresentationConfig.h"

namespace gfx {

void WorldRenderer::draw(VkCommandBuffer cmd,
                         const Pipeline& shapePipeline,
                         const ShapeRenderer& shapes,
                         const RenderSnapshot& snapshot,
                         const Camera& camera,
                         SpritePipeline* spritePipeline,
                         SpriteRenderer* sprite) const {
    if (cmd == VK_NULL_HANDLE || !shapePipeline.isInitialized()) return;

    shapes.bind(cmd, shapePipeline);

    if (snapshot.semanticPlatformsValid) {
        for (const auto& platform : snapshot.semanticPlatformCells) {
            shapes.drawRect(cmd, shapePipeline,
                            platform.worldX, platform.worldY,
                            static_cast<float>(compositor::CELL_SIZE),
                            static_cast<float>(compositor::CELL_SIZE),
                            presentation::COLOR_PLATFORM_R,
                            presentation::COLOR_PLATFORM_G,
                            presentation::COLOR_PLATFORM_B,
                            1.0f, &camera);
        }
    } else {
        // Preserve the legacy rectangle path for levels whose presentation
        // geometry cannot be represented by the 16x16 semantic compositor.
        for (const auto& platform : snapshot.platforms) {
            shapes.drawRect(cmd, shapePipeline,
                            platform.x, platform.y,
                            platform.width, platform.height,
                            presentation::COLOR_PLATFORM_R,
                            presentation::COLOR_PLATFORM_G,
                            presentation::COLOR_PLATFORM_B,
                            1.0f, &camera);
        }
    }

    if (snapshot.flag.visible) {
        const auto& bounds = snapshot.flag.bounds;
        const float midX = bounds.x + bounds.width * 0.5f;

        shapes.drawRect(cmd, shapePipeline,
                        midX - 2.0f, bounds.y,
                        4.0f, bounds.height,
                        presentation::COLOR_FLAG_POLE_R,
                        presentation::COLOR_FLAG_POLE_G,
                        presentation::COLOR_FLAG_POLE_B,
                        1.0f, &camera);

        const float flagWidth = bounds.width * 0.45f;
        const float flagHeight = bounds.height * 0.42f;
        shapes.drawRect(cmd, shapePipeline,
                        midX + 2.0f, bounds.y + bounds.height - flagHeight,
                        flagWidth, flagHeight,
                        presentation::COLOR_FLAG_R,
                        presentation::COLOR_FLAG_G,
                        presentation::COLOR_FLAG_B,
                        1.0f, &camera);
        shapes.drawRect(cmd, shapePipeline,
                        midX + 2.0f,
                        bounds.y + bounds.height - flagHeight + flagHeight * 0.35f,
                        flagWidth, flagHeight * 0.18f,
                        0.6f, 0.45f, 0.0f, 0.85f, &camera);
    }

    // Ghost rendering (speedrun replay overlay)
    if (snapshot.ghost.visible) {
        const auto& ghost = snapshot.ghost;
        if (spritePipeline && sprite) {
            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline->handle());
            sprite->bind(cmd, spritePipeline->layout());
            sprite->draw(cmd, spritePipeline->layout(),
                         ghost.bounds.x, ghost.bounds.y,
                         ghost.bounds.width, ghost.bounds.height,
                         ghost.facingDirection < 0.0f,
                         presentation::COLOR_GHOST_R,
                         presentation::COLOR_GHOST_G,
                         presentation::COLOR_GHOST_B,
                         ghost.alpha,
                         camera.position.x, camera.position.y);
            shapes.bind(cmd, shapePipeline);
        } else {
            shapes.drawRect(cmd, shapePipeline,
                            ghost.bounds.x, ghost.bounds.y,
                            ghost.bounds.width, ghost.bounds.height,
                            presentation::COLOR_GHOST_R,
                            presentation::COLOR_GHOST_G,
                            presentation::COLOR_GHOST_B,
                            ghost.alpha,
                            &camera);
        }
    }

    const auto& player = snapshot.player;
    // Game Feel (Steve Swink 2008 & Maddy Thorson 2018):
    // Deformação visual de antecipação (squash) proporcional à energia acumulada.
    // A colisão física canónica do PhysicsBody permanece estritamente 16x16 e determinística.
    const float squashY = 1.0f - 0.12f * player.chargeRatio;
    const float stretchX = 1.0f + 0.10f * player.chargeRatio;
    const float playerDrawW = player.bounds.width * stretchX;
    const float playerDrawH = player.bounds.height * squashY;
    const float playerDrawX = player.bounds.x - (playerDrawW - player.bounds.width) * 0.5f;
    const float playerDrawY = player.bounds.y;

    if (spritePipeline && sprite) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline->handle());
        sprite->bind(cmd, spritePipeline->layout());
        sprite->draw(cmd, spritePipeline->layout(),
                     playerDrawX, playerDrawY,
                     playerDrawW, playerDrawH,
                     player.facingDirection < 0.0f,
                     1.0f, 1.0f, 1.0f, 1.0f,
                     camera.position.x, camera.position.y);
        shapes.bind(cmd, shapePipeline);
    } else {
        shapes.drawRect(cmd, shapePipeline,
                        playerDrawX, playerDrawY,
                        playerDrawW, playerDrawH,
                        presentation::COLOR_PLAYER_R,
                        presentation::COLOR_PLAYER_G,
                        presentation::COLOR_PLAYER_B,
                        1.0f, &camera);
    }

    // Commitment Jump: barra de carga / forca de salto visivel acima do jogador
    if (player.chargeRatio > 0.01f) {
        const float barX = player.bounds.x - 2.0f;
        const float barY = playerDrawY + playerDrawH + 3.0f;
        const float barW = player.bounds.width + 4.0f;
        const float barH = 3.0f;
        shapes.drawRect(cmd, shapePipeline, barX - 1.0f, barY - 1.0f,
                        barW + 2.0f, barH + 2.0f,
                        0.05f, 0.05f, 0.08f, 0.85f, &camera);
        const float r = 0.85f + 0.15f * player.chargeRatio;
        const float g = 0.85f * (1.0f - player.chargeRatio * 0.55f);
        const float b = 0.15f * (1.0f - player.chargeRatio);
        shapes.drawRect(cmd, shapePipeline, barX, barY,
                        barW * player.chargeRatio, barH,
                        r, g, b, 0.95f, &camera);
    }
}

} // namespace gfx
