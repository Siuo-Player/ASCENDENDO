// =============================================================================
//  Game/Logic/Physics.cpp
//
//  @version 3.1
//  @history
//    v3.1 — criado
// =============================================================================

#include "Logic/Physics.h"

namespace logic {

// ── AABB ──────────────────────────────────────────────────────────────────────

bool AABB::overlaps(const AABB& other) const {
    // Separating Axis Theorem simplificado para AABBs
    return min.x < other.max.x && max.x > other.min.x
        && min.y < other.max.y && max.y > other.min.y;
}

// ── PhysicsWorld ──────────────────────────────────────────────────────────────

void PhysicsWorld::step(PhysicsBody& body, float dt) const {
    // 1. Gravidade (só se não está no chão)
    if (!body.isGrounded) {
        body.velocity.y += GRAVITY * dt;
    }

    // 2. Integração de Euler: posição += velocidade * dt
    body.position += body.velocity * dt;

    // 3. Colisão simples com o chão (Y = GROUND_Y)
    if (body.position.y <= GROUND_Y) {
        body.position.y = GROUND_Y;
        body.velocity.y = 0.0f;
        body.isGrounded = true;
    } else {
        body.isGrounded = false;
    }
}

void PhysicsWorld::jump(PhysicsBody& body, float force) const {
    if (body.isGrounded) {
        body.velocity.y = force;
        body.isGrounded = false;
    }
    // Salto no ar ignorado — mecânica de salto por carga (Fase 6)
}

bool PhysicsWorld::collides(const AABB& a, const AABB& b) {
    return a.overlaps(b);
}

int PhysicsWorld::advance(float deltaTime) {
    m_accumulator += deltaTime;
    int steps = 0;
    while (m_accumulator >= FIXED_STEP) {
        m_accumulator -= FIXED_STEP;
        ++steps;
    }
    return steps;
}

} // namespace logic
