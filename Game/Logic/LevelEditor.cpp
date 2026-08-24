// =============================================================================
//  Game/Logic/LevelEditor.cpp
// =============================================================================

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
}

LevelEditorDocument::LevelEditorDocument(bool finalCampaignLevel,
                                         const AABB& initialGround)
    : m_finalCampaignLevel(finalCampaignLevel),
      m_initialGround(initialGround) {
    // O spawn ocupa o topo do chão inicial. Os limites são arredondados para
    // dentro do chão, nunca para fora dele.
    m_spawnMinX = ceilToGrid(initialGround.min.x);
    m_spawnMaxX = floorToGrid(initialGround.max.x - config::PLAYER_WIDTH);
    if (m_spawnMaxX < m_spawnMinX) m_spawnMaxX = m_spawnMinX;

    m_spawnPosition = {
        m_spawnMinX,
        snapScalar(initialGround.max.y),
    };
}

float LevelEditorDocument::snap(float value) {
    return snapScalar(value);
}

Vec2 LevelEditorDocument::snap(const Vec2& point) {
    return { snapScalar(point.x), snapScalar(point.y) };
}

AABB LevelEditorDocument::snap(const AABB& rect) {
    return { snap(rect.min), snap(rect.max) };
}

bool LevelEditorDocument::insideLogicalBounds(const AABB& rect) const {
    return rect.min.x >= -EPS &&
           rect.min.y >= -EPS &&
           rect.max.x <= config::LOGICAL_WIDTH + EPS &&
           rect.max.y <= config::LOGICAL_HEIGHT + EPS;
}

bool LevelEditorDocument::validPlatform(const AABB& rect) const {
    if (!insideLogicalBounds(rect)) return false;
    return rect.width() >= config::EDITOR_GRID_SNAP - EPS &&
           rect.height() >= config::EDITOR_GRID_SNAP - EPS;
}

bool LevelEditorDocument::addPlatform(const AABB& requested,
                                      std::size_t* createdIndex) {
    // Uma operação que começa fora do canvas é inválida mesmo que o snap
    // posterior pudesse trazê-la de volta para dentro. A UI, portanto,
    // pode simplesmente não oferecer esse estado ao utilizador.
    if (!insideLogicalBounds(requested)) return false;

    const AABB rect = snap(requested);
    if (!validPlatform(rect)) return false;

    m_platforms.push_back({rect});
    if (createdIndex) *createdIndex = m_platforms.size() - 1;
    return true;
}

bool LevelEditorDocument::movePlatform(std::size_t index,
                                       const Vec2& requestedMin) {
    if (index >= m_platforms.size()) return false;

    const AABB old = m_platforms[index].bounds;
    const AABB requested = {
        requestedMin,
        { requestedMin.x + old.width(), requestedMin.y + old.height() },
    };
    if (!insideLogicalBounds(requested)) return false;

    const Vec2 newMin = snap(requestedMin);
    const AABB moved = {
        newMin,
        { newMin.x + old.width(), newMin.y + old.height() },
    };

    if (!validPlatform(moved)) return false;
    m_platforms[index].bounds = moved;
    return true;
}

bool LevelEditorDocument::removePlatform(std::size_t index) {
    if (index >= m_platforms.size()) return false;
    m_platforms.erase(m_platforms.begin() + static_cast<std::ptrdiff_t>(index));
    return true;
}

bool LevelEditorDocument::setSpawnX(float requestedX) {
    if (requestedX < m_spawnMinX - EPS || requestedX > m_spawnMaxX + EPS) return false;

    const float snappedX = snapScalar(requestedX);
    if (snappedX < m_spawnMinX - EPS || snappedX > m_spawnMaxX + EPS) return false;

    m_spawnPosition.x = snappedX;
    // O Y nunca é controlado pelo utilizador.
    m_spawnPosition.y = snapScalar(m_initialGround.max.y);
    return true;
}

bool LevelEditorDocument::validFlag(const AABB& rect) const {
    return insideLogicalBounds(rect) &&
           rect.width() >= config::EDITOR_GRID_SNAP - EPS &&
           rect.height() >= config::EDITOR_GRID_SNAP - EPS;
}

bool LevelEditorDocument::setFlag(const AABB& requested) {
    if (!m_finalCampaignLevel) return false;
    if (!insideLogicalBounds(requested)) return false;

    const AABB rect = snap(requested);
    if (!validFlag(rect)) return false;
    m_flag = rect;
    return true;
}

Vec2 LevelEditorDocument::presetSize(EditorSizePreset preset) {
    switch (preset) {
        case EditorSizePreset::SMALL:  return { 64.0f, 16.0f };
        case EditorSizePreset::MEDIUM: return { 128.0f, 20.0f };
        case EditorSizePreset::LARGE:  return { 192.0f, 24.0f };
    }
    return { 128.0f, 20.0f };
}

} // namespace logic
