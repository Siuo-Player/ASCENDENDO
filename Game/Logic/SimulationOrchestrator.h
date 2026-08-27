#pragma once
// =============================================================================
//  Game/Logic/SimulationOrchestrator.h
//
//  Owns the gameplay fixed-step orchestration used by the application loop.
// =============================================================================

namespace core { class KeyBindings; }

namespace logic {

class InputManager;
class Level;
class PhysicsWorld;
class Player;

class SimulationOrchestrator {
public:
    // Advances the physics accumulator and applies exactly the fixed number of
    // simulation steps reported by PhysicsWorld::advance(). Each step receives
    // an explicit semantic TickInput derived from the frame sample.
    int advance(float dt, const InputManager& input, const core::KeyBindings& bindings,
                Player& player, PhysicsWorld& world, Level& level) const;
};

} // namespace logic
