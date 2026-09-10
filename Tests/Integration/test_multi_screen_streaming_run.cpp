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
void runFullChargeJump(GameSession& session, InputManager& input,
                       const core::KeyBindings& bindings) {
    input.injectRawState(false, false, true, true, false);
    session.update(0.25f, input, bindings, 640, 360, 640.0f, 360.0f);
    session.update(0.15f, input, bindings, 640, 360, 640.0f, 360.0f);
    input.injectRawState(false, false, false, false, true);
    session.update(config::FIXED_STEP, input, bindings, 640, 360, 640.0f, 360.0f);
}
bool waitForGroundedY(GameSession& session, InputManager& input,
                      const core::KeyBindings& bindings, float expectedY) {
    for (int frame = 0; frame < 120; ++frame) {
        input.injectRawState(false, false, false, false, false);
        session.update(config::FIXED_STEP, input, bindings, 640, 360, 640.0f, 360.0f);
        if (session.player().isGrounded() &&
            session.player().position().y == doctest::Approx(expectedY)) return true;
    }
    return false;
}
}

TEST_SUITE("MultiScreenStreamingRun") {
TEST_CASE("GameSession streams the next screen before boundary and reaches the final FLAG") {
    const auto firstLevel = writeLevel(
        "ascendendo-stream-run-first.lvl",
        "NAME Stream First\nSCREENS 1\nSPAWN 100 20\n"
        "PLATFORM 0 0 640 16\nPLATFORM 0 120 640 16\nPLATFORM 0 220 640 16\n");
    const auto finalLevel = writeLevel(
        "ascendendo-stream-run-final.lvl",
        "NAME Stream Final\nSCREENS 1\nSPAWN 100 20\n"
        "PLATFORM 0 0 640 16\nFLAG 0 16 640 32\n");
    const auto runsPath = std::filesystem::temp_directory_path() / "ascendendo-stream-run.csv";
    removeFile(runsPath);

    GameSession session(std::vector<std::filesystem::path>{firstLevel, finalLevel},
                        "streaming-test-campaign", runsPath.string());
    session.beginPlaying(static_cast<float>(config::LOGICAL_WIDTH));

    REQUIRE(session.state() == core::GameState::PLAYING);
    CHECK(session.player().position().x == doctest::Approx(100.0f));
    CHECK(session.player().position().y == doctest::Approx(20.0f));
    CHECK(session.level().platformCount() == 3);
    CHECK_FALSE(session.level().hasFlag);

    InputManager input;
    core::KeyBindings bindings;

    // GameSession starts from authored spawn; settle onto the explicit floor
    // so charging is exercised through the normal grounded input path.
    REQUIRE(waitForGroundedY(session, input, bindings, 16.0f));

    runFullChargeJump(session, input, bindings);
    REQUIRE(waitForGroundedY(session, input, bindings, 136.0f));

    // The second ascent crosses the 180px preload threshold before landing on
    // the next platform, causing the final campaign level to be appended at Y=360.
    runFullChargeJump(session, input, bindings);
    CHECK(session.level().platformCount() == 4);
    CHECK(session.level().hasFlag);
    CHECK(session.player().position().y < config::LOGICAL_HEIGHT);
    REQUIRE(waitForGroundedY(session, input, bindings, 236.0f));

    // The appended level's floor is authoritative after streaming. Its FLAG is
    // directly above that floor, so completion proves streamed geometry and
    // final-level completion are using the same GameSession simulation.
    bool completed = false;
    float completionTime = 0.0f;
    for (int frame = 0; frame < 120; ++frame) {
        input.injectRawState(false, false, false, false, false);
        const auto result = session.update(config::FIXED_STEP, input, bindings,
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