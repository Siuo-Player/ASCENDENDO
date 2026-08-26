#pragma once
// =============================================================================
//  Game/Logic/SimulationOrchestrator.h
//
//  Owns the gameplay fixed-step orchestration used by the application loop.
// =============================================================================

namespace logic {

class InputManager;
class Level;
class PhysicsWorld;
class Player;

class SimulationOrchestrator {
public:
    // Advances the physics accumulator and applies exactly the fixed number of
    // simulation steps reported by PhysicsWorld::advance().
    int advance(float dt, const InputManager& input, Player& player,
                PhysicsWorld& world, Level& level) const;
};

} // namespace logic
