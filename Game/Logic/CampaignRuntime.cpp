#include "Logic/CampaignRuntime.h"
#include "Logic/Level.h"
#include "Logic/LevelDataIO.h"
#include "Logic/LevelDataValidator.h"

#include <filesystem>

namespace logic {

namespace {

bool hasValidCampaignFlag(const LevelData& data, std::size_t levelIndex,
                          std::size_t campaignSize) noexcept {
    const bool isFinalLevel = levelIndex + 1 == campaignSize;
    return data.flag.has_value() == isFinalLevel;
}

} // namespace

void CampaignRuntime::reset() {
    m_nextLevelIndex = 0;
    m_spawnY = 0.0f;
}

bool CampaignRuntime::loadInitialLevel(Level& level, float maxWidth) {
    reset();
    level.clear();

    if (m_campaign.empty() || !std::filesystem::exists(m_campaign.front())) return false;

    const auto data = LevelDataIO::load(m_campaign.front());
    if (!data || !LevelDataValidator::validate(*data) ||
        !hasValidCampaignFlag(*data, 0, m_campaign.size())) {
        return false;
    }

    m_spawnY = level.appendFromData(*data, maxWidth, 0.0f);
    m_nextLevelIndex = 1;
    return true;
}

bool CampaignRuntime::loadLevelAt(Level& level, std::size_t index, float maxWidth) {
    if (index >= m_campaign.size()) return false;
    if (!std::filesystem::exists(m_campaign[index])) return false;

    const auto data = LevelDataIO::load(m_campaign[index]);
    if (!data || !LevelDataValidator::validate(*data) ||
        !hasValidCampaignFlag(*data, index, m_campaign.size())) {
        return false;
    }

    level.clear();
    m_spawnY = level.appendFromData(*data, maxWidth, 0.0f);
    m_nextLevelIndex = index + 1;
    return true;
}

bool CampaignRuntime::streamNextLevel(Level& level, float maxWidth) {
    if (!hasMoreLevels()) return false;
    if (!std::filesystem::exists(m_campaign[m_nextLevelIndex])) return false;

    const auto data = LevelDataIO::load(m_campaign[m_nextLevelIndex]);
    if (!data || !LevelDataValidator::validate(*data) ||
        !hasValidCampaignFlag(*data, m_nextLevelIndex, m_campaign.size())) {
        return false;
    }

    const float nextSpawnY = level.appendFromData(*data, maxWidth, m_spawnY);
    ++m_nextLevelIndex;
    m_spawnY = nextSpawnY;
    return true;
}

} // namespace logic
