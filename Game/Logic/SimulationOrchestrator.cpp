#include "Logic/SimulationOrchestrator.h"

#include "Logic/InputManager.h"
#include "Logic/Level.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Core/Config.h"

namespace logic {

int SimulationOrchestrator::advance(float dt, const InputManager& input,
                                     Player& player, PhysicsWorld& world,
                                     Level& level) const {
    const int steps = world.advance(dt);
    for (int i = 0; i < steps; ++i) {
        player.update(input, world, config::FIXED_STEP);
        level.resolveCollision(player.body);
    }
    return steps;
}

} // namespace logic
