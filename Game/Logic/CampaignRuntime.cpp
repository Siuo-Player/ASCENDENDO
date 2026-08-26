#include "Logic/CampaignRuntime.h"
#include "Logic/Level.h"
#include "Core/Config.h"

#include <utility>

namespace logic {

void CampaignRuntime::reset() {
    m_nextLevelIndex = 0;
    m_spawnY = 0.0f;
}

bool CampaignRuntime::loadInitialLevel(Level& level, float maxWidth) {
    reset();
    level.clear();

    if (m_campaign.empty()) return false;

    m_spawnY = level.appendFromFile(
        m_campaign.front().string(), maxWidth, 0.0f);
    m_nextLevelIndex = 1;
    return true;
}

bool CampaignRuntime::streamNextLevel(Level& level, float maxWidth) {
    if (!hasMoreLevels()) return false;

    const float nextSpawnY = level.appendFromFile(
        m_campaign[m_nextLevelIndex].string(), maxWidth, m_spawnY);
    ++m_nextLevelIndex;
    m_spawnY = nextSpawnY;
    return true;
}

} // namespace logic
