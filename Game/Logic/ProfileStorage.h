#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace logic {

struct CampaignRecord {
    std::string campaignID;
    uint32_t completions = 0;
    float bestTimeSeconds = 0.0f;
    float lastTimeSeconds = 0.0f;
};

struct UserProfile {
    uint32_t version = 1;
    uint32_t totalCompletions = 0;
    float totalPlayTimeSeconds = 0.0f;
    std::string lastPlayedCampaignID;
    std::vector<CampaignRecord> records;

    void recordCompletion(const std::string& campaignID, float elapsedSeconds);
    const CampaignRecord* findRecord(const std::string& campaignID) const;

    std::string serialize() const;
    static std::optional<UserProfile> deserialize(std::string_view text);
};

class ProfileStorage {
public:
    static bool save(const std::filesystem::path& path, const UserProfile& profile);
    static UserProfile load(const std::filesystem::path& path);
};

} // namespace logic
