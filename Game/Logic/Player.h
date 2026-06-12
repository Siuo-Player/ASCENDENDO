#pragma once
// =============================================================================
//  Game/Logic/Player.h
//
//  @version 7.1
// =============================================================================

#include "Logic/Physics.h"
#include "Logic/InputManager.h"
#include "Core/Config.h" // Importa as constantes globais

namespace logic {

class Player {
public:
    PhysicsBody body;
    float       jumpCharge      = 0.0f;
    bool        isCharging      = false;
    float       facingDirection = 1.0f;

    Player() {
        // Redimensionamento consumido diretamente das configurações globais
        body.width = config::PLAYER_WIDTH;
        body.height = config::PLAYER_HEIGHT;
    }

    void  update(const InputManager& input, PhysicsWorld& world, float dt);

    Vec2  position()    const { return body.position;   }
    Vec2  velocity()    const { return body.velocity;   }
    bool  isGrounded()  const { return body.isGrounded; }
    float chargeRatio() const { return jumpCharge;      }

private:
    bool m_didJump = false;

    void applyHorizontalMovement(const InputManager& input);
    void updateJumpCharge(const InputManager& input, float dt);
};

} // namespace logic