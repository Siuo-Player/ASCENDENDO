#pragma once
// =============================================================================
// Game/Graphics/EditorRenderer.h
//
// Pass gráfico do editor. Conhece apenas dados de apresentação, nunca o
// documento de edição nem InputManager.
// =============================================================================

#include "Logic/EditorRenderSnapshot.h"
#include <vulkan/vulkan.h>

namespace gfx {

class Camera;
class Pipeline;
class ShapeRenderer;
class TextPipeline;
class FontRenderer;

class EditorRenderer {
public:
    void draw(VkCommandBuffer cmd,
              const Pipeline& shapePipeline,
              const ShapeRenderer& shapes,
              const Camera& camera,
              const logic::EditorRenderSnapshot& snapshot,
              TextPipeline* textPipeline = nullptr,
              FontRenderer* font = nullptr) const;
};

} // namespace gfx
