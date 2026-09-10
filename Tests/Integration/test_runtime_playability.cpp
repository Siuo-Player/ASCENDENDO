#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/GameSession.h"
#include "../../Game/Logic/Level.h"
#include "../../Game/Logic/Player.h"
#include "../../Game/Logic/TickInput.h"
#include "../../Game/Core/Config.h"
#include "../../Game/Core/KeyBindings.h"

#include <filesystem>
#include <fstream>
#include <string>

using namespace logic;

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

TEST_SUITE("RuntimePlayability") {
    TEST_CASE("player can jump and land on an elevated platform using runtime physics") {
        Level level;
        level.addPlatform(0.0f, 0.0f, 220.0f, 10.0f);
        level.addPlatform(50.0f, 10.0f, 120.0f, 10.0f);

        Player player;
        player.body.position = {10.0f, 10.0f};
        player.body.isGrounded = true;
        player.facingDirection = 1.0f;

        PhysicsWorld world;
        TickInput jump;
        jump.jumpPressed = true;
        jump.jumpReleased = true;

        bool landedElevated = false;
        for (int tick = 0; tick < 120; ++tick) {
            player.update(tick == 0 ? jump : TickInput{}, world, config::FIXED_STEP);
            level.resolveCollision(player.body);

            if (player.isGrounded() && player.position().y == 20.0f) {
                landedElevated = true;
                break;
            }
        }

        REQUIRE(landedElevated);
        CHECK(player.position().x >= 50.0f - player.body.width);
        CHECK(player.position().x <= 170.0f);
        CHECK(player.velocity().y == 0.0f);
    }

    TEST_CASE("GameSession executes an authored spawn to FLAG run through semantic input") {
        const auto path = writeTestLevel(
            "ascendendo-runtime-complete-run.lvl",
            "NAME Complete Run\n"
            "SCREENS 1\n"
            "SPAWN 100 20\n"
            "PLATFORM 0 4 640 16\n"
            "FLAG 160 20 48 32\n");
        const auto runsPath = std::filesystem::temp_directory_path() /
            "ascendendo-runtime-complete-run.csv";
        removeTestFile(runsPath);

        GameSession session({path}, "test-campaign", runsPath.string());
        session.beginPlaying(static_cast<float>(config::LOGICAL_WIDTH));
        REQUIRE(session.state() == core::GameState::PLAYING);
        CHECK(session.player().position().x == doctest::Approx(100.0f));
        CHECK(session.player().position().y == doctest::Approx(20.0f));
        CHECK(session.level().hasFlag);

        InputManager input;
        core::KeyBindings bindings;
        input.injectRawState(false, true, false, false, false);

        const auto firstFrame = session.update(
            0.25f, input, bindings,
            640, 360, 640.0f, 360.0f);
        CHECK_FALSE(firstFrame.campaignCompleted);
        REQUIRE(session.state() == core::GameState::PLAYING);
        CHECK(session.player().position().x > 100.0f);
        CHECK(session.player().position().x < 160.0f);

        const auto result = session.update(
            0.25f, input, bindings,
            640, 360, 640.0f, 360.0f);

        CHECK(result.campaignCompleted);
        CHECK(result.runRecorded);
        CHECK(result.completionElapsedSeconds == doctest::Approx(0.5f));
        CHECK(session.state() == core::GameState::CREDITS);
        CHECK(session.elapsedTime() == doctest::Approx(0.5f));
        CHECK(session.player().position().x >= 164.0f);

        std::ifstream runs(runsPath);
        REQUIRE(runs.is_open());
        std::string line;
        std::size_t lineCount = 0;
        while (std::getline(runs, line)) ++lineCount;
        CHECK(lineCount == 2);

        removeTestFile(path);
        removeTestFile(runsPath);
    }
}
