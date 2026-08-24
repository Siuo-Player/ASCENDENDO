// =============================================================================
// Game/Graphics/WorldRenderer.cpp
// =============================================================================
#include "Graphics/WorldRenderer.h"
#include "Graphics/ShapeRenderer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/SpritePipeline.h"
#include "Graphics/SpriteRenderer.h"
#include "Graphics/Camera.h"
#include "Logic/Player.h"
#include "Logic/Level.h"
#include "Core/Config.h"

namespace gfx {

void WorldRenderer::draw(VkCommandBuffer cmd,
                         const Pipeline& shapePipeline,
                         const ShapeRenderer& shapes,
                         const logic::Player& player,
                         const Camera& camera,
                         const logic::Level* level,
                         SpritePipeline* spritePipeline,
                         SpriteRenderer* sprite) const {
    if (cmd == VK_NULL_HANDLE || !shapePipeline.isInitialized()) return;

    shapes.bind(cmd, shapePipeline);

    if (level) {
        for (const auto& platform : level->platforms()) {
            shapes.drawRect(cmd, shapePipeline,
                            platform.bounds.min.x, platform.bounds.min.y,
                            platform.bounds.width(), platform.bounds.height(),
                            config::COLOR_PLATFORM_R,
                            config::COLOR_PLATFORM_G,
                            config::COLOR_PLATFORM_B,
                            1.0f, &camera);
        }

        if (level->hasFlag) {
            const auto& bounds = level->flagBounds;
            const float midX = bounds.min.x + bounds.width() * 0.5f;

            shapes.drawRect(cmd, shapePipeline,
                            midX - 2.0f, bounds.min.y,
                            4.0f, bounds.height(),
                            config::COLOR_FLAG_POLE_R,
                            config::COLOR_FLAG_POLE_G,
                            config::COLOR_FLAG_POLE_B,
                            1.0f, &camera);

            const float flagWidth = bounds.width() * 0.45f;
            const float flagHeight = bounds.height() * 0.42f;
            shapes.drawRect(cmd, shapePipeline,
                            midX + 2.0f, bounds.max.y - flagHeight,
                            flagWidth, flagHeight,
                            config::COLOR_FLAG_R,
                            config::COLOR_FLAG_G,
                            config::COLOR_FLAG_B,
                            1.0f, &camera);
            shapes.drawRect(cmd, shapePipeline,
                            midX + 2.0f,
                            bounds.max.y - flagHeight + flagHeight * 0.35f,
                            flagWidth, flagHeight * 0.18f,
                            0.6f, 0.45f, 0.0f, 0.85f, &camera);
        }
    }

    if (spritePipeline && sprite) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline->handle());
        sprite->bind(cmd, spritePipeline->layout());
        const bool flip = player.facingDirection < 0.0f;
        sprite->draw(cmd, spritePipeline->layout(),
                     player.position().x, player.position().y,
                     player.body.width, player.body.height, flip,
                     1.0f, 1.0f, 1.0f, 1.0f,
                     camera.position.x, camera.position.y);
        shapes.bind(cmd, shapePipeline);
    } else {
        shapes.drawRect(cmd, shapePipeline,
                        player.position().x, player.position().y,
                        player.body.width, player.body.height,
                        config::COLOR_PLAYER_R,
                        config::COLOR_PLAYER_G,
                        config::COLOR_PLAYER_B,
                        1.0f, &camera);
    }
}

} // namespace gfx
