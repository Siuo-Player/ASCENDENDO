// =============================================================================
//  Game/Logic/Physics.cpp
//
//  @version 6.2c
//  @history
//    v3.1  — criado
//    v6.2c — paredes absolutas esquerda/direita (config::LOGICAL_WIDTH)
// =============================================================================

#include "Logic/Physics.h"
#include "Core/Config.h"

namespace logic {

bool AABB::overlaps(const AABB& other) const {
    return min.x < other.max.x && max.x > other.min.x
        && min.y < other.max.y && max.y > other.min.y;
}

void PhysicsWorld::step(PhysicsBody& body, float dt) const {
    // 1. Gravidade
    if (!body.isGrounded) {
        body.velocity.y += GRAVITY * dt;
    }

    // 2. Integração de Euler
    body.position += body.velocity * dt;

    // 3. Chão absoluto (Y = GROUND_Y)
    if (body.position.y <= GROUND_Y) {
        body.position.y = GROUND_Y;
        body.velocity.y = 0.0f;
        body.isGrounded  = true;
    } else {
        body.isGrounded = false;
    }

    // 4. Paredes absolutas esquerda e direita
    if (body.position.x < 0.0f) {
        body.position.x  = 0.0f;
        if (body.velocity.x < 0.0f) body.velocity.x = 0.0f;
    }
    const float rightWall = config::LOGICAL_WIDTH - body.width;
    if (body.position.x > rightWall) {
        body.position.x  = rightWall;
        if (body.velocity.x > 0.0f) body.velocity.x = 0.0f;
    }
}

void PhysicsWorld::jump(PhysicsBody& body, float force) const {
    if (body.isGrounded) {
        body.velocity.y = force;
        body.isGrounded = false;
    }
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
