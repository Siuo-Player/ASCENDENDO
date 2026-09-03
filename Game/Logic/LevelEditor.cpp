#include "Logic/LevelEditor.h"
#include "Core/Config.h"

#include <algorithm>
#include <cmath>

namespace logic {

namespace {
constexpr float EPS = 0.0001f;

float snapScalar(float value) {
    const float grid = config::EDITOR_GRID_SNAP;
    return std::round(value / grid) * grid;
}

float ceilToGrid(float value) {
    const float grid = config::EDITOR_GRID_SNAP;
    return std::ceil(value / grid) * grid;
}

float floorToGrid(float value) {
    const float grid = config::EDITOR_GRID_SNAP;
    return std::floor(value / grid) * grid;
}

bool hasMinimumSize(const AABB& rect) {
    return rect.width() >= config::EDITOR_GRID_SNAP - EPS &&
           rect.height() >= config::EDITOR_GRID_SNAP - EPS;
}
}

LevelEditorDocument::LevelEditorDocument(bool finalCampaignLevel,
                                         const AABB& initialGround)
    : m_finalCampaignLevel(finalCampaignLevel),
      m_initialGround(initialGround) {
    m_spawnMinX = ceilToGrid(initialGround.min.x);
    m_spawnMaxX = floorToGrid(initialGround.max.x - config::PLAYER_WIDTH);
    if (m_spawnMaxX < m_spawnMinX) m_spawnMaxX = m_spawnMinX;

    m_spawnPosition = {
        m_spawnMinX,
        snapScalar(initialGround.max.y),
    };
}

void LevelEditorDocument::bumpGeneration() {
    ++m_generation;
}

float LevelEditorDocument::snap(float value) {
    return snapScalar(value);
}

Vec2 LevelEditorDocument::snap(const Vec2& point) {
    return {snapScalar(point.x), snapScalar(point.y)};
}

AABB LevelEditorDocument::snap(const AABB& rect) {
    return {snap(rect.min), snap(rect.max)};
}

bool LevelEditorDocument::insideLogicalBounds(const AABB& rect) const {
    return rect.min.x >= -EPS &&
           rect.min.y >= -EPS &&
           rect.max.x <= config::LOGICAL_WIDTH + EPS &&
           rect.max.y <= config::LOGICAL_HEIGHT + EPS;
}

bool LevelEditorDocument::validPlatform(const AABB& rect) const {
    return insideLogicalBounds(rect) && hasMinimumSize(rect);
}

bool LevelEditorDocument::addPlatform(const AABB& requested,
                                      std::size_t* createdIndex) {
    if (!insideLogicalBounds(requested) || !hasMinimumSize(requested)) return false;

    const AABB rect = snap(requested);
    if (!validPlatform(rect)) return false;

    m_platforms.push_back({rect});
    if (createdIndex) *createdIndex = m_platforms.size() - 1;
    bumpGeneration();
    return true;
}

bool LevelEditorDocument::movePlatform(std::size_t index,
                                       const Vec2& requestedMin) {
    if (index >= m_platforms.size()) return false;

    const AABB old = m_platforms[index].bounds;
    const AABB requested = {
        requestedMin,
        {requestedMin.x + old.width(), requestedMin.y + old.height()},
    };
    if (!insideLogicalBounds(requested) || !hasMinimumSize(requested)) return false;

    const Vec2 newMin = snap(requestedMin);
    const AABB moved = {
        newMin,
        {newMin.x + old.width(), newMin.y + old.height()},
    };

    if (!validPlatform(moved)) return false;
    if (moved.min.x == old.min.x && moved.min.y == old.min.y &&
        moved.max.x == old.max.x && moved.max.y == old.max.y)
        return true;

    m_platforms[index].bounds = moved;
    bumpGeneration();
    return true;
}

bool LevelEditorDocument::removePlatform(std::size_t index) {
    if (index >= m_platforms.size()) return false;
    m_platforms.erase(m_platforms.begin() + static_cast<std::ptrdiff_t>(index));
    bumpGeneration();
    return true;
}

bool LevelEditorDocument::setSpawnX(float requestedX) {
    if (requestedX < m_spawnMinX - EPS || requestedX > m_spawnMaxX + EPS) return false;

    const float snappedX = snapScalar(requestedX);
    if (snappedX < m_spawnMinX - EPS || snappedX > m_spawnMaxX + EPS) return false;
    if (snappedX == m_spawnPosition.x) return true;

    m_spawnPosition.x = snappedX;
    m_spawnPosition.y = snapScalar(m_initialGround.max.y);
    bumpGeneration();
    return true;
}

bool LevelEditorDocument::validFlag(const AABB& rect) const {
    return insideLogicalBounds(rect) && hasMinimumSize(rect);
}

bool LevelEditorDocument::setFlag(const AABB& requested) {
    if (!m_finalCampaignLevel) return false;
    if (!insideLogicalBounds(requested) || !hasMinimumSize(requested)) return false;

    const AABB rect = snap(requested);
    if (!validFlag(rect)) return false;
    if (m_flag &&
        m_flag->min.x == rect.min.x && m_flag->min.y == rect.min.y &&
        m_flag->max.x == rect.max.x && m_flag->max.y == rect.max.y)
        return true;

    m_flag = rect;
    bumpGeneration();
    return true;
}

void LevelEditorDocument::removeFlag() {
    if (!m_flag) return;
    m_flag.reset();
    bumpGeneration();
}

LevelData LevelEditorDocument::toLevelData(const std::string& name) const {
    LevelData data;
    data.name = name;
    data.spawnPosition = m_spawnPosition;

    // The initial ground is implicit in the editor document but was historically
    // materialized by saveEditorLevel(). Keep it materialized in LevelData too.
    data.platforms.push_back(m_initialGround);
    data.platforms.reserve(m_platforms.size() + 1);
    for (const auto& platform : m_platforms) {
        data.platforms.push_back(platform.bounds);
    }

    if (m_flag) data.flag = *m_flag;
    return data;
}

Vec2 LevelEditorDocument::presetSize(EditorSizePreset preset) {
    switch (preset) {
        case EditorSizePreset::SMALL: return {64.0f, 16.0f};
        case EditorSizePreset::MEDIUM: return {128.0f, 20.0f};
        case EditorSizePreset::LARGE: return {192.0f, 24.0f};
    }
    return {128.0f, 20.0f};
}

} // namespace logic