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
TEST_CASE("GameSession streams the final level before boundary and uses the derived goal") {
    const auto firstLevel = writeLevel(
        "ascendendo-stream-run-first.lvl",
        "NAME Stream First\nSCREENS 1\n"
        "PLATFORM 0 100 640 16\n"
        "PLATFORM 0 170 640 16\n");
    const auto finalLevel = writeLevel(
        "ascendendo-stream-run-final.lvl",
        "NAME Stream Final\nSCREENS 1\n"
        "PLATFORM 280 16 80 16\n");
    const auto runsPath = std::filesystem::temp_directory_path() / "ascendendo-stream-run.csv";
    removeFile(runsPath);

    GameSession session(std::vector<std::filesystem::path>{firstLevel, finalLevel},
                        "streaming-test-campaign", runsPath.string());
    session.beginPlaying(static_cast<float>(config::LOGICAL_WIDTH));

    REQUIRE(session.state() == core::GameState::PLAYING);
    CHECK(session.player().position().x == doctest::Approx(320.0f));
    CHECK(session.player().position().y == doctest::Approx(16.0f));
    CHECK(session.level().platformCount() == 3); // implicit ground + 2 authored
    CHECK_FALSE(session.level().hasFlag);

    InputManager input;
    core::KeyBindings bindings;

    session.player().body.position.y =
        config::LOGICAL_HEIGHT - config::CAMPAIGN_STREAM_PRELOAD_DISTANCE;
    session.update(0.0f, input, bindings, 640, 360, 640.0f, 360.0f);

    REQUIRE(session.level().platformCount() == 4); // + final authored platform
    REQUIRE(session.level().hasFlag);
    CHECK(session.level().flagBounds.min.x == doctest::Approx(280.0f));
    CHECK(session.level().flagBounds.min.y == doctest::Approx(392.0f));
    CHECK(session.level().flagBounds.max.x == doctest::Approx(360.0f));
    CHECK(session.level().flagBounds.max.y == doctest::Approx(432.0f));

    // Complete through the normal GameSession overlap path using the derived goal.
    session.player().body.position = {300.0f, 392.0f};
    const auto result = session.update(0.0f, input, bindings,
                                       640, 360, 640.0f, 360.0f);

    CHECK(result.campaignCompleted);
    CHECK(result.runRecorded);
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
