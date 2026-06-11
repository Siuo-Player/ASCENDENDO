// =============================================================================
//  Game/Logic/Level.cpp
//
//  @version 6.2c
//  @history
//    v6.2  — criado (colisao de topo one-way)
//    v6.2c — colisao lateral adicionada (MTV + direção da velocidade)
//
//  ALGORITMO DE RESOLUÇÃO (dois casos):
//
//  Caso 1 — Skin overlap (corpo pousado):
//    Probe alargado 1px para baixo deteta corpo pousado sem sobreposicao real.
//    → pousa no topo se velocity.y <= 0.
//
//  Caso 2 — Sobreposicao real (MTV):
//    Calcula a saida minima em X e Y.
//    Se |velocity.x| > |velocity.y|  →  colisao lateral (empurra em X).
//    Caso contrário             →  colisao vertical:
//      - corpo a cair → pousa no topo.
//      - corpo a subir → passa livremente (one-way).
// =============================================================================

#include "Logic/Level.h"
#include <algorithm> // std::min

namespace logic {

void Level::addPlatform(float x, float y, float w, float h) {
    m_platforms.push_back({ AABB{ {x, y}, {x + w, y + h} } });
}

bool Level::resolveCollision(PhysicsBody& body) const {
    bool collided = false;

    for (const auto& platform : m_platforms) {
        AABB bodyAABB = body.bounds();

        // ── Caso 1: skin probe para persistência de isGrounded ────────────────
        AABB probe = bodyAABB;
        probe.min.y -= SKIN_WIDTH;

        if (!probe.overlaps(platform.bounds)) continue;

        if (!bodyAABB.overlaps(platform.bounds)) {
            // Só skin overlap: corpo em cima da plataforma
            if (body.velocity.y <= 0.0f) {
                body.position.y = platform.bounds.max.y;
                body.velocity.y = 0.0f;
                body.isGrounded  = true;
                collided = true;
            }
            continue; // skip MTV para esta plataforma
        }

        // ── Caso 2: sobreposicao real — resolucao MTV ─────────────────────────
        float exitLeft  = bodyAABB.max.x  - platform.bounds.min.x;
        float exitRight = platform.bounds.max.x - bodyAABB.min.x;
        float exitUp    = platform.bounds.max.y - bodyAABB.min.y;
        float exitDown  = bodyAABB.max.y  - platform.bounds.min.y;

        float minExitX = std::min(exitLeft,  exitRight);
        float minExitY = std::min(exitUp,    exitDown);

        // Decide o eixo pelo dominante na velocidade:
        // movimento predominantemente horizontal → colisao lateral.
        bool lateralCollision =
            std::abs(body.velocity.x) > std::abs(body.velocity.y)
            && minExitX <= minExitY;

        if (lateralCollision) {
            // Empurrar corpo para fora em X
            body.position.x = (exitLeft < exitRight)
                ? platform.bounds.min.x - body.width   // sai pela esquerda
                : platform.bounds.max.x;                // sai pela direita
            body.velocity.x = 0.0f;
            collided = true;

        } else {
            // Colisao vertical
            if (exitUp <= exitDown) {
                // Corpo veio de cima (topo da plataforma)
                if (body.velocity.y <= 0.0f) {          // a cair ou parado
                    body.position.y = platform.bounds.max.y;
                    body.velocity.y = 0.0f;
                    body.isGrounded  = true;
                    collided = true;
                }
                // A subir → passa livremente (one-way from below)
            }
            // exitDown < exitUp: toca por baixo → one-way → ignorar
        }
    }

    return collided;
}

} // namespace logic
