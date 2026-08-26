#include "Core/CampaignLoader.h"

#include <fstream>
#include <string>

namespace core {

std::vector<std::filesystem::path> CampaignLoader::load(
    const std::filesystem::path& campaignFile,
    const std::filesystem::path& levelsRoot) {
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
        levels.push_back(levelsRoot / line);
    }

    return levels;
}

} // namespace core
