#include "doctest/doctest.h"
#include "Logic/GameSession.h"
#include "Core/Config.h"
#include "Core/GameState.h"

#include <cmath>
#include <filesystem>
#include <limits>

TEST_SUITE("GameSession") {
    TEST_CASE("starts in MENU with the first menu item selected") {
        logic::GameSession session({}, "campaign-id", "runs.csv");

        CHECK(session.state() == core::GameState::MENU);
        CHECK(session.menuSelection() == 0);
        CHECK(session.elapsedTime() == doctest::Approx(0.0f));
    }

    TEST_CASE("enters PLAYING with a valid campaign and reset gameplay state") {
        logic::GameSession session(
            {std::filesystem::path("Game/Assets/Levels/inicio.lvl")},
            "campaign-id",
            "runs.csv");

        session.beginPlaying(static_cast<float>(config::LOGICAL_WIDTH));

        CHECK(session.state() == core::GameState::PLAYING);
        CHECK(session.elapsedTime() == doctest::Approx(0.0f));
        CHECK(session.player().position().x == doctest::Approx(320.0f));
        CHECK(session.player().position().y == doctest::Approx(40.0f));
        CHECK(session.level().platformCount() > 0);
    }

    TEST_CASE("does not enter PLAYING when the initial level cannot load") {
        logic::GameSession session(
            {std::filesystem::path("Game/Assets/Levels/does-not-exist.lvl")},
            "campaign-id",
            "runs.csv");

        session.beginPlaying(static_cast<float>(config::LOGICAL_WIDTH));

        CHECK(session.state() == core::GameState::MENU);
        CHECK(session.level().platformCount() == 0);
        CHECK(session.elapsedTime() == doctest::Approx(0.0f));
    }

    TEST_CASE("failed specific level load preserves active session state") {
        logic::GameSession session(
            {
                std::filesystem::path("Game/Assets/Levels/inicio.lvl"),
                std::filesystem::path("Game/Assets/Levels/zigzag.lvl")
            },
            "campaign-id",
            "runs.csv");

        REQUIRE(session.beginPlayingLevel(0, static_cast<float>(config::LOGICAL_WIDTH)));
        REQUIRE(session.state() == core::GameState::PLAYING);

        const int platformCount = session.level().platformCount();
        const auto playerPosition = session.player().position();
        const float elapsed = session.elapsedTime();

        CHECK_FALSE(session.beginPlayingLevel(99, static_cast<float>(config::LOGICAL_WIDTH)));
        CHECK(session.state() == core::GameState::PLAYING);
        CHECK(session.level().platformCount() == platformCount);
        CHECK(session.player().position() == playerPosition);
        CHECK(session.elapsedTime() == doctest::Approx(elapsed));
    }

    TEST_CASE("invalid frame delta does not contaminate elapsed time or simulation") {
        logic::GameSession session(
            {std::filesystem::path("Game/Assets/Levels/inicio.lvl")},
            "campaign-id",
            "runs.csv");
        session.beginPlaying(static_cast<float>(config::LOGICAL_WIDTH));
        REQUIRE(session.state() == core::GameState::PLAYING);

        const auto positionBefore = session.player().position();
        const auto velocityBefore = session.player().velocity();
        const float elapsedBefore = session.elapsedTime();
        logic::InputManager input;
        core::KeyBindings bindings;
        const float nan = std::numeric_limits<float>::quiet_NaN();

        session.update(nan, input, bindings, 640, 360, 640.0f, 360.0f);

        CHECK(session.elapsedTime() == doctest::Approx(elapsedBefore));
        CHECK(std::isfinite(session.elapsedTime()));
        CHECK(session.player().position() == positionBefore);
        CHECK(session.player().velocity() == velocityBefore);
    }

    TEST_CASE("preserves explicit editor return state") {
        logic::GameSession session({}, "campaign-id", "runs.csv");

        session.openEditor(core::GameState::PLAYING);

        CHECK(session.state() == core::GameState::EDITOR);

        logic::InputManager input;
        core::KeyBindings bindings;
        const auto result = session.update(
            0.0f, input, bindings, 640, 360, 640.0f, 360.0f);

        CHECK(result.stateChanged == false);
        CHECK(session.state() == core::GameState::EDITOR);
    }
}
