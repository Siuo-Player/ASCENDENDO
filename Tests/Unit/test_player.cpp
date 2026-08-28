// =============================================================================
//  Tests/Unit/test_player.cpp
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/Player.h"
#include "Logic/TickInput.h"
#include "Logic/Physics.h"
#include "Logic/InputManager.h"
#include "Core/Config.h"
#include "Core/KeyBindings.h"

using namespace logic;

TEST_SUITE("Player / Movimento e Salto") {

    TEST_CASE("Controlo Horizontal no chao") {
        Player p; PhysicsWorld w;
        p.body.isGrounded = true;

        p.update(TickInput{false, true, false, false, false}, w, PhysicsWorld::FIXED_STEP);
        CHECK(p.velocity().x == doctest::Approx(config::PLAYER_MOVE_SPEED));
        CHECK(p.facingDirection == 1.0f);

        p.update(TickInput{true, false, false, false, false}, w, PhysicsWorld::FIXED_STEP);
        CHECK(p.velocity().x == doctest::Approx(-config::PLAYER_MOVE_SPEED));
        CHECK(p.facingDirection == -1.0f);
    }

    TEST_CASE("Salto Parabolico e Perda de Controlo Aereo") {
        Player p; PhysicsWorld w;
        p.body.isGrounded = true;

        p.update(TickInput{false, true, false, false, false}, w, PhysicsWorld::FIXED_STEP);
        p.update(TickInput{false, false, false, true, true}, w, PhysicsWorld::FIXED_STEP);

        CHECK(p.isGrounded() == false);

        const float expectedY = (config::PLAYER_MIN_JUMP * 0.866f) +
            (PhysicsWorld::GRAVITY * PhysicsWorld::FIXED_STEP);
        const float expectedX = config::PLAYER_MIN_JUMP * 0.5f;

        CHECK(p.velocity().y == doctest::Approx(expectedY));
        CHECK(p.velocity().x == doctest::Approx(expectedX));

        p.update(TickInput{true, false, false, false, false}, w, PhysicsWorld::FIXED_STEP);
        CHECK(p.velocity().x == doctest::Approx(expectedX));
    }

    TEST_CASE("Gameplay respeita rebind de accao") {
        core::KeyBindings bindings;
        bindings.rebind(core::GameAction::MoveRight, Key::C);

        Player p; PhysicsWorld w; InputManager i;
        p.body.isGrounded = true;
        i.onKeyEvent(Key::C, Action::PRESS);

        const TickInput tick = i.tickInput(bindings, 0);
        p.update(tick, w, PhysicsWorld::FIXED_STEP);

        CHECK(p.velocity().x == doctest::Approx(config::PLAYER_MOVE_SPEED));
        CHECK(p.facingDirection == 1.0f);
    }
}

TEST_SUITE("Player / Queda de Borda") {

    TEST_CASE("cair da borda sem saltar: mantem inercia X a 100% (fisica normal)") {
        Player p; PhysicsWorld w;

        p.body.isGrounded = true;
        p.update(TickInput{false, true, false, false, false}, w, PhysicsWorld::FIXED_STEP);
        REQUIRE(p.velocity().x == doctest::Approx(config::PLAYER_MOVE_SPEED));

        p.body.isGrounded = false;
        p.body.position.y = 100.0f;

        p.update(TickInput{false, false, false, false, false}, w, PhysicsWorld::FIXED_STEP);

        CHECK(p.velocity().x == doctest::Approx(config::PLAYER_MOVE_SPEED));
    }
}
