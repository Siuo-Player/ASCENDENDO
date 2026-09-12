#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace logic {

struct CampaignValidationEntry {
    std::string name;
    bool valid = false;
    std::size_t reachablePlatforms = 0;
    std::size_t totalPlatforms = 0;
};

struct CampaignValidationSnapshot {
    std::vector<CampaignValidationEntry> entries;
    std::size_t validLevels = 0;

    bool allValid() const noexcept {
        return !entries.empty() && validLevels == entries.size();
    }
};

CampaignValidationSnapshot validateCampaignForAdmin(
    const std::vector<std::string>& levelPaths);

} // namespace logic
