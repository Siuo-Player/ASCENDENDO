#include "doctest/doctest.h"
#include "Logic/SimulationOrchestrator.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Logic/Level.h"
#include "Logic/InputManager.h"
#include "Core/KeyBindings.h"

using namespace logic;

TEST_SUITE("SimulationOrchestrator") {
    TEST_CASE("zero delta produces no simulation steps") {
        SimulationOrchestrator simulation;
        PhysicsWorld world;
        Player player;
        Level level;
        InputManager input;
        core::KeyBindings bindings;

        CHECK(simulation.advance(0.0f, input, bindings, player, world, level) == 0);
        CHECK(world.accumulator() == doctest::Approx(0.0f));
    }

    TEST_CASE("fixed-step advancement is owned by physics world") {
        SimulationOrchestrator simulation;
        PhysicsWorld world;
        Player player;
        Level level;
        InputManager input;
        core::KeyBindings bindings;

        player.body.position.y = 10.0f;
        const float beforeY = player.position().y;
        const int steps = simulation.advance(PhysicsWorld::FIXED_STEP, input, bindings,
                                             player, world, level);

        CHECK(steps == 1);
        CHECK(player.position().y < beforeY);
    }

    TEST_CASE("large delta respects physics step cap") {
        SimulationOrchestrator simulation;
        PhysicsWorld world;
        Player player;
        Level level;
        InputManager input;
        core::KeyBindings bindings;

        const int steps = simulation.advance(1.0f, input, bindings, player, world, level);
        CHECK(steps == PhysicsWorld::MAX_STEPS_PER_ADVANCE);
    }

    TEST_CASE("same continuous tick input is invariant to frame repartition") {
        SimulationOrchestrator simulation;
        core::KeyBindings bindingsA;
        core::KeyBindings bindingsB;
        PhysicsWorld worldA;
        PhysicsWorld worldB;
        Player playerA;
        Player playerB;
        Level levelA;
        Level levelB;
        InputManager inputA;
        InputManager inputB;

        inputA.onKeyEvent(Key::D, Action::PRESS);
        inputB.onKeyEvent(Key::D, Action::PRESS);

        for (int i = 0; i < 4; ++i) {
            simulation.advance(PhysicsWorld::FIXED_STEP, inputA, bindingsA,
                               playerA, worldA, levelA);
        }

        simulation.advance(2.0f * PhysicsWorld::FIXED_STEP, inputB, bindingsB,
                           playerB, worldB, levelB);
        simulation.advance(2.0f * PhysicsWorld::FIXED_STEP, inputB, bindingsB,
                           playerB, worldB, levelB);

        CHECK(playerA.position().x == doctest::Approx(playerB.position().x));
        CHECK(playerA.position().y == doctest::Approx(playerB.position().y));
        CHECK(playerA.velocity().x == doctest::Approx(playerB.velocity().x));
        CHECK(playerA.velocity().y == doctest::Approx(playerB.velocity().y));
        CHECK(playerA.isGrounded() == playerB.isGrounded());
    }
}
