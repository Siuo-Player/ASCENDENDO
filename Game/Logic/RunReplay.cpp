#include "Logic/RunReplay.h"

#include "Core/Config.h"
#include "Core/Security.h"
#include "Logic/CampaignRuntime.h"
#include "Logic/Level.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace logic {

namespace {
constexpr std::string_view REPLAY_HMAC_KEY = "ascendendo-replay-auth-key-v1";

constexpr uint8_t MASK_LEFT          = 0x01;
constexpr uint8_t MASK_RIGHT         = 0x02;
constexpr uint8_t MASK_JUMP_HELD     = 0x04;
constexpr uint8_t MASK_JUMP_PRESSED  = 0x08;
constexpr uint8_t MASK_JUMP_RELEASED = 0x10;

char nibbleToHex(uint8_t n) {
    n &= 0x0F;
    return (n < 10) ? static_cast<char>('0' + n) : static_cast<char>('a' + (n - 10));
}

int hexToNibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}
} // namespace

RunReplay::RunReplay(RunReplayHeader header, std::vector<TickInput> inputs)
    : header_(std::move(header)), inputs_(std::move(inputs)) {
    header_.totalTicks = static_cast<uint32_t>(inputs_.size());
}

void RunReplay::addTick(const TickInput& input) {
    inputs_.push_back(input);
    header_.totalTicks = static_cast<uint32_t>(inputs_.size());
}

void RunReplay::clear() {
    inputs_.clear();
    header_.totalTicks = 0;
}

uint8_t RunReplay::encodeTickInput(const TickInput& input) noexcept {
    uint8_t byte = 0;
    if (input.left)         byte |= MASK_LEFT;
    if (input.right)        byte |= MASK_RIGHT;
    if (input.jumpHeld)     byte |= MASK_JUMP_HELD;
    if (input.jumpPressed)  byte |= MASK_JUMP_PRESSED;
    if (input.jumpReleased) byte |= MASK_JUMP_RELEASED;
    return byte;
}

TickInput RunReplay::decodeTickInput(uint8_t byte) noexcept {
    TickInput input;
    input.left         = (byte & MASK_LEFT) != 0;
    input.right        = (byte & MASK_RIGHT) != 0;
    input.jumpHeld     = (byte & MASK_JUMP_HELD) != 0;
    input.jumpPressed  = (byte & MASK_JUMP_PRESSED) != 0;
    input.jumpReleased = (byte & MASK_JUMP_RELEASED) != 0;
    return input;
}

std::string RunReplay::serialize() const {
    std::ostringstream ss;
    ss << "ASCENDENDO_REPLAY_V1\n";
    ss << "version:" << header_.version << "\n";
    ss << "campaignID:" << header_.campaignID << "\n";
    ss << "campaignName:" << header_.campaignName << "\n";
    ss << "playerID:" << header_.playerID << "\n";
    ss << "playerName:" << header_.playerName << "\n";
    ss << "claimedSeconds:" << std::fixed << std::setprecision(4) << header_.claimedElapsedSeconds << "\n";
    ss << "totalTicks:" << inputs_.size() << "\n";
    ss << "timestamp:" << header_.timestamp << "\n";
    ss << "INPUTS:\n";

    // Encode ticks to hex stream
    for (const auto& tick : inputs_) {
        const uint8_t byte = encodeTickInput(tick);
        ss << nibbleToHex(byte >> 4) << nibbleToHex(byte);
    }
    ss << "\n";

    return ss.str();
}

