#pragma once
// =============================================================================
//  Game/Logic/RunReplay.h
//
//  Ascendendo Authenticated Run Replay & Ghost Verification Subsystem
//  - Records tick-by-tick inputs for complete campaign speedruns
//  - Serializes to authenticated .replay format with HMAC-SHA256
//  - Headless verification engine: re-simulates physics tick-by-tick
//    to mathematically prove a speedrun's time and completion without GPU
// =============================================================================

#include "Logic/TickInput.h"
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace logic {

struct RunReplayHeader {
    uint32_t version = 1;
    std::string campaignID;
    std::string campaignName;
    std::string playerID;
    std::string playerName;
    float claimedElapsedSeconds = 0.0f;
    uint32_t totalTicks = 0;
    int64_t timestamp = 0;
};

struct ReplayVerificationResult {
    bool valid = false;
    bool reachedGoal = false;
    float simulatedElapsedSeconds = 0.0f;
    uint32_t simulatedTicks = 0;
    std::string errorMessage;
};

class RunReplay {
public:
    RunReplay() = default;
    RunReplay(RunReplayHeader header, std::vector<TickInput> inputs);

    const RunReplayHeader& header() const noexcept { return header_; }
    RunReplayHeader& header() noexcept { return header_; }
    const std::vector<TickInput>& inputs() const noexcept { return inputs_; }

    void addTick(const TickInput& input);
    void clear();

    // ── Serialization & Integrity ────────────────────────────────────────────
    std::string serialize() const;
    static std::optional<RunReplay> deserialize(std::string_view text);

    bool saveToFile(const std::filesystem::path& path) const;
    static std::optional<RunReplay> loadFromFile(const std::filesystem::path& path);

    // ── Headless Deterministic Verification ───────────────────────────────────
    // Re-simulates the exact physics loop on campaign levels.
    // Proves whether the player physically reaches the goal and computes the exact time.
    ReplayVerificationResult verifyAgainstCampaign(
        const std::vector<std::filesystem::path>& levelPaths,
        float logicalWidth = 640.0f) const;

    // Compact encoding helpers
    static uint8_t encodeTickInput(const TickInput& input) noexcept;
    static TickInput decodeTickInput(uint8_t byte) noexcept;

private:
    RunReplayHeader header_;
    std::vector<TickInput> inputs_;
};

} // namespace logic
