// =============================================================================
//  Game/Logic/Player.cpp
//
//  @version 6.2c
//  @history
//    v6.1  — Commitment Jump + bloqueio aereo
//    v6.2c — queda de borda zera velocity.x (m_didJump distingue intenção)
// =============================================================================
#include "Logic/Player.h"

namespace logic {

void Player::update(const InputManager& input, PhysicsWorld& world, float dt) {
    applyHorizontalMovement(input);
    updateJumpCharge(input, dt);
    world.step(body, dt);
}

void Player::applyHorizontalMovement(const InputManager& input) {
    if (body.isGrounded) {
        // No chao: controlo directo + reset do flag de salto
        m_didJump = false;
        if (input.isLeft() && !input.isRight()) {
            body.velocity.x = -MOVE_SPEED;
            facingDirection  = -1.0f;
        } else if (input.isRight() && !input.isLeft()) {
            body.velocity.x =  MOVE_SPEED;
            facingDirection  =  1.0f;
        } else {
            body.velocity.x = 0.0f;
        }
    } else {
        // No ar:
        //   m_didJump=true  → Commitment Jump: mantém inércia da parábola
        //   m_didJump=false → caiu da borda sem saltar: cai a pique (vel.x=0)
        if (!m_didJump) {
            body.velocity.x = 0.0f;
        }
    }
}

void Player::updateJumpCharge(const InputManager& input, float dt) {
    const bool spaceHeld = input.isKeyDown(Key::SPACE);
    const bool spaceDown = input.isKeyJustPressed(Key::SPACE);
    const bool spaceUp   = input.isKeyJustReleased(Key::SPACE);

    // Ângulo de 60°: cos=0.5 (componente lateral), sin=0.866 (componente vertical)
    const float COS60 = 0.5f;
    const float SIN60 = 0.866f;

    // Toque rápido (press + release no mesmo frame)
    if (body.isGrounded && spaceDown && spaceUp) {
        body.velocity.y = MIN_JUMP_FORCE * SIN60;
        body.velocity.x = MIN_JUMP_FORCE * COS60 * facingDirection;
        body.isGrounded  = false;
        jumpCharge       = 0.0f;
        isCharging       = false;
        m_didJump        = true;   // ← salto intencional
        return;
    }

    // Acumular carga (manter SPACE premido no chão)
    if (body.isGrounded && spaceHeld) {
        isCharging  = true;
        jumpCharge += dt / CHARGE_TIME;
        if (jumpCharge > 1.0f) jumpCharge = 1.0f;
    }

    // Libertar o salto carregado
    if (spaceUp) {
        if (isCharging && body.isGrounded) {
            float totalForce = MIN_JUMP_FORCE + (MAX_JUMP_FORCE - MIN_JUMP_FORCE) * jumpCharge;
            body.velocity.y  = totalForce * SIN60;
            body.velocity.x  = totalForce * COS60 * facingDirection;
            body.isGrounded  = false;
            m_didJump        = true;   // ← salto intencional
        }
        jumpCharge = 0.0f;
        isCharging = false;
    }
}

} // namespace logic
