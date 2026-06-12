// =============================================================================
//  Game/Logic/Player.cpp
//
//  @version 6.7
//  @history
//    v6.1  — Commitment Jump + bloqueio aereo
//    v6.2c — queda de borda zera velocity.x (m_didJump distingue intenção)
//    v6.7  — remoção de game-feel hacks: inércia horizontal total no ar (física normal)
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
        // No chao: controlo direto sobre a velocidade
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
    } 
    // No ar: Não aplicamos nenhuma força ou corte.
    // Pela Lei da Inércia, a velocidade horizontal atual (salto ou queda) mantém-se constante.
}

void Player::updateJumpCharge(const InputManager& input, float dt) {
    const bool spaceHeld = input.isKeyDown(Key::SPACE);
    const bool spaceDown = input.isKeyJustPressed(Key::SPACE);
    const bool spaceUp   = input.isKeyJustReleased(Key::SPACE);

    const float COS60 = 0.5f;
    const float SIN60 = 0.866f;

    if (body.isGrounded && spaceDown && spaceUp) {
        body.velocity.y = MIN_JUMP_FORCE * SIN60;
        body.velocity.x = MIN_JUMP_FORCE * COS60 * facingDirection;
        body.isGrounded  = false;
        jumpCharge       = 0.0f;
        isCharging       = false;
        m_didJump        = true;
        return;
    }

    if (body.isGrounded && spaceHeld) {
        isCharging  = true;
        jumpCharge += dt / CHARGE_TIME;
        if (jumpCharge > 1.0f) jumpCharge = 1.0f;
    }

    if (spaceUp) {
        if (isCharging && body.isGrounded) {
            float totalForce = MIN_JUMP_FORCE + (MAX_JUMP_FORCE - MIN_JUMP_FORCE) * jumpCharge;
            body.velocity.y  = totalForce * SIN60;
            body.velocity.x  = totalForce * COS60 * facingDirection;
            body.isGrounded  = false;
            m_didJump        = true;
        }
        jumpCharge = 0.0f;
        isCharging = false;
    }
}

} // namespace logic