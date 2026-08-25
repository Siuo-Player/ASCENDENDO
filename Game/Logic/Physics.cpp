// =============================================================================
//  Game/Logic/Physics.cpp
// =============================================================================

#include "Logic/Physics.h"
#include "Core/Config.h"

#include <cmath>

namespace logic {

int PhysicsWorld::advance(float dt) {
    if (!std::isfinite(dt) || dt <= 0.0f) return 0;

    if (dt > MAX_FRAME_TIME) dt = MAX_FRAME_TIME;

    m_accumulator += dt;
    int steps = 0;
    while (m_accumulator >= FIXED_STEP && steps < MAX_STEPS_PER_ADVANCE) {
        m_accumulator -= FIXED_STEP;
        ++steps;
    }

    // If a hitch filled the accumulator beyond the per-frame budget, discard
    // the excess debt instead of carrying an unbounded catch-up backlog.
    if (steps == MAX_STEPS_PER_ADVANCE && m_accumulator >= FIXED_STEP) {
        m_accumulator = 0.0f;
    }

    return steps;
}

void PhysicsWorld::step(PhysicsBody& body, float dt) {
    if (!std::isfinite(dt) || dt <= 0.0f) return;

    if (!body.isGrounded) {
        body.velocity.y += GRAVITY * dt;
    }

    body.position.x += body.velocity.x * dt;
    body.position.y += body.velocity.y * dt;

    body.isGrounded = false;

    if (body.position.x < 0.0f) {
        body.position.x = 0.0f;
        body.velocity.x = 0.0f;
    } else if (body.position.x > config::LOGICAL_WIDTH - body.width) {
        body.position.x = config::LOGICAL_WIDTH - body.width;
        body.velocity.x = 0.0f;
    }

    if (body.position.y <= 0.0f) {
        body.position.y = 0.0f;
        if (body.velocity.y < 0.0f) body.velocity.y = 0.0f;
        body.isGrounded = true;
    }
}

void PhysicsWorld::jump(PhysicsBody& body, float force) {
    if (body.isGrounded && std::isfinite(force)) {
        body.velocity.y = force;
        body.isGrounded = false;
    }
}

} // namespace logic
