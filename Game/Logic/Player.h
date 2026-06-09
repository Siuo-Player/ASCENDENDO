#pragma once
// =============================================================================
//  Game/Logic/Player.h
//
//  @version 3.2
//  @history
//    v3.2 — criado (movimento horizontal, salto por carga, integração Input+Physics)
//
//  Mecânica de salto por carga (visual na Fase 6 — barra de UI):
//    SPACE held  → jumpCharge sobe de 0.0 a 1.0 ao longo de CHARGE_TIME
//    SPACE release → salta com força = MIN + (MAX - MIN) * jumpCharge
//
//  Uso no Fixed Timestep:
//    int steps = world.advance(deltaTime);
//    for (int i = 0; i < steps; ++i)
//        player.update(input, world, PhysicsWorld::FIXED_STEP);
// =============================================================================

#include "Logic/Physics.h"
#include "Logic/InputManager.h"

namespace logic {

class Player {
public:
    static constexpr float MOVE_SPEED     = 200.0f;
    static constexpr float MIN_JUMP_FORCE = 300.0f;
    static constexpr float MAX_JUMP_FORCE = 750.0f;
    static constexpr float CHARGE_TIME    = 0.5f;

    PhysicsBody body;
    float       jumpCharge = 0.0f;  // 0.0–1.0 (para a barra de UI, Fase 6)
    bool        isCharging = false;

    void update(const InputManager& input, PhysicsWorld& world, float dt);

    Vec2  position()    const { return body.position;    }
    Vec2  velocity()    const { return body.velocity;    }
    bool  isGrounded()  const { return body.isGrounded;  }
    float chargeRatio() const { return jumpCharge;        }

private:
    void applyHorizontalMovement(const InputManager& input);
    void updateJumpCharge(const InputManager& input, float dt);
};

} // namespace logic
