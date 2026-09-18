#include "Core/CampaignLoader.h"

#include <fstream>
#include <string>

namespace core {

namespace {

std::vector<std::filesystem::path> loadFromRoot(
    const std::filesystem::path& campaignFile,
    const std::filesystem::path& root) {
    std::vector<std::filesystem::path> levels;

    std::ifstream file(campaignFile);
    if (!file.is_open()) {
        return levels;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty() || line.front() == '#') {
            continue;
        }
        levels.push_back((root / line).lexically_normal());
    }

    return levels;
}

} // namespace

std::vector<std::filesystem::path> CampaignLoader::load(
    const std::filesystem::path& campaignFile) {
    return loadFromRoot(campaignFile, campaignFile.parent_path());
}

std::vector<std::filesystem::path> CampaignLoader::load(
    const std::filesystem::path& campaignFile,
    const std::filesystem::path& levelsRoot) {
    return loadFromRoot(campaignFile, levelsRoot);
}

} // namespace core
