#include "doctest/doctest.h"
#include "Logic/GameSession.h"
#include "Core/Config.h"
#include "Core/GameState.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <limits>

namespace {

std::filesystem::path writeTestLevel(const std::string& name, const std::string& body) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream out(path);
    REQUIRE(out.is_open());
    out << body;
    REQUIRE(out.good());
    return path;
}

void removeTestFile(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

} // namespace

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

    TEST_CASE("uses authored spawn for a real level run") {
        const auto path = writeTestLevel(
            "ascendendo-authored-spawn-runtime.lvl",
            "NAME Authored Spawn\n"
            "SCREENS 1\n"
            "SPAWN 123.25 57.50\n"
            "PLATFORM 80 40 160 20\n");

        logic::GameSession session({path}, "campaign-id", "runs.csv");
        session.beginPlaying(static_cast<float>(config::LOGICAL_WIDTH));

        CHECK(session.state() == core::GameState::PLAYING);
        CHECK(session.player().position().x == doctest::Approx(123.25f));
        CHECK(session.player().position().y == doctest::Approx(57.50f));
        CHECK(session.level().spawnPosition.x == doctest::Approx(123.25f));
        CHECK(session.level().spawnPosition.y == doctest::Approx(57.50f));

        removeTestFile(path);
    }

    TEST_CASE("legacy level without SPAWN keeps the documented fallback") {
        const auto path = writeTestLevel(
            "ascendendo-legacy-spawn-runtime.lvl",
            "NAME Legacy Spawn\n"
            "PLATFORM 80 40 160 20\n");

        logic::GameSession session({path}, "campaign-id", "runs.csv");
        session.beginPlaying(static_cast<float>(config::LOGICAL_WIDTH));

        CHECK(session.state() == core::GameState::PLAYING);
        CHECK(session.player().position().x == doctest::Approx(config::LOGICAL_WIDTH / 2.0f));
        CHECK(session.player().position().y == doctest::Approx(40.0f));

        removeTestFile(path);
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