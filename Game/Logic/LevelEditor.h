#pragma once

#include "Logic/Physics.h"
#include "Logic/LevelData.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace logic {

enum class EditorToolMode {
    STAMP,
    DRAG,
};

enum class EditorSizePreset {
    SMALL,
    MEDIUM,
    LARGE,
};

struct EditorPlatform {
    AABB bounds;
};

class LevelEditorDocument {
public:
    LevelEditorDocument(bool finalCampaignLevel, const AABB& initialGround);

    bool isFinalCampaignLevel() const { return m_finalCampaignLevel; }

    static float snap(float value);
    static Vec2 snap(const Vec2& point);
    static AABB snap(const AABB& rect);

    bool addPlatform(const AABB& requested, std::size_t* createdIndex = nullptr);
    bool movePlatform(std::size_t index, const Vec2& requestedMin);
    bool removePlatform(std::size_t index);

    const std::vector<EditorPlatform>& platforms() const { return m_platforms; }
    std::size_t platformCount() const { return m_platforms.size(); }

    bool setSpawnX(float requestedX);
    Vec2 spawnPosition() const { return m_spawnPosition; }

    float spawnMinX() const { return m_spawnMinX; }
    float spawnMaxX() const { return m_spawnMaxX; }

    bool setFlag(const AABB& requested);
    void removeFlag() { m_flag.reset(); }
    bool hasFlag() const { return m_flag.has_value(); }
    const AABB* flag() const { return m_flag.has_value() ? &*m_flag : nullptr; }

    LevelData toLevelData(const std::string& name) const;

    static Vec2 presetSize(EditorSizePreset preset);

private:
    bool insideLogicalBounds(const AABB& rect) const;
    bool validPlatform(const AABB& rect) const;
    bool validFlag(const AABB& rect) const;

    bool m_finalCampaignLevel = false;
    AABB m_initialGround{};
    std::vector<EditorPlatform> m_platforms;
    Vec2 m_spawnPosition{};
    float m_spawnMinX = 0.0f;
    float m_spawnMaxX = 0.0f;
    std::optional<AABB> m_flag;
};

} // namespace logic