std::optional<RunReplay> RunReplay::deserialize(std::string_view text) {
    std::istringstream stream{std::string(text)};
    std::string line;
    if (!std::getline(stream, line)) {
        return std::nullopt;
    }
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }
    if (line != "ASCENDENDO_REPLAY_V1") {
        return std::nullopt;
    }

    RunReplayHeader header;
    bool readingInputs = false;
    std::string hexInputs;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (readingInputs) {
            hexInputs += line;
            continue;
        }
        if (line == "INPUTS:") {
            readingInputs = true;
            continue;
        }

        const auto colon = line.find(':');
        if (colon == std::string::npos) continue;

        const std::string key = line.substr(0, colon);
        const std::string val = line.substr(colon + 1);

        if (key == "version") {
            header.version = static_cast<uint32_t>(std::stoul(val));
        } else if (key == "campaignID") {
            header.campaignID = val;
        } else if (key == "campaignName") {
            header.campaignName = val;
        } else if (key == "playerID") {
            header.playerID = val;
        } else if (key == "playerName") {
            header.playerName = val;
        } else if (key == "claimedSeconds") {
            header.claimedElapsedSeconds = std::stof(val);
        } else if (key == "totalTicks") {
            header.totalTicks = static_cast<uint32_t>(std::stoul(val));
        } else if (key == "timestamp") {
            header.timestamp = std::stoll(val);
        }
    }

    // Decode hex inputs
    std::vector<TickInput> inputs;
    inputs.reserve(hexInputs.size() / 2);
    for (std::size_t i = 0; i + 1 < hexInputs.size(); i += 2) {
        const int hi = hexToNibble(hexInputs[i]);
        const int lo = hexToNibble(hexInputs[i + 1]);
        if (hi < 0 || lo < 0) {
            return std::nullopt; // Corrupted hex stream
        }
        const uint8_t byte = static_cast<uint8_t>((hi << 4) | lo);
        inputs.push_back(decodeTickInput(byte));
    }

    if (!readingInputs) {
        return std::nullopt;
    }
    if (header.version == 0 || header.claimedElapsedSeconds < 0.0f || !std::isfinite(header.claimedElapsedSeconds)) {
        return std::nullopt;
    }

    header.totalTicks = static_cast<uint32_t>(inputs.size());
    return RunReplay(std::move(header), std::move(inputs));
}

bool RunReplay::saveToFile(const std::filesystem::path& path) const {
    return core::Security::writeSecureFile(path, serialize(), REPLAY_HMAC_KEY);
}

std::optional<RunReplay> RunReplay::loadFromFile(const std::filesystem::path& path) {
    const auto payload = core::Security::readSecureFile(path, REPLAY_HMAC_KEY);
    if (!payload) return std::nullopt;
    return deserialize(*payload);
}

ReplayVerificationResult RunReplay::verifyAgainstCampaign(
    const std::vector<std::filesystem::path>& levelPaths,
    float logicalWidth) const {
    ReplayVerificationResult result;

    if (levelPaths.empty()) {
        result.errorMessage = "Campaign contains no levels.";
        return result;
    }
    if (inputs_.empty()) {
        result.errorMessage = "Replay contains zero input ticks.";
        return result;
    }

    CampaignRuntime runtime(levelPaths);
    Level level;
    if (!runtime.loadInitialLevel(level, logicalWidth)) {
        result.errorMessage = "Failed to load initial campaign level.";
        return result;
    }

    Player player;
    player.body.position = level.spawnPosition;
    player.body.isGrounded = true;

    PhysicsWorld world;
    uint32_t currentTick = 0;

    for (const auto& tickInput : inputs_) {
        ++currentTick;
        player.update(tickInput, world, config::FIXED_STEP);
        level.resolveCollision(player.body);

        // Preload / stream next level when reaching boundary
        const float streamTriggerY =
            runtime.currentSpawnY() - config::CAMPAIGN_STREAM_PRELOAD_DISTANCE;
        if (player.position().y >= streamTriggerY && runtime.hasMoreLevels()) {
            runtime.streamNextLevel(level, logicalWidth);
        }

        // Check goal collision with final level flag
        if (level.hasFlag &&
            PhysicsWorld::collides(player.body.bounds(), level.flagBounds)) {
            result.reachedGoal = true;
            result.simulatedTicks = currentTick;
            result.simulatedElapsedSeconds = static_cast<float>(currentTick) * config::FIXED_STEP;
            result.valid = true;
            return result;
        }
    }

    // Finished inputs without reaching goal
    result.simulatedTicks = currentTick;
    result.simulatedElapsedSeconds = static_cast<float>(currentTick) * config::FIXED_STEP;
    result.reachedGoal = false;
    result.errorMessage = "Player did not reach the campaign goal within the recorded ticks.";
    return result;
}

} // namespace logic
