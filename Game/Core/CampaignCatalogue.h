#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace core {

struct CampaignDescriptor {
    std::string id;
    std::string name;
    std::filesystem::path playlistFile;
};

struct CampaignCatalogueResult {
    bool exists = false;
    std::vector<CampaignDescriptor> campaigns;
    std::vector<std::string> errors;

    bool valid() const noexcept {
        return !exists || (!campaigns.empty() && errors.empty());
    }
};

class CampaignCatalogue {
public:
    // The catalogue is optional during the migration from the original single
    // campaign. When the file does not exist, callers can keep the legacy
    // campaign.txt path. Once present, malformed catalogues are errors.
    static CampaignCatalogueResult load(
        const std::filesystem::path& catalogueFile,
        const std::filesystem::path& assetsRoot);
};

} // namespace core
