#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/GameSession.h"
#include "../../Game/Core/Config.h"

#include <filesystem>

TEST_SUITE("GameSession capture mode") {
    TEST_CASE("non-final campaign level can be loaded in isolation without campaign goal") {
        logic::GameSession session(
            {
                "Game/Assets/Levels/inicio.lvl",
                "Game/Assets/Levels/zigzag.lvl",
                "Game/Assets/Levels/precipicio.lvl"
            },
            "capture-test",
            (std::filesystem::temp_directory_path() / "ascendendo-capture-runs.csv").string());

        REQUIRE(session.beginPlayingLevelForCapture(2, config::LOGICAL_WIDTH));
        CHECK(session.state() == core::GameState::PLAYING);
        CHECK_FALSE(session.level().hasFlag);
        CHECK(session.elapsedTime() == doctest::Approx(0.0f));
    }

    TEST_CASE("capture mode rejects an out-of-range level") {
        logic::GameSession session(
            {"Game/Assets/Levels/inicio.lvl"},
            "capture-test",
            (std::filesystem::temp_directory_path() / "ascendendo-capture-runs.csv").string());

        CHECK_FALSE(session.beginPlayingLevelForCapture(1, config::LOGICAL_WIDTH));
        CHECK(session.state() == core::GameState::MENU);
    }
}
