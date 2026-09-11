#pragma once

#include "Logic/LevelData.h"
#include "Logic/Physics.h"

#include <string>
#include <vector>

namespace logic {

struct Platform {
    AABB bounds;
};

class Level {
public:
    static constexpr float AUTO_GROUND_HEIGHT = 16.0f;
    static constexpr float AUTO_GROUND_SPAWN_X = config::LOGICAL_WIDTH / 2.0f;

    std::string name = "Sem Nome";
    bool hasFlag = false;
    AABB flagBounds;
    Vec2 spawnPosition = {AUTO_GROUND_SPAWN_X, AUTO_GROUND_HEIGHT};

    // Append one local LevelData chunk into world coordinates.
    // The implicit ground exists only at world Y=0..16 for the initial chunk.
    // The campaign FLAG is derived automatically from the highest authored
    // platform only when this chunk is the final campaign level.
    float appendFromData(const LevelData& data,
                         float maxWidth,
                         float offsetY,
                         bool finalCampaignLevel = false);

    void addPlatform(float x, float y, float w, float h);
    bool resolveCollision(PhysicsBody& body) const;

    const std::vector<Platform>& platforms() const { return m_platforms; }
    int platformCount() const { return static_cast<int>(m_platforms.size()); }

    void clear() {
        m_platforms.clear();
        name = "Sem Nome";
        hasFlag = false;
        flagBounds = {};
        spawnPosition = {AUTO_GROUND_SPAWN_X, AUTO_GROUND_HEIGHT};
        m_spawnInitialized = false;
    }

private:
    static constexpr float SKIN_WIDTH = 1.0f;
    static constexpr float AUTO_FLAG_HEIGHT = 40.0f;
    std::vector<Platform> m_platforms;
    bool m_spawnInitialized = false;
};

} // namespace logic
