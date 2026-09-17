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

constexpr float TEST_FRAME_DT = config::FIXED_STEP * 2.0f;

enum class HorizontalDirection {
    NONE,
    LEFT,
    RIGHT,
};

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

    GameSessionUpdateResult frame(HorizontalDirection direction = HorizontalDirection::NONE,
                                  bool jumpHeld = false,
                                  bool jumpPressed = false,
                                  bool jumpReleased = false) {
        const bool left = direction == HorizontalDirection::LEFT;
        const bool right = direction == HorizontalDirection::RIGHT;
        input.injectRawState(left, right, jumpHeld, jumpPressed, jumpReleased);
        return session.update(
            TEST_FRAME_DT,
            input,
            bindings,
            static_cast<int>(config::LOGICAL_WIDTH),
            static_cast<int>(config::LOGICAL_HEIGHT),
            config::LOGICAL_WIDTH,
            config::LOGICAL_HEIGHT);
    }

    bool chargedJump(HorizontalDirection direction) {
        (void)frame(direction, true, true, false);
        for (int frameIndex = 0; frameIndex < 30 && session.player().chargeRatio() < 1.0f; ++frameIndex) {
            (void)frame(direction, true, false, false);
        }
        if (session.player().chargeRatio() < 1.0f) return false;

        auto result = frame(direction, false, false, true);
        if (result.campaignCompleted) return true;

        bool leftGround = !session.player().isGrounded();
        for (int frameIndex = 0; frameIndex < 90; ++frameIndex) {
            result = frame();
            leftGround = leftGround || !session.player().isGrounded();
            if (result.campaignCompleted) return true;
            if (leftGround && session.player().isGrounded()) return true;
        }
        return false;
    }
};

} // namespace

TEST_SUITE("StableGameplayLoop") {
TEST_CASE("complete run uses real charge physics, streams vertically, and reaches the derived flag") {
    // Each jump corridor is separated from the lower platform that was just used.
    // The final goal platform is narrow enough to pass on its right side during
    // ascent, while remaining wide enough to catch the same trajectory on descent.
    const auto firstLevel = writeLevel(
        "ascendendo-acceptance-first.lvl",
        "NAME Acceptance First\n"
        "SCREENS 1\n"
        "PLATFORM 500 104 140 16\n"
        "PLATFORM 240 230 170 16\n");
    const auto finalLevel = writeLevel(
        "ascendendo-acceptance-final.lvl",
        "NAME Acceptance Final\n"
        "SCREENS 1\n"
        "PLATFORM 0 16 160 1\n");
    const auto runsPath = std::filesystem::temp_directory_path() /
        "ascendendo-acceptance-stable-gameplay.csv";
    removeFile(runsPath);

    GameSession session(
        std::vector<std::filesystem::path>{firstLevel, finalLevel},
        "stable-gameplay-acceptance",
        runsPath.string());
    session.beginPlaying(config::LOGICAL_WIDTH);

    InputManager input;
    core::KeyBindings bindings;
    RunDriver driver{session, input, bindings};

    REQUIRE(session.state() == core::GameState::PLAYING);
    CHECK(session.player().position().x == doctest::Approx(320.0f));
    CHECK(session.player().position().y == doctest::Approx(16.0f));
    CHECK(session.level().platformCount() == 3); // implicit ground + 2 authored
    CHECK_FALSE(session.level().hasFlag);

    REQUIRE(driver.chargedJump(HorizontalDirection::RIGHT));
    CHECK(session.player().isGrounded());
    CHECK(session.player().position().y == doctest::Approx(120.0f));

    REQUIRE(driver.chargedJump(HorizontalDirection::LEFT));
    CHECK(session.player().isGrounded());
    CHECK(session.player().position().y == doctest::Approx(246.0f));
    CHECK(session.player().position().y >=
          config::LOGICAL_HEIGHT - config::CAMPAIGN_STREAM_PRELOAD_DISTANCE);
    (void)driver.frame(); // run the normal GameSession streaming check at the landing state
    CHECK(session.level().platformCount() == 4); // final platform was streamed
    REQUIRE(session.level().hasFlag);
    CHECK(session.level().flagBounds.min.y == doctest::Approx(377.0f));

    bool completed = false;
    bool jumpReleased = false;
    for (int frameIndex = 0; frameIndex < 120 && !completed; ++frameIndex) {
        if (frameIndex == 0) {
            (void)driver.frame(HorizontalDirection::LEFT, true, true, false);
        } else if (!jumpReleased && session.player().chargeRatio() < 1.0f) {
            (void)driver.frame(HorizontalDirection::LEFT, true, false, false);
        } else if (!jumpReleased) {
            const auto result = driver.frame(HorizontalDirection::LEFT, false, false, true);
            jumpReleased = true;
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