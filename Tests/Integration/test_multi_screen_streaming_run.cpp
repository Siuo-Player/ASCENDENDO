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

} // namespace

TEST_SUITE("MultiScreenStreamingRun") {
    TEST_CASE("GameSession streams the next screen before boundary and reaches the final FLAG") {
        const auto firstLevel = writeLevel(
            "ascendendo-stream-run-first.lvl",
            "NAME Stream First\n"
            "SCREENS 1\n"
            "SPAWN 100 20\n"
            "PLATFORM 0 4 640 16\n"
            "PLATFORM 0 180 640 16\n");
        const auto finalLevel = writeLevel(
            "ascendendo-stream-run-final.lvl",
            "NAME Stream Final\n"
            "SCREENS 1\n"
            "SPAWN 100 20\n"
            "PLATFORM 0 4 640 16\n"
            "FLAG 0 40 640 32\n");
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
        CHECK(session.player().position().y == doctest::Approx(20.0f));
        CHECK(session.level().platformCount() == 2);
        CHECK_FALSE(session.level().hasFlag);

        InputManager input;
        core::KeyBindings bindings;

        // Charge for the full configured 0.4 s, split around the runtime's
        // 0.25 s frame cap, then release. This produces the strongest normal
        // jump without changing any movement or physics constants.
        input.injectRawState(false, false, true, true, false);
        session.update(0.25f, input, bindings, 640, 360, 640.0f, 360.0f);
        session.update(0.15f, input, bindings, 640, 360, 640.0f, 360.0f);

        input.injectRawState(false, false, false, false, true);
        session.update(
            config::FIXED_STEP, input, bindings,
            640, 360, 640.0f, 360.0f);

        bool streamedBeforeBoundary = false;
        for (int frame = 0; frame < 120; ++frame) {
            input.injectRawState(false, false, false, false, false);
            session.update(
                config::FIXED_STEP, input, bindings,
                640, 360, 640.0f, 360.0f);

            if (session.level().platformCount() == 3) {
                streamedBeforeBoundary = true;
                CHECK(session.player().position().y < config::LOGICAL_HEIGHT);
                CHECK(session.level().hasFlag);
                break;
            }
        }

        REQUIRE(streamedBeforeBoundary);

        // The first elevated platform remains authoritative after streaming;
        // the player must be able to settle on it without a position jump.
        bool landedOnTransitionPlatform = false;
        for (int frame = 0; frame < 120; ++frame) {
            input.injectRawState(false, false, false, false, false);
            session.update(
                config::FIXED_STEP, input, bindings,
                640, 360, 640.0f, 360.0f);
            if (session.player().isGrounded() &&
                session.player().position().y == doctest::Approx(196.0f)) {
                landedOnTransitionPlatform = true;
                break;
            }
        }
        REQUIRE(landedOnTransitionPlatform);

        // A second full charge crosses from the first screen into the final
        // streamed level; its wide FLAG makes completion independent of
        // horizontal drift while still exercising real collision/completion.
        input.injectRawState(false, false, true, true, false);
        session.update(0.25f, input, bindings, 640, 360, 640.0f, 360.0f);
        session.update(0.15f, input, bindings, 640, 360, 640.0f, 360.0f);
        input.injectRawState(false, false, false, false, true);

        bool completed = false;
        float completionTime = 0.0f;
        for (int frame = 0; frame < 120; ++frame) {
            const auto result = session.update(
                config::FIXED_STEP, input, bindings,
                640, 360, 640.0f, 360.0f);
            input.injectRawState(false, false, false, false, false);
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
