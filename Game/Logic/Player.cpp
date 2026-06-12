// =============================================================================
//  Game/Logic/Player.cpp
//
//  @version 7.2
// =============================================================================

#include "Logic/Player.h"
#include "Core/Config.h"

namespace logic {

void Player::update(const InputManager& input, PhysicsWorld& world, float dt) {
    applyHorizontalMovement(input);
    updateJumpCharge(input, dt);
    world.step(body, dt);
}

void Player::applyHorizontalMovement(const InputManager& input) {
    if (body.isGrounded) {
        m_didJump = false;
        if (input.isLeft() && !input.isRight()) {
            body.velocity.x = -config::PLAYER_MOVE_SPEED;
            facingDirection  = -1.0f;
        } else if (input.isRight() && !input.isLeft()) {
            body.velocity.x =  config::PLAYER_MOVE_SPEED;
            facingDirection  =  1.0f;
        } else {
            body.velocity.x = 0.0f;
        }
    } 
}

void Player::updateJumpCharge(const InputManager& input, float dt) {
    const bool spaceHeld = input.isKeyDown(Key::SPACE);
    const bool spaceDown = input.isKeyJustPressed(Key::SPACE);
    const bool spaceUp   = input.isKeyJustReleased(Key::SPACE);

    const float COS60 = 0.5f;
    const float SIN60 = 0.866f;

    if (body.isGrounded && spaceDown && spaceUp) {
        body.velocity.y = config::PLAYER_MIN_JUMP * SIN60;
        body.velocity.x = config::PLAYER_MIN_JUMP * COS60 * facingDirection;
        body.isGrounded  = false;
        jumpCharge       = 0.0f;
        isCharging       = false;
        m_didJump        = true;
        return;
    }

    if (body.isGrounded && spaceHeld) {
        isCharging  = true;
        jumpCharge += dt / config::PLAYER_CHARGE_TIME;
        if (jumpCharge > 1.0f) jumpCharge = 1.0f;
    }

    if (spaceUp) {
        if (isCharging && body.isGrounded) {
            float totalForce = config::PLAYER_MIN_JUMP + (config::PLAYER_MAX_JUMP - config::PLAYER_MIN_JUMP) * jumpCharge;
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