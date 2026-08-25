#include "Logic/CampaignEditor.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>

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
