#pragma once

#include <filesystem>
#include <vector>

namespace core {

class CampaignLoader {
public:
    static std::vector<std::filesystem::path> load(
        const std::filesystem::path& campaignFile,
        const std::filesystem::path& levelsRoot);
};

} // namespace core
