#include "doctest/doctest.h"
#include "Logic/GameSession.h"

TEST_SUITE("GameSession") {
    TEST_CASE("starts in MENU with the first menu item selected") {
        logic::GameSession session({}, "campaign-id", "runs.csv");

        CHECK(session.state() == gfx::GameState::MENU);
        CHECK(session.menuSelection() == 0);
        CHECK(session.elapsedTime() == doctest::Approx(0.0f));
    }

    TEST_CASE("enters PLAYING with reset gameplay state") {
        logic::GameSession session({}, "campaign-id", "runs.csv");

        session.beginPlaying(640.0f);

        CHECK(session.state() == gfx::GameState::PLAYING);
        CHECK(session.elapsedTime() == doctest::Approx(0.0f));
        CHECK(session.player().position().x == doctest::Approx(320.0f));
        CHECK(session.player().position().y == doctest::Approx(40.0f));
    }

    TEST_CASE("preserves explicit editor return state") {
        logic::GameSession session({}, "campaign-id", "runs.csv");

        session.openEditor(gfx::GameState::PLAYING);

        CHECK(session.state() == gfx::GameState::EDITOR);

        logic::InputManager input;
        core::KeyBindings bindings;
        const auto result = session.update(
            0.0f, input, bindings, 640, 360, 640.0f, 360.0f);

        CHECK(result.stateChanged == false);
        CHECK(session.state() == gfx::GameState::EDITOR);
    }
}
