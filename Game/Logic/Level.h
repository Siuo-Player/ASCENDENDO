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
    std::string name = "Sem Nome";
    bool hasFlag = false;
    AABB flagBounds;
    Vec2 spawnPosition = {config::LOGICAL_WIDTH / 2.0f, 40.0f};

    // Append one local LevelData chunk into world coordinates.
    // Returns offsetY + LOGICAL_HEIGHT so CampaignRuntime can stream fixed-height chunks.
    float appendFromData(const LevelData& data, float maxWidth, float offsetY);

    void addPlatform(float x, float y, float w, float h);
    bool resolveCollision(PhysicsBody& body) const;

    const std::vector<Platform>& platforms() const { return m_platforms; }
    int platformCount() const { return static_cast<int>(m_platforms.size()); }

    void clear() {
        m_platforms.clear();
        name = "Sem Nome";
        hasFlag = false;
        flagBounds = {};
        spawnPosition = {config::LOGICAL_WIDTH / 2.0f, 40.0f};
        m_spawnInitialized = false;
    }

private:
    static constexpr float SKIN_WIDTH = 1.0f;
    std::vector<Platform> m_platforms;
    bool m_spawnInitialized = false;
};

} // namespace logic
