#include "Core/CampaignCatalogue.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <unordered_set>

namespace core {

namespace {

std::string trim(std::string value) {
    const auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

bool validId(const std::string& id) {
    if (id.empty()) return false;
    if (!std::isalnum(static_cast<unsigned char>(id.front()))) return false;
    for (const unsigned char ch : id) {
        if (!(std::isalnum(ch) || ch == '_' || ch == '-')) return false;
    }
    return true;
}

bool portableRelativePath(const std::string& value) {
    const std::filesystem::path path(value);
    if (path.empty() || path.is_absolute() || value.find('\\') != std::string::npos) {
        return false;
    }
    for (const auto& part : path) {
        if (part == ".." || part.empty()) return false;
    }
    return true;
}

bool insideRoot(const std::filesystem::path& path,
                const std::filesystem::path& root) {
    const auto relative = path.lexically_relative(root);
    if (relative.empty()) return true;
    const auto first = relative.begin();
    return first == relative.end() || *first != "..";
}

} // namespace

CampaignCatalogueResult CampaignCatalogue::load(
    const std::filesystem::path& catalogueFile,
    const std::filesystem::path& assetsRoot) {
    CampaignCatalogueResult result;
    result.exists = std::filesystem::is_regular_file(catalogueFile);
    if (!result.exists) return result;

    std::ifstream file(catalogueFile);
    if (!file.is_open()) {
        result.errors.push_back("unable to open campaign catalogue");
        return result;
    }

    std::error_code ec;
    const auto canonicalRoot = std::filesystem::weakly_canonical(assetsRoot, ec);
    if (ec) {
        result.errors.push_back("unable to resolve assets root");
        return result;
    }

    const auto canonicalCatalogue = std::filesystem::weakly_canonical(catalogueFile, ec);
    if (ec || !insideRoot(canonicalCatalogue, canonicalRoot)) {
        result.errors.push_back("campaign catalogue is outside the assets root");
        return result;
    }

    std::unordered_set<std::string> seenIds;
    std::unordered_set<std::string> seenPlaylists;

    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(file, line)) {
        ++lineNumber;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        line = trim(std::move(line));
        if (line.empty() || line.front() == '#') continue;

        const auto firstSeparator = line.find('|');
        const auto secondSeparator =
            firstSeparator == std::string::npos ? std::string::npos : line.find('|', firstSeparator + 1);
        if (firstSeparator == std::string::npos || secondSeparator == std::string::npos ||
            line.find('|', secondSeparator + 1) != std::string::npos) {
            result.errors.push_back(
                "line " + std::to_string(lineNumber) + ": expected id|name|playlist");
            continue;
        }

        const std::string id = trim(line.substr(0, firstSeparator));
        const std::string name = trim(line.substr(firstSeparator + 1,
                                                  secondSeparator - firstSeparator - 1));
        const std::string playlist = trim(line.substr(secondSeparator + 1));

        if (!validId(id)) {
            result.errors.push_back("line " + std::to_string(lineNumber) + ": invalid campaign id");
            continue;
        }
        if (name.empty()) {
            result.errors.push_back("line " + std::to_string(lineNumber) + ": campaign name is empty");
            continue;
        }
        if (!portableRelativePath(playlist) ||
            std::filesystem::path(playlist).filename() != "campaign.txt") {
            result.errors.push_back(
                "line " + std::to_string(lineNumber) + ": playlist must be a portable relative path ending in campaign.txt");
            continue;
        }

        if (!seenIds.insert(id).second) {
            result.errors.push_back("line " + std::to_string(lineNumber) + ": duplicate campaign id: " + id);
            continue;
        }

        const auto playlistPath = (catalogueFile.parent_path() / playlist).lexically_normal();
        const auto canonicalPlaylist = std::filesystem::weakly_canonical(playlistPath, ec);
        if (ec || !insideRoot(canonicalPlaylist, canonicalRoot)) {
            result.errors.push_back(
                "line " + std::to_string(lineNumber) + ": playlist escapes the assets root");
            continue;
        }
        if (!std::filesystem::is_regular_file(canonicalPlaylist, ec) || ec) {
            result.errors.push_back(
                "line " + std::to_string(lineNumber) + ": playlist does not exist: " + playlist);
            continue;
        }

        const std::string playlistKey = canonicalPlaylist.lexically_normal().string();
        if (!seenPlaylists.insert(playlistKey).second) {
            result.errors.push_back(
                "line " + std::to_string(lineNumber) + ": duplicate campaign playlist: " + playlist);
            continue;
        }

        result.campaigns.push_back({id, name, canonicalPlaylist});
    }

    if (result.campaigns.empty() && result.errors.empty()) {
        result.errors.push_back("campaign catalogue contains no campaigns");
    }

    return result;
}

} // namespace core
