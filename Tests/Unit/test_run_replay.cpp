#include "doctest/doctest.h"

#include "Core/Config.h"
#include "Core/Security.h"
#include "Logic/Level.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Logic/RunReplay.h"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace {

class TempDir {
public:
    TempDir() {
        const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
                ("ascendendo-replay-test-" + std::to_string(now));
        std::filesystem::create_directories(path_);
    }
    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path_, ec);
    }
    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

void writeLevel(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::trunc);
    out << content;
}

} // namespace

TEST_SUITE("RunReplay") {

TEST_CASE("TickInput compact bitmask encoding and decoding") {
    logic::TickInput in1;
    in1.left = true;
    in1.jumpHeld = true;
    const uint8_t byte1 = logic::RunReplay::encodeTickInput(in1);
    const logic::TickInput dec1 = logic::RunReplay::decodeTickInput(byte1);
    CHECK(dec1.left == true);
    CHECK(dec1.right == false);
    CHECK(dec1.jumpHeld == true);
    CHECK(dec1.jumpPressed == false);
    CHECK(dec1.jumpReleased == false);

    logic::TickInput in2;
    in2.right = true;
    in2.jumpReleased = true;
    const uint8_t byte2 = logic::RunReplay::encodeTickInput(in2);
    const logic::TickInput dec2 = logic::RunReplay::decodeTickInput(byte2);
    CHECK(dec2.left == false);
    CHECK(dec2.right == true);
    CHECK(dec2.jumpHeld == false);
    CHECK(dec2.jumpPressed == false);
    CHECK(dec2.jumpReleased == true);
}

TEST_CASE("RunReplay serialization roundtrip") {
    logic::RunReplayHeader header;
    header.version = 1;
    header.campaignID = "01-iniciacao";
    header.campaignName = "Primeira Ascensao";
    header.playerID = "as_71037d7c";
    header.playerName = "Speedrunner";
    header.claimedElapsedSeconds = 12.345f;
    header.timestamp = 1730000000;

    std::vector<logic::TickInput> inputs;
    for (int i = 0; i < 60; ++i) {
        logic::TickInput t;
        t.right = (i % 2 == 0);
        t.jumpHeld = (i > 10 && i < 30);
        inputs.push_back(t);
    }

    logic::RunReplay original(header, inputs);
    const std::string text = original.serialize();

    const auto restored = logic::RunReplay::deserialize(text);
    REQUIRE(restored.has_value());
    CHECK(restored->header().campaignID == "01-iniciacao");
    CHECK(restored->header().campaignName == "Primeira Ascensao");
    CHECK(restored->header().playerID == "as_71037d7c");
    CHECK(restored->header().playerName == "Speedrunner");
    CHECK(doctest::Approx(restored->header().claimedElapsedSeconds) == 12.345f);
    CHECK(restored->inputs().size() == 60);

    for (std::size_t i = 0; i < inputs.size(); ++i) {
        CHECK(restored->inputs()[i].right == inputs[i].right);
        CHECK(restored->inputs()[i].jumpHeld == inputs[i].jumpHeld);
    }
}

TEST_CASE("RunReplay secure file persistence and tamper detection") {
    TempDir temp;
    const auto replayFile = temp.path() / "test_run.replay";

    logic::RunReplayHeader header;
    header.campaignID = "01-iniciacao";
    header.claimedElapsedSeconds = 5.0f;
    std::vector<logic::TickInput> inputs(300);

    logic::RunReplay original(header, inputs);
    REQUIRE(original.saveToFile(replayFile));

    // Normal load
    const auto loaded = logic::RunReplay::loadFromFile(replayFile);
    REQUIRE(loaded.has_value());
    CHECK(loaded->header().campaignID == "01-iniciacao");
    CHECK(loaded->inputs().size() == 300);

    // Deliberate tampering: flip a byte in the file
    {
        std::fstream file(replayFile, std::ios::in | std::ios::out | std::ios::binary);
        file.seekp(64);
        char b = 0;
        file.get(b);
        file.seekp(64);
        file.put(static_cast<char>(b ^ 0xFF));
    }

    // Tampered file MUST be rejected by HMAC verification
    const auto tampered = logic::RunReplay::loadFromFile(replayFile);
    CHECK_FALSE(tampered.has_value());
}

TEST_CASE("RunReplay headless verification against campaign levels") {
    TempDir temp;
    const auto lvlPath = temp.path() / "test.lvl";

    // Create a level with a platform directly above the spawn floor
    // Spawn floor is Y=16, height 16. Spawn at X=320, Y=16.
    // Target platform at X=300, Y=64, W=100, H=16.
    writeLevel(lvlPath, "NAME ReplayTestLevel\nPLATFORM 300 64 100 16\n");

    const std::vector<std::filesystem::path> campaign = { lvlPath };

    // 1. Replay with no inputs -> player stays at spawn, does not reach goal
    logic::RunReplayHeader header;
    header.campaignID = "test";
    std::vector<logic::TickInput> idleInputs(60); // 1 second of idle
    logic::RunReplay idleReplay(header, idleInputs);

    const auto idleResult = idleReplay.verifyAgainstCampaign(campaign);
    CHECK(idleResult.valid == false);
    CHECK(idleResult.reachedGoal == false);
}

} // TEST_SUITE
