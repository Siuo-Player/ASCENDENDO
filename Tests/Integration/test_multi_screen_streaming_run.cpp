#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/GameSession.h"
#include "../../Game/Logic/InputManager.h"
#include "../../Game/Core/Config.h"
#include "../../Game/Core/KeyBindings.h"

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

void runFullChargeJump(GameSession& session,
                       InputManager& input,
                       const core::KeyBindings& bindings) {
    input.injectRawState(false, false, true, true, false);
    session.update(0.25f, input, bindings, 640, 360, 640.0f, 360.0f);
    session.update(0.15f, input, bindings, 640, 360, 640.0f, 360.0f);

    input.injectRawState(false, false, false, false, true);
    session.update(config::FIXED_STEP, input, bindings,
                   640, 360, 640.0f, 360.0f);
}

bool waitForGroundedY(GameSession& session,
                      InputManager& input,
                      const core::KeyBindings& bindings,
                      float expectedY) {
    for (int frame = 0; frame < 120; ++frame) {
        input.injectRawState(false, false, false, false, false);
        session.update(config::FIXED_STEP, input, bindings,
                       640, 360, 640.0f, 360.0f);
        if (session.player().isGrounded() &&
            session.player().position().y == doctest::Approx(expectedY)) {
            return true;
        }
    }
    return false;
}

} // namespace

TEST_SUITE("MultiScreenStreamingRun") {
    TEST_CASE("GameSession streams the next screen before boundary and reaches the final FLAG") {
        const auto firstLevel = writeLevel(
            "ascendendo-stream-run-first.lvl",
            "NAME Stream First\n"
            "SCREENS 1\n"
            "SPAWN 100 60\n"
            "PLATFORM 0 120 640 16\n"
            "PLATFORM 0 220 640 16\n");
        const auto finalLevel = writeLevel(
            "ascendendo-stream-run-final.lvl",
            "NAME Stream Final\n"
            "SCREENS 1\n"
            "SPAWN 100 60\n"
            "PLATFORM 0 4 640 16\n"
            "FLAG 0 20 640 32\n");
        const auto runsPath = std::filesystem::temp_directory_path() /
            "ascendendo-stream-run.csv";
        removeFile(runsPath);

        GameSession session(
            std::vector<std::filesystem::path>{firstLevel, finalLevel},
            "streaming-test-campaign",
            runsPath.string());
        session.beginPlaying(static_cast<float>(config::LOGICAL_WIDTH));

        REQUIRE(session.state() == core::GameState::PLAYING);
        CHECK(session.player().position().x == doctest::Approx(100.0f));
        CHECK(session.player().position().y == doctest::Approx(60.0f));
        CHECK(session.level().platformCount() == 2);
        CHECK_FALSE(session.level().hasFlag);

        InputManager input;
        core::KeyBindings bindings;

        // First reachable step: spawn at y=60 -> platform top 120.
        runFullChargeJump(session, input, bindings);
        REQUIRE(waitForGroundedY(session, input, bindings, 136.0f));

        // Second jump reaches the next platform and crosses the streaming
        // preload threshold (180) before landing on the y=220 platform.
        runFullChargeJump(session, input, bindings);
        CHECK(session.level().platformCount() == 3);
        CHECK(session.level().hasFlag);
        CHECK(session.player().position().y < config::LOGICAL_HEIGHT);
        REQUIRE(waitForGroundedY(session, input, bindings, 236.0f));

        // The final streamed FLAG is in the next campaign level. A full charge
        // takes the player upward through the streamed boundary; completion is
        // checked through GameSession rather than manipulating player state.
        runFullChargeJump(session, input, bindings);

        bool completed = false;
        float completionTime = 0.0f;
        for (int frame = 0; frame < 120; ++frame) {
            input.injectRawState(false, false, false, false, false);
            const auto result = session.update(
                config::FIXED_STEP, input, bindings,
                640, 360, 640.0f, 360.0f);
            if (result.campaignCompleted) {
                completed = true;
                completionTime = result.completionElapsedSeconds;
                CHECK(result.runRecorded);
                break;
            }
        }

        CHECK(completed);
        CHECK(completionTime == doctest::Approx(session.elapsedTime()));
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
