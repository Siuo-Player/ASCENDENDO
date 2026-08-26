#pragma once

#include <cstddef>
#include <filesystem>
#include <vector>

namespace logic {

class Level;

class CampaignRuntime {
public:
    CampaignRuntime() = default;
    explicit CampaignRuntime(std::vector<std::filesystem::path> campaign)
        : m_campaign(std::move(campaign)) {}

    void reset();
    bool loadInitialLevel(Level& level, float maxWidth);
    bool streamNextLevel(Level& level, float maxWidth);

    bool hasMoreLevels() const { return m_nextLevelIndex < m_campaign.size(); }
    std::size_t currentLevelIndex() const { return m_nextLevelIndex; }
    float currentSpawnY() const { return m_spawnY; }
    std::size_t levelCount() const { return m_campaign.size(); }
    const std::vector<std::filesystem::path>& levels() const { return m_campaign; }

private:
    std::vector<std::filesystem::path> m_campaign;
    std::size_t m_nextLevelIndex = 0;
    float m_spawnY = 0.0f;
};

} // namespace logic
