// =============================================================================
//  Game/Logic/Player.cpp
//
//  @version 3.2
//  @history
//    v3.2 — criado
// =============================================================================

#include "Logic/Player.h"

namespace logic {

void Player::update(const InputManager& input, PhysicsWorld& world, float dt) {
    applyHorizontalMovement(input);
    updateJumpCharge(input, dt);
    world.step(body, dt);
}

// ── Movimento Horizontal ──────────────────────────────────────────────────────

void Player::applyHorizontalMovement(const InputManager& input) {
    if (input.isLeft() && !input.isRight()) {
        body.velocity.x = -MOVE_SPEED;
    } else if (input.isRight() && !input.isLeft()) {
        body.velocity.x =  MOVE_SPEED;
    } else {
        body.velocity.x = 0.0f;
    }
}

// ── Salto por Carga ───────────────────────────────────────────────────────────
//
//  Ordem de prioridade por frame:
//   1. Toque rápido (PRESS + RELEASE mesmo frame) → força mínima
//   2. SPACE mantido no chão                      → acumular carga
//   3. SPACE solto com carga activa               → saltar com força proporcional

void Player::updateJumpCharge(const InputManager& input, float dt) {
    const bool spaceHeld  = input.isKeyDown(Key::SPACE);
    const bool spaceDown  = input.isKeyJustPressed(Key::SPACE);
    const bool spaceUp    = input.isKeyJustReleased(Key::SPACE);

    // 1. Toque rápido: press + release no mesmo frame
    if (body.isGrounded && spaceDown && spaceUp) {
        body.velocity.y = MIN_JUMP_FORCE;
        body.isGrounded = false;
        jumpCharge      = 0.0f;
        isCharging      = false;
        return;
    }

    // 2. Acumular enquanto no chão com SPACE held
    if (body.isGrounded && spaceHeld) {
        isCharging  = true;
        jumpCharge += dt / CHARGE_TIME;
        if (jumpCharge > 1.0f) jumpCharge = 1.0f;
    }

    // 3. Ao soltar SPACE
    if (spaceUp) {
        if (isCharging && body.isGrounded) {
            float force     = MIN_JUMP_FORCE + (MAX_JUMP_FORCE - MIN_JUMP_FORCE) * jumpCharge;
            body.velocity.y = force;
            body.isGrounded = false;
        }
        jumpCharge = 0.0f;
        isCharging = false;
    }
}

} // namespace logic
