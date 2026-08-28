// =============================================================================
// Game/Graphics/WorldRenderer.cpp
// =============================================================================
#include "Graphics/WorldRenderer.h"
#include "Graphics/ShapeRenderer.h"
#include "Graphics/Pipeline.h"
#include "Graphics/SpritePipeline.h"
#include "Graphics/SpriteRenderer.h"
#include "Graphics/Camera.h"
#include "Core/Config.h"

namespace gfx {

void WorldRenderer::draw(VkCommandBuffer cmd,
                         const Pipeline& shapePipeline,
                         const ShapeRenderer& shapes,
                         const core::RenderSnapshot& snapshot,
                         const Camera& camera,
                         SpritePipeline* spritePipeline,
                         SpriteRenderer* sprite) const {
    if (cmd == VK_NULL_HANDLE || !shapePipeline.isInitialized()) return;

    shapes.bind(cmd, shapePipeline);

    for (const auto& platform : snapshot.platforms) {
        shapes.drawRect(cmd, shapePipeline,
                        platform.x, platform.y,
                        platform.width, platform.height,
                        config::COLOR_PLATFORM_R,
                        config::COLOR_PLATFORM_G,
                        config::COLOR_PLATFORM_B,
                        1.0f, &camera);
    }

    if (snapshot.flag.visible) {
        const auto& bounds = snapshot.flag.bounds;
        const float midX = bounds.x + bounds.width * 0.5f;

        shapes.drawRect(cmd, shapePipeline,
                        midX - 2.0f, bounds.y,
                        4.0f, bounds.height,
                        config::COLOR_FLAG_POLE_R,
                        config::COLOR_FLAG_POLE_G,
                        config::COLOR_FLAG_POLE_B,
                        1.0f, &camera);

        const float flagWidth = bounds.width * 0.45f;
        const float flagHeight = bounds.height * 0.42f;
        shapes.drawRect(cmd, shapePipeline,
                        midX + 2.0f, bounds.y + bounds.height - flagHeight,
                        flagWidth, flagHeight,
                        config::COLOR_FLAG_R,
                        config::COLOR_FLAG_G,
                        config::COLOR_FLAG_B,
                        1.0f, &camera);
        shapes.drawRect(cmd, shapePipeline,
                        midX + 2.0f,
                        bounds.y + bounds.height - flagHeight + flagHeight * 0.35f,
                        flagWidth, flagHeight * 0.18f,
                        0.6f, 0.45f, 0.0f, 0.85f, &camera);
    }

    const auto& player = snapshot.player;
    if (spritePipeline && sprite) {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, spritePipeline->handle());
        sprite->bind(cmd, spritePipeline->layout());
        sprite->draw(cmd, spritePipeline->layout(),
                     player.bounds.x, player.bounds.y,
                     player.bounds.width, player.bounds.height,
                     player.facingLeft,
                     1.0f, 1.0f, 1.0f, 1.0f,
                     camera.position.x, camera.position.y);
        shapes.bind(cmd, shapePipeline);
    } else {
        shapes.drawRect(cmd, shapePipeline,
                        player.bounds.x, player.bounds.y,
                        player.bounds.width, player.bounds.height,
                        config::COLOR_PLAYER_R,
                        config::COLOR_PLAYER_G,
                        config::COLOR_PLAYER_B,
                        1.0f, &camera);
    }
}

} // namespace gfx
