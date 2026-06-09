#pragma once
// =============================================================================
//  Game/Logic/Player.h
//
//  @version 6.1
//  @history
//    v6.1 — Salto Parabólico e Perda de Controlo Aéreo implementados.
// =============================================================================

#include "Logic/Physics.h"
#include "Logic/InputManager.h"

namespace logic {

class Player {
public:
    static constexpr float MOVE_SPEED     = 200.0f;
    static constexpr float MIN_JUMP_FORCE = 350.0f;
    static constexpr float MAX_JUMP_FORCE = 800.0f;
    static constexpr float CHARGE_TIME    = 0.5f;

    PhysicsBody body;
    float       jumpCharge      = 0.0f;
    bool        isCharging      = false;
    float       facingDirection = 1.0f; // 1.0f = Direita, -1.0f = Esquerda

    void update(const InputManager& input, PhysicsWorld& world, float dt);

    Vec2  position()    const { return body.position;    }
    Vec2  velocity()    const { return body.velocity;    }
    bool  isGrounded()  const { return body.isGrounded;  }
    float chargeRatio() const { return jumpCharge;       }

private:
    void applyHorizontalMovement(const InputManager& input);
    void updateJumpCharge(const InputManager& input, float dt);
};

} // namespace logic