// =============================================================================
//  Game/Logic/Level.cpp
//
//  @version 6.2
//  @history
//    v6.2 — criado
// =============================================================================

#include "Logic/Level.h"

namespace logic {

void Level::addPlatform(float x, float y, float w, float h) {
    m_platforms.push_back({ AABB{ {x, y}, {x + w, y + h} } });
}

bool Level::resolveCollision(PhysicsBody& body) const {
    // AABB expandida 1px para baixo (SKIN_WIDTH) para detetar corpo pousado.
    // Problema resolvido: step() coloca isGrounded=false quando pos.y > GROUND_Y,
    // mas o corpo pousado tem min.y == platform.max.y → nao haveria sobreposicao
    // sem este skin. Com ele, 119 < 120 → overlap deteclado → pousa corretamente.
    AABB probe = body.bounds();
    probe.min.y -= SKIN_WIDTH;

    for (const auto& platform : m_platforms) {
        if (!probe.overlaps(platform.bounds)) continue;

        // One-way: so pousa no topo quando o corpo esta a cair ou estacionario.
        // Salto para cima atravessa a plataforma livremente (velocity.y > 0).
        if (body.velocity.y <= 0.0f) {
            body.position.y = platform.bounds.max.y;
            body.velocity.y = 0.0f;
            body.isGrounded  = true;
            return true; // Resolver uma colisao por frame
        }
    }

    return false;
}

} // namespace logic
