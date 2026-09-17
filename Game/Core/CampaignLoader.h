#pragma once

#include <filesystem>
#include <vector>

namespace core {

class CampaignLoader {
public:
    // Resolves playlist entries relative to the directory containing the
    // playlist itself. This is the canonical form for catalogued campaigns.
    static std::vector<std::filesystem::path> load(
        const std::filesystem::path& campaignFile);

    // Compatibility overload for the original single-campaign layout where
    // every playlist entry is resolved relative to Levels/.
    static std::vector<std::filesystem::path> load(
        const std::filesystem::path& campaignFile,
        const std::filesystem::path& levelsRoot);
};

} // namespace core
