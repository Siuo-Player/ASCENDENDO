#include "Core/RuntimeBootstrap.h"

#include "Core/CampaignID.h"
#include "Core/CampaignLoader.h"

#include <utility>

namespace core {

RuntimeBootstrapResult RuntimeBootstrap::prepare(RuntimePaths paths) {
    RuntimeBootstrapResult result{
        std::move(paths),
        false,
        {},
        {},
    };

    result.userDirectoriesReady = result.paths.ensureUserDirectories();
    result.campaign = CampaignLoader::load(
        result.paths.campaignFile(),
        result.paths.levelsRoot());
    result.campaignID = computeCampaignID(
        result.paths.levelsRoot().string());

    return result;
}

RuntimeBootstrapResult RuntimeBootstrap::fromProcess(const char* argv0) {
    return prepare(RuntimePaths::fromProcess(argv0));
}

} // namespace core
