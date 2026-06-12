// =============================================================================
//  Game/Logic/Physics.cpp
//
//  @version 7.2
// =============================================================================

#include "Logic/Physics.h"
#include "Core/Config.h"

namespace logic {

int PhysicsWorld::advance(float dt) {
    m_accumulator += dt;
    int steps = 0;
    while (m_accumulator >= FIXED_STEP) {
        m_accumulator -= FIXED_STEP;
        steps++;
    }
    return steps;
}

void PhysicsWorld::step(PhysicsBody& body, float dt) {
    // 1. Aplica Gravidade se estiver no ar
    if (!body.isGrounded) {
        body.velocity.y += GRAVITY * dt;
    }

    // 2. Integra a Velocidade na Posição
    body.position.x += body.velocity.x * dt;
    body.position.y += body.velocity.y * dt;

    // FIX: Limpa a flag de Grounded! Se o jogador andar para o vazio, vai cair.
    // O Level::resolveCollision ou o chão absoluto voltam a meter isto a true se colidirem.
    body.isGrounded = false; 

    // 3. Paredes Absolutas (Left e Right bounds)
    if (body.position.x < 0.0f) {
        body.position.x = 0.0f;
        body.velocity.x = 0.0f;
    } else if (body.position.x > config::LOGICAL_WIDTH - body.width) {
        body.position.x = config::LOGICAL_WIDTH - body.width;
        body.velocity.x = 0.0f;
    }

    // 4. Chão Absoluto (Base do Mundo)
    if (body.position.y <= 0.0f) {
        body.position.y = 0.0f;
        if (body.velocity.y < 0.0f) {
            body.velocity.y = 0.0f;
        }
        body.isGrounded = true;
    }
}

void PhysicsWorld::jump(PhysicsBody& body, float force) {
    if (body.isGrounded) {
        body.velocity.y = force;
        body.isGrounded = false;
    }
}

} // namespace logic