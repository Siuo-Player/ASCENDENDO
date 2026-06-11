#pragma once
// =============================================================================
//  Game/Logic/Player.h
//
//  @version 6.2c
//  @history
//    v6.1  — Commitment Jump + bloqueio aereo implementados
//    v6.2c — m_didJump: distingue salto intencional de queda de borda
//            (queda de borda → sem inércia X; salto → mantém parábola)
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
    float       facingDirection = 1.0f;

    void  update(const InputManager& input, PhysicsWorld& world, float dt);

    Vec2  position()    const { return body.position;   }
    Vec2  velocity()    const { return body.velocity;   }
    bool  isGrounded()  const { return body.isGrounded; }
    float chargeRatio() const { return jumpCharge;      }

private:
    // true apenas quando o jogador executou um salto intencional (SPACE).
    // Permite Commitment Jump (inércia X no ar).
    // Reset quando pousa (isGrounded=true).
    bool m_didJump = false;

    void applyHorizontalMovement(const InputManager& input);
    void updateJumpCharge(const InputManager& input, float dt);
};

} // namespace logic
