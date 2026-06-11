// =============================================================================
//  Game/Graphics/Camera.cpp
//
//  @version 6.4
//  @history
//    v4.1  — worldToNDC criado
//    v6.4  — follow() adicionado (tracking vertical Lerp)
// =============================================================================
#include "Graphics/Camera.h"
#include "Core/Config.h"

namespace gfx {

logic::Vec2 Camera::worldToNDC(const logic::Vec2& worldPos) const {
    logic::Vec2 ndc;
    float screenX = worldPos.x - position.x;
    ndc.x = (screenX / config::LOGICAL_WIDTH) * 2.0f - 1.0f;
    float screenY = worldPos.y - position.y;
    ndc.y = -((screenY / config::LOGICAL_HEIGHT) * 2.0f - 1.0f);
    return ndc;
}

void Camera::follow(const logic::Vec2& target, float dt, float speed) {
    // Offset vertical: jogador aparece no 35% inferior do ecra.
    // LOGICAL_HEIGHT * 0.35 = 360 * 0.35 = 126px
    constexpr float VERTICAL_OFFSET = config::LOGICAL_HEIGHT * 0.35f;

    // Y alvo: jogador a 126px do fundo do viewport
    float targetY = target.y - VERTICAL_OFFSET;
    if (targetY < 0.0f) targetY = 0.0f; // Nao mostrar abaixo do mundo

    // Lerp suave em direcao ao alvo
    position.y += (targetY - position.y) * speed * dt;

    // Garantia adicional: camera nunca vai abaixo de 0
    if (position.y < 0.0f) position.y = 0.0f;
}

} // namespace gfx
