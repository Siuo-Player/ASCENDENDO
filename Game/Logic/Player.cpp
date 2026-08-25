// =============================================================================
//  Game/Logic/Player.cpp
// =============================================================================

#include "Logic/Player.h"
#include "Core/Config.h"
#include "Core/KeyBindings.h"

namespace logic {

namespace {
const core::KeyBindings& defaultBindings() {
    static const core::KeyBindings bindings;
    return bindings;
}

const core::KeyBindings& activeBindings() {
    if (const core::KeyBindings* bindings = core::KeyBindings::active()) {
        return *bindings;
    }
    return defaultBindings();
}
}

void Player::update(const InputManager& input, PhysicsWorld& world, float dt) {
    applyHorizontalMovement(input);
    updateJumpCharge(input, dt);
    world.step(body, dt);
}

void Player::applyHorizontalMovement(const InputManager& input) {
    const auto& bindings = activeBindings();

    if (body.isGrounded) {
        m_didJump = false;
        if (core::isActionHeld(bindings, input, core::GameAction::MoveLeft) &&
            !core::isActionHeld(bindings, input, core::GameAction::MoveRight)) {
            body.velocity.x = -config::PLAYER_MOVE_SPEED;
            facingDirection  = -1.0f;
        } else if (core::isActionHeld(bindings, input, core::GameAction::MoveRight) &&
                   !core::isActionHeld(bindings, input, core::GameAction::MoveLeft)) {
            body.velocity.x = config::PLAYER_MOVE_SPEED;
            facingDirection  = 1.0f;
        } else {
            body.velocity.x = 0.0f;
        }
    }
}

void Player::updateJumpCharge(const InputManager& input, float dt) {
    const auto& bindings = activeBindings();
    const bool jumpHeld = core::isActionHeld(bindings, input, core::GameAction::Jump);
    const bool jumpDown = core::isActionJustPressed(bindings, input, core::GameAction::Jump);
    const bool jumpUp   = core::isActionJustReleased(bindings, input, core::GameAction::Jump);

    const float COS60 = 0.5f;
    const float SIN60 = 0.866f;

    if (body.isGrounded && jumpDown && jumpUp) {
        body.velocity.y = config::PLAYER_MIN_JUMP * SIN60;
        body.velocity.x = config::PLAYER_MIN_JUMP * COS60 * facingDirection;
        body.isGrounded = false;
        jumpCharge = 0.0f;
        isCharging = false;
        m_didJump = true;
        return;
    }

    if (body.isGrounded && jumpHeld) {
        isCharging = true;
        jumpCharge += dt / config::PLAYER_CHARGE_TIME;
        if (jumpCharge > 1.0f) jumpCharge = 1.0f;
    }

    if (jumpUp) {
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
