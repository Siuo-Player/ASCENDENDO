#pragma once
// =============================================================================
//  Game/Logic/Player.h
// =============================================================================

#include "Logic/Physics.h"
#include "Logic/TickInput.h"
#include "Core/Config.h"

namespace logic {

class Player {
public:
    PhysicsBody body;
    float       jumpCharge      = 0.0f;
    bool        isCharging      = false;
    float       facingDirection = 1.0f;

    Player() {
        body.width = config::PLAYER_WIDTH;
        body.height = config::PLAYER_HEIGHT;
    }

    void update(const TickInput& input, PhysicsWorld& world, float dt);

    Vec2  position()    const { return body.position; }
    Vec2  velocity()    const { return body.velocity; }
    bool  isGrounded()  const { return body.isGrounded; }
    float chargeRatio() const { return jumpCharge; }

private:
    bool m_didJump = false;

    void applyHorizontalMovement(const TickInput& input);
    void updateJumpCharge(const TickInput& input, float dt);
};

} // namespace logic
