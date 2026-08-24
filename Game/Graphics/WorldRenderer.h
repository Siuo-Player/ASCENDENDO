#pragma once
// =============================================================================
// Game/Graphics/WorldRenderer.h
//
// Pass de cena jogável: plataformas, FLAG e jogador.
// Não conhece GameState, menus ou editor.
// =============================================================================

#include <vulkan/vulkan.h>

namespace logic { class Player; class Level; }
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
              const logic::Player& player,
              const Camera& camera,
              const logic::Level* level,
              SpritePipeline* spritePipeline = nullptr,
              SpriteRenderer* sprite = nullptr) const;
};

} // namespace gfx
