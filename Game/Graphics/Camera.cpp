// =============================================================================
//  Game/Graphics/Camera.cpp
//
//  @version 4.1
//  @history
//    v4.1 — criado
// =============================================================================

#include "Graphics/Camera.h"

namespace gfx {

logic::Vec2 Camera::worldToNDC(const logic::Vec2& worldPos) const {
    logic::Vec2 ndc;

    // Eixo X: Normalizar de [0, WIDTH] para [-1.0, 1.0]
    float screenX = worldPos.x - position.x;
    ndc.x = (screenX / LOGICAL_WIDTH) * 2.0f - 1.0f;

    // Eixo Y: O mundo cresce para cima, NDC do Vulkan cresce para baixo.
    // screenY = 0 (base da câmara) -> NDC = 1.0 (base do ecrã Vulkan)
    // screenY = HEIGHT (topo da câmara) -> NDC = -1.0 (topo do ecrã Vulkan)
    float screenY = worldPos.y - position.y;
    ndc.y = -((screenY / LOGICAL_HEIGHT) * 2.0f - 1.0f);

    return ndc;
}

} // namespace gfx