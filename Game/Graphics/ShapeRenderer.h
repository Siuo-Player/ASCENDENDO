#pragma once
// =============================================================================
// Game/Graphics/ShapeRenderer.h
//
// Primitive renderer partilhado pelos novos passes. Sem estado de cena.
// =============================================================================

#include <vulkan/vulkan.h>

namespace gfx {

class Pipeline;
class Camera;

class ShapeRenderer {
public:
    ShapeRenderer() = default;

    bool bind(VkCommandBuffer cmd, const Pipeline& pipeline) const;

    void drawRect(VkCommandBuffer cmd, const Pipeline& pipeline,
                  float x, float y, float w, float h,
                  float r, float g, float b, float a = 1.0f,
                  const Camera* camera = nullptr) const;
};

} // namespace gfx
