#include "Logic/SimulationOrchestrator.h"

#include "Logic/InputManager.h"
#include "Logic/Level.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Core/Config.h"

namespace logic {

int SimulationOrchestrator::advance(float dt, const InputManager& input,
                                     const core::KeyBindings& bindings,
                                     Player& player, PhysicsWorld& world,
                                     Level& level) const {
    const int steps = world.advance(dt);
    for (int i = 0; i < steps; ++i) {
        const TickInput tickInput = input.tickInput(bindings, static_cast<std::size_t>(i));
        player.update(tickInput, world, config::FIXED_STEP);
        level.resolveCollision(player.body);
    }
    return steps;
}

} // namespace logic
