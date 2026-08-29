#include "Logic/CampaignRuntime.h"
#include "Logic/Level.h"
#include "Logic/LevelDataIO.h"
#include "Logic/LevelDataValidator.h"

#include <filesystem>

namespace logic {

void CampaignRuntime::reset() {
    m_nextLevelIndex = 0;
    m_spawnY = 0.0f;
}

bool CampaignRuntime::loadInitialLevel(Level& level, float maxWidth) {
    return loadLevelAt(level, 0, maxWidth);
}

bool CampaignRuntime::loadLevelAt(Level& level, std::size_t index, float maxWidth) {
    if (index >= m_campaign.size()) return false;
    if (!std::filesystem::exists(m_campaign[index])) return false;

    const auto data = LevelDataIO::load(m_campaign[index]);
    if (!data || !LevelDataValidator::validate(*data)) return false;

    level.clear();
    m_spawnY = level.appendFromData(*data, maxWidth, 0.0f);
    m_nextLevelIndex = index + 1;
    return true;
}

bool CampaignRuntime::streamNextLevel(Level& level, float maxWidth) {
    if (!hasMoreLevels()) return false;
    if (!std::filesystem::exists(m_campaign[m_nextLevelIndex])) return false;

    const auto data = LevelDataIO::load(m_campaign[m_nextLevelIndex]);
    if (!data || !LevelDataValidator::validate(*data)) return false;

    const float nextSpawnY = level.appendFromData(*data, maxWidth, m_spawnY);
    ++m_nextLevelIndex;
    m_spawnY = nextSpawnY;
    return true;
}

} // namespace logic
