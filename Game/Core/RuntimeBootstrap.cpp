#include "Core/RuntimeBootstrap.h"

#include "Core/CampaignID.h"
#include "Core/CampaignLoader.h"

#include <utility>

namespace core {

RuntimeBootstrapResult RuntimeBootstrap::prepare(RuntimePaths paths) {
    RuntimeBootstrapResult result{
        std::move(paths),
        false,
        false,
        true,
        {},
        {},
        0,
        {},
        {},
    };

    result.userDirectoriesReady = result.paths.ensureUserDirectories();

    const CampaignCatalogueResult catalogue = CampaignCatalogue::load(
        result.paths.campaignCatalogueFile(),
        result.paths.assetsRoot());
    result.campaignCataloguePresent = catalogue.exists;
    result.campaignCatalogueValid = catalogue.valid();
    result.campaignCatalogueErrors = catalogue.errors;

    if (catalogue.exists) {
        if (!catalogue.valid()) return result;
        result.campaigns = catalogue.campaigns;
    } else {
        result.campaigns.push_back({
            "main",
            "Campanha Principal",
            result.paths.campaignFile(),
        });
    }

    const CampaignDescriptor& active = result.campaigns.front();
    result.campaign = CampaignLoader::load(active.playlistFile);
    result.campaignID = computeCampaignID(active.playlistFile.parent_path().string());

    return result;
}

RuntimeBootstrapResult RuntimeBootstrap::fromProcess(const char* argv0) {
    return prepare(RuntimePaths::fromProcess(argv0));
}

} // namespace core
