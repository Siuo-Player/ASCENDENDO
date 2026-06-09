// =============================================================================
//  Game/Graphics/Camera.cpp
//
//  @version 4.1
// =============================================================================
#include "Graphics/Camera.h"
#include "Core/Config.h" // Acesso às variáveis globais

namespace gfx {

logic::Vec2 Camera::worldToNDC(const logic::Vec2& worldPos) const {
    logic::Vec2 ndc;

    float screenX = worldPos.x - position.x;
    ndc.x = (screenX / config::LOGICAL_WIDTH) * 2.0f - 1.0f;

    float screenY = worldPos.y - position.y;
    ndc.y = -((screenY / config::LOGICAL_HEIGHT) * 2.0f - 1.0f);

    return ndc;
}

} // namespace gfx