#include "doctest/doctest.h"
#include "Logic/SimulationOrchestrator.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Logic/Level.h"
#include "Logic/InputManager.h"

using namespace logic;

TEST_SUITE("SimulationOrchestrator") {
    TEST_CASE("zero delta produces no simulation steps") {
        SimulationOrchestrator simulation;
        PhysicsWorld world;
        Player player;
        Level level;
        InputManager input;

        CHECK(simulation.advance(0.0f, input, player, world, level) == 0);
        CHECK(world.accumulator() == doctest::Approx(0.0f));
    }

    TEST_CASE("fixed-step advancement is owned by physics world") {
        SimulationOrchestrator simulation;
        PhysicsWorld world;
        Player player;
        Level level;
        InputManager input;

        player.body.position.y = 10.0f;
        const float beforeY = player.position().y;
        const int steps = simulation.advance(PhysicsWorld::FIXED_STEP, input, player, world, level);

        CHECK(steps == 1);
        CHECK(player.position().y < beforeY);
    }

    TEST_CASE("large delta respects physics step cap") {
        SimulationOrchestrator simulation;
        PhysicsWorld world;
        Player player;
        Level level;
        InputManager input;

        const int steps = simulation.advance(1.0f, input, player, world, level);
        CHECK(steps == PhysicsWorld::MAX_STEPS_PER_ADVANCE);
    }
}
