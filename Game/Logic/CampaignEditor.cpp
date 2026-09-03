#include "Logic/CampaignEditor.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_set>
#include <utility>

namespace logic {

namespace {
std::string readLevelName(const std::filesystem::path& path) {
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.rfind("NAME ", 0) == 0) return line.substr(5);
    }
    return path.stem().string();
}

bool isInside(const std::filesystem::path& path,
              const std::filesystem::path& root) {
    const auto relative = path.lexically_relative(root);
    if (relative.empty()) return true;
    auto it = relative.begin();
    return it == relative.end() || *it != "..";
}
}

bool CampaignEditorDocument::loadFromCampaignFile(const std::string& campaignPath) {
    std::ifstream file(campaignPath);
    if (!file.is_open()) return false;

    m_levels.clear();
    m_selectedIndex = static_cast<std::size_t>(-1);

    const std::filesystem::path base = std::filesystem::path(campaignPath).parent_path();
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty() || line[0] == '#') continue;

        const std::filesystem::path levelPath = base / line;
        CampaignLevelBlock block;
        block.path = levelPath.lexically_normal().string();
        block.name = readLevelName(levelPath);
        block.order = m_levels.size();
        m_levels.push_back(std::move(block));
    }

    rebuildPositions();
    if (!m_levels.empty()) select(0);
    return true;
}

bool CampaignEditorDocument::moveLevel(std::size_t index, std::size_t newOrder) {
    if (index >= m_levels.size() || newOrder >= m_levels.size()) return false;
    if (index == newOrder) return true;

    CampaignLevelBlock moved = m_levels[index];
    m_levels.erase(m_levels.begin() + static_cast<std::ptrdiff_t>(index));
    m_levels.insert(m_levels.begin() + static_cast<std::ptrdiff_t>(newOrder), std::move(moved));

    for (std::size_t i = 0; i < m_levels.size(); ++i) {
        m_levels[i].order = i;
    }
    m_selectedIndex = newOrder;
    rebuildPositions();
    return true;
}

CampaignValidationResult CampaignEditorDocument::validateCampaign() const {
    if (m_levels.empty()) {
        return {false, "campaign contains no levels"};
    }

    std::unordered_set<std::string> seen;
    for (std::size_t i = 0; i < m_levels.size(); ++i) {
        const std::filesystem::path levelPath(m_levels[i].path);
        if (levelPath.empty() || levelPath.extension() != ".lvl") {
            return {false, "campaign entry " + std::to_string(i) + " is not a .lvl file"};
        }

        std::error_code ec;
        const auto canonicalLevel = std::filesystem::weakly_canonical(levelPath, ec);
        if (ec || !std::filesystem::is_regular_file(canonicalLevel, ec) || ec) {
            return {false, "campaign entry " + std::to_string(i) + " does not reference an existing level"};
        }

        const std::filesystem::path canonicalRoot =
            std::filesystem::weakly_canonical(levelPath.parent_path(), ec);
        if (ec || !isInside(canonicalLevel, canonicalRoot)) {
            return {false, "campaign entry " + std::to_string(i) + " escapes its level directory"};
        }

        const std::string identity = canonicalLevel.lexically_normal().string();
        if (!seen.insert(identity).second) {
            return {false, "campaign contains duplicate level references"};
        }
    }

    return {true, "campaign is valid"};
}

CampaignSaveResult CampaignEditorDocument::saveToCampaignFile(const std::string& campaignPath) const {
    const auto validation = validateCampaign();
    if (!validation.valid) {
        return {false, campaignPath, validation.message};
    }

    const std::filesystem::path destination(campaignPath);
    const std::filesystem::path base = destination.parent_path();
    if (base.empty()) {
        return {false, campaignPath, "campaign destination has no parent directory"};
    }

    std::error_code ec;
    std::filesystem::create_directories(base, ec);
    if (ec) {
        return {false, campaignPath, "unable to create campaign directory"};
    }

    for (const auto& level : m_levels) {
        const std::filesystem::path levelPath(level.path);
        const auto canonicalLevel = std::filesystem::weakly_canonical(levelPath, ec);
        if (ec) {
            return {false, campaignPath, "unable to resolve campaign level path"};
        }
        const auto canonicalBase = std::filesystem::weakly_canonical(base, ec);
        if (ec || !isInside(canonicalLevel, canonicalBase)) {
            return {false, campaignPath, "campaign entry is outside the campaign directory"};
        }
    }

    const std::filesystem::path temporary = destination.string() + ".tmp-save";
    std::filesystem::remove(temporary, ec);

    {
        std::ofstream file(temporary, std::ios::trunc);
        if (!file.is_open()) {
            return {false, campaignPath, "unable to open temporary campaign file"};
        }

        file << "# Playlist da Campanha (A ordem das linhas define a ordem no jogo)\n";
        for (const auto& level : m_levels) {
            const std::filesystem::path relative =
                std::filesystem::path(level.path).lexically_relative(base);
            if (relative.empty() || relative == "." ||
                (!relative.empty() && *relative.begin() == "..")) {
                file.close();
                std::filesystem::remove(temporary, ec);
                return {false, campaignPath, "campaign entry cannot be represented relative to campaign file"};
            }
            file << relative.generic_string() << '\n';
        }

        file.flush();
        if (!file.good()) {
            file.close();
            std::filesystem::remove(temporary, ec);
            return {false, campaignPath, "failed while writing temporary campaign file"};
        }
    }

    std::filesystem::remove(destination, ec);
    ec.clear();
    std::filesystem::rename(temporary, destination, ec);
    if (ec) {
        std::filesystem::remove(temporary, ec);
        return {false, campaignPath, "unable to replace campaign file"};
    }

    return {true, campaignPath, "campaign saved"};
}

void CampaignEditorDocument::select(std::size_t index) {
    if (index >= m_levels.size()) return;
    m_selectedIndex = index;
    for (std::size_t i = 0; i < m_levels.size(); ++i)
        m_levels[i].selected = (i == index);
}

float CampaignEditorDocument::contentHeight() const {
    if (m_levels.empty()) return 0.0f;
    return m_levels.size() * PREVIEW_HEIGHT +
           (m_levels.size() - 1) * PREVIEW_GAP;
}

float CampaignEditorDocument::topYForOrder(std::size_t order) const {
    return static_cast<float>(order) * (PREVIEW_HEIGHT + PREVIEW_GAP);
}

void CampaignEditorDocument::rebuildPositions() {
    for (std::size_t i = 0; i < m_levels.size(); ++i) {
        m_levels[i].order = i;
        m_levels[i].topY = topYForOrder(i);
    }
}

} // namespace logic
