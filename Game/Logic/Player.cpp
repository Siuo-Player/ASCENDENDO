// =============================================================================
//  Game/Logic/Player.cpp
//
//  @version 6.1
// =============================================================================
#include "Logic/Player.h"

namespace logic {

void Player::update(const InputManager& input, PhysicsWorld& world, float dt) {
    applyHorizontalMovement(input);
    updateJumpCharge(input, dt);
    world.step(body, dt);
}

void Player::applyHorizontalMovement(const InputManager& input) {
    // Só podes controlar a tua velocidade X se estiveres no chão!
    if (body.isGrounded) {
        if (input.isLeft() && !input.isRight()) {
            body.velocity.x = -MOVE_SPEED;
            facingDirection = -1.0f;
        } else if (input.isRight() && !input.isLeft()) {
            body.velocity.x =  MOVE_SPEED;
            facingDirection =  1.0f;
        } else {
            body.velocity.x = 0.0f; // Fricção instantânea ao largar teclas
        }
    }
    // No ar, o X não é alterado (mantendo a inércia geométrica da parábola).
}

void Player::updateJumpCharge(const InputManager& input, float dt) {
    const bool spaceHeld  = input.isKeyDown(Key::SPACE);
    const bool spaceDown  = input.isKeyJustPressed(Key::SPACE);
    const bool spaceUp    = input.isKeyJustReleased(Key::SPACE);

    // Constantes matemáticas de um ângulo de salto de ~60 graus
    // cos(60) = 0.5 (força lateral)  |  sin(60) = 0.866 (força vertical)
    const float JUMP_ANGLE_COS = 0.5f;
    const float JUMP_ANGLE_SIN = 0.866f;

    if (body.isGrounded && spaceDown && spaceUp) { // Toque rápido
        body.velocity.y = MIN_JUMP_FORCE * JUMP_ANGLE_SIN;
        body.velocity.x = MIN_JUMP_FORCE * JUMP_ANGLE_COS * facingDirection;
        body.isGrounded = false;
        jumpCharge      = 0.0f;
        isCharging      = false;
        return;
    }

    if (body.isGrounded && spaceHeld) {
        isCharging  = true;
        jumpCharge += dt / CHARGE_TIME;
        if (jumpCharge > 1.0f) jumpCharge = 1.0f;
    }

    if (spaceUp) {
        if (isCharging && body.isGrounded) { // Libertar o salto pesado
            float totalForce = MIN_JUMP_FORCE + (MAX_JUMP_FORCE - MIN_JUMP_FORCE) * jumpCharge;
            
            // Decompor a força do salto na parábola
            body.velocity.y = totalForce * JUMP_ANGLE_SIN;
            body.velocity.x = totalForce * JUMP_ANGLE_COS * facingDirection;
            
            body.isGrounded = false;
        }
        jumpCharge = 0.0f;
        isCharging = false;
    }
}

} // namespace logic