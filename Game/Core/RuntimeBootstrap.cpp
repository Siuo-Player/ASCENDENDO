#include "Core/RuntimeBootstrap.h"

#include "Core/CampaignID.h"
#include "Core/CampaignLoader.h"

namespace core {

RuntimeBootstrapResult RuntimeBootstrap::prepare(
    const char* argv0,
    bool* userDirectoriesReady) {
    RuntimeBootstrapResult result{
        RuntimePaths::fromProcess(argv0),
        {},
        {},
    };

    const bool directoriesReady = result.paths.ensureUserDirectories();
    if (userDirectoriesReady) {
        *userDirectoriesReady = directoriesReady;
    }

    result.campaign = CampaignLoader::load(
        result.paths.campaignFile(),
        result.paths.levelsRoot());

    result.campaignID = computeCampaignID(
        result.paths.levelsRoot().string());

    return result;
}

} // namespace core
