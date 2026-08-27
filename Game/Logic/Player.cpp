// =============================================================================
//  Game/Logic/Player.cpp
// =============================================================================

#include "Logic/Player.h"
#include "Core/Config.h"

namespace logic {

void Player::update(const TickInput& input, PhysicsWorld& world, float dt) {
    applyHorizontalMovement(input);
    updateJumpCharge(input, dt);
    world.step(body, dt);
}

void Player::applyHorizontalMovement(const TickInput& input) {
    if (body.isGrounded) {
        m_didJump = false;
        if (input.left && !input.right) {
            body.velocity.x = -config::PLAYER_MOVE_SPEED;
            facingDirection = -1.0f;
        } else if (input.right && !input.left) {
            body.velocity.x = config::PLAYER_MOVE_SPEED;
            facingDirection = 1.0f;
        } else {
            body.velocity.x = 0.0f;
        }
    }
}

void Player::updateJumpCharge(const TickInput& input, float dt) {
    const float COS60 = 0.5f;
    const float SIN60 = 0.866f;

    if (body.isGrounded && input.jumpPressed && input.jumpReleased) {
        body.velocity.y = config::PLAYER_MIN_JUMP * SIN60;
        body.velocity.x = config::PLAYER_MIN_JUMP * COS60 * facingDirection;
        body.isGrounded = false;
        jumpCharge = 0.0f;
        isCharging = false;
        m_didJump = true;
        return;
    }

    if (body.isGrounded && input.jumpHeld) {
        isCharging = true;
        jumpCharge += dt / config::PLAYER_CHARGE_TIME;
        if (jumpCharge > 1.0f) jumpCharge = 1.0f;
    }

    if (input.jumpReleased) {
        if (isCharging && body.isGrounded) {
            const float totalForce = config::PLAYER_MIN_JUMP +
                (config::PLAYER_MAX_JUMP - config::PLAYER_MIN_JUMP) * jumpCharge;
            body.velocity.y = totalForce * SIN60;
            body.velocity.x = totalForce * COS60 * facingDirection;
            body.isGrounded = false;
            m_didJump = true;
        }
        jumpCharge = 0.0f;
        isCharging = false;
    }
}

} // namespace logic
