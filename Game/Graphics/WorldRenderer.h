#pragma once
// =============================================================================
// Game/Graphics/WorldRenderer.h
//
// Pass de cena jogável: plataformas, FLAG e jogador.
// Consome apenas dados de apresentação.
// =============================================================================

#include <vulkan/vulkan.h>
#include "Graphics/RenderSnapshot.h"

namespace gfx {
class Camera;
class Pipeline;
class ShapeRenderer;
class SpritePipeline;
class SpriteRenderer;

class WorldRenderer {
public:
    void draw(VkCommandBuffer cmd,
              const Pipeline& shapePipeline,
              const ShapeRenderer& shapes,
              const RenderSnapshot& snapshot,
              const Camera& camera,
              SpritePipeline* spritePipeline = nullptr,
              SpriteRenderer* sprite = nullptr) const;
};

} // namespace gfx
