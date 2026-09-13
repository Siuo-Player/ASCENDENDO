#include "../../external/doctest/doctest.h"
#include "../../Game/Core/Config.h"
#include "../../Game/Core/KeyBindings.h"
#include "../../Game/Logic/GameSession.h"
#include "../../Game/Logic/InputManager.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace logic;

namespace {

std::filesystem::path writeLevel(const std::string& name, const std::string& body) {
    const auto path = std::filesystem::temp_directory_path() / name;
    std::ofstream out(path);
    REQUIRE(out.is_open());
    out << body;
    REQUIRE(out.good());
    return path;
}

void removeFile(const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

struct RunDriver {
    GameSession& session;
    InputManager& input;
    const core::KeyBindings& bindings;

    GameSessionUpdateResult frame(bool jumpHeld = false,
                                  bool jumpPressed = false,
                                  bool jumpReleased = false) {
        input.injectRawState(false, false, jumpHeld, jumpPressed, jumpReleased);
        return session.update(
            config::FIXED_STEP,
            input,
            bindings,
            static_cast<int>(config::LOGICAL_WIDTH),
            static_cast<int>(config::LOGICAL_HEIGHT),
            config::LOGICAL_WIDTH,
            config::LOGICAL_HEIGHT);
    }

    bool chargedJump() {
        // 24 fixed ticks at 60 Hz are exactly the configured 0.4 s charge time.
        for (int tick = 0; tick < 24; ++tick) {
            (void)frame(true, tick == 0, false);
        }
        auto result = frame(false, false, true);
        if (result.campaignCompleted) return true;

        for (int tick = 0; tick < 120; ++tick) {
            result = frame();
            if (result.campaignCompleted) return true;
            if (session.player().isGrounded()) return true;
        }
        return false;
    }
};

} // namespace

TEST_SUITE("StableGameplayLoop") {
TEST_CASE("complete run uses real charge physics, streams vertically, and reaches the derived flag") {
    const auto firstLevel = writeLevel(
        "ascendendo-acceptance-first.lvl",
        "NAME Acceptance First\n"
        "SCREENS 1\n"
        "PLATFORM 0 124 640 16\n"
        "PLATFORM 0 229 640 16\n");
    const auto finalLevel = writeLevel(
        "ascendendo-acceptance-final.lvl",
        "NAME Acceptance Final\n"
        "SCREENS 1\n"
        "PLATFORM 0 16 640 6\n");
    const auto runsPath = std::filesystem::temp_directory_path() /
        "ascendendo-acceptance-stable-gameplay.csv";
    removeFile(runsPath);

    GameSession session(
        std::vector<std::filesystem::path>{firstLevel, finalLevel},
        "stable-gameplay-acceptance",
        runsPath.string());
    session.beginPlaying(config::LOGICAL_WIDTH);

    REQUIRE(session.state() == core::GameState::PLAYING);
    CHECK(session.player().position().x == doctest::Approx(320.0f));
    CHECK(session.player().position().y == doctest::Approx(16.0f));
    CHECK(session.level().platformCount() == 3); // implicit ground + 2 authored
    CHECK_FALSE(session.level().hasFlag);

    InputManager input;
    core::KeyBindings bindings;
    RunDriver driver{session, input, bindings};

    REQUIRE(driver.chargedJump());
    CHECK(session.player().isGrounded());
    CHECK(session.player().position().y == doctest::Approx(140.0f));

    REQUIRE(driver.chargedJump());
    CHECK(session.player().isGrounded());
    CHECK(session.player().position().y == doctest::Approx(245.0f));
    CHECK(session.level().platformCount() == 4); // final platform was streamed
    REQUIRE(session.level().hasFlag);
    CHECK(session.level().flagBounds.min.y == doctest::Approx(382.0f));

    bool completed = false;
    for (int tick = 0; tick < 180 && !completed; ++tick) {
        if (tick < 24) {
            (void)driver.frame(true, tick == 0, false);
        } else if (tick == 24) {
            const auto result = driver.frame(false, false, true);
            completed = result.campaignCompleted;
        } else {
            const auto result = driver.frame();
            completed = result.campaignCompleted;
        }
    }

    REQUIRE(completed);
    CHECK(session.state() == core::GameState::CREDITS);

    std::ifstream runs(runsPath);
    REQUIRE(runs.is_open());
    std::string line;
    std::size_t lineCount = 0;
    while (std::getline(runs, line)) ++lineCount;
    CHECK(lineCount == 2);

    removeFile(firstLevel);
    removeFile(finalLevel);
    removeFile(runsPath);
}
}
