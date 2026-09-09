#include "Logic/LevelEditor.h"
#include "Core/Config.h"

#include <algorithm>
#include <cmath>

namespace logic {

namespace {
constexpr float EPS = 0.0001f;
constexpr float MIN_PLATFORM_WIDTH = 4.0f;
constexpr float MIN_PLATFORM_HEIGHT = 4.0f;

bool hasMinimumSize(const AABB& rect) {
    return rect.width() >= MIN_PLATFORM_WIDTH - EPS &&
           rect.height() >= MIN_PLATFORM_HEIGHT - EPS;
}

bool insideLayoutBounds(const AABB& rect, const core::LevelLayout& layout) {
    return std::isfinite(rect.min.x) && std::isfinite(rect.min.y) &&
           std::isfinite(rect.max.x) && std::isfinite(rect.max.y) &&
           rect.min.x >= -EPS &&
           rect.min.y >= -EPS &&
           rect.max.x <= layout.width() + EPS &&
           rect.max.y <= layout.height() + EPS;
}
}

LevelEditorDocument::LevelEditorDocument(bool finalCampaignLevel,
                                         const AABB& initialGround,
                                         std::size_t screenCount)
    : m_finalCampaignLevel(finalCampaignLevel),
      m_initialGround(initialGround),
      m_layout(screenCount) {
    m_spawnMinX = initialGround.min.x;
    m_spawnMaxX = std::max(m_spawnMinX, initialGround.max.x - config::PLAYER_WIDTH);

    m_spawnPosition = {
        m_spawnMinX,
        initialGround.max.y,
    };
}

void LevelEditorDocument::bumpGeneration() {
    ++m_generation;
}

bool LevelEditorDocument::insideLogicalBounds(const AABB& rect) const {
    return insideLayoutBounds(rect, m_layout);
}

bool LevelEditorDocument::inFinalScreen(const AABB& rect) const {
    const float finalScreenBottom = m_layout.screenBottomY(m_layout.screenCount() - 1);
    return rect.min.y >= finalScreenBottom - EPS;
}

bool LevelEditorDocument::validPlatform(const AABB& rect) const {
    return insideLogicalBounds(rect) && hasMinimumSize(rect);
}

bool LevelEditorDocument::addPlatform(const AABB& requested,
                                      std::size_t* createdIndex) {
    if (!validPlatform(requested)) return false;

    m_platforms.push_back({requested});
    if (createdIndex) *createdIndex = m_platforms.size() - 1;
    bumpGeneration();
    return true;
}

bool LevelEditorDocument::movePlatform(std::size_t index,
                                       const Vec2& requestedMin) {
    if (index >= m_platforms.size()) return false;
    if (!std::isfinite(requestedMin.x) || !std::isfinite(requestedMin.y)) return false;

    const AABB old = m_platforms[index].bounds;
    const AABB moved = {
        requestedMin,
        {requestedMin.x + old.width(), requestedMin.y + old.height()},
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
    if (!std::isfinite(requestedX) ||
        requestedX < m_spawnMinX - EPS ||
        requestedX > m_spawnMaxX + EPS)
        return false;

    if (requestedX == m_spawnPosition.x) return true;

    m_spawnPosition.x = requestedX;
    m_spawnPosition.y = m_initialGround.max.y;
    bumpGeneration();
    return true;
}

bool LevelEditorDocument::validFlag(const AABB& rect) const {
    return insideLogicalBounds(rect) && hasMinimumSize(rect) && inFinalScreen(rect);
}

bool LevelEditorDocument::setFlag(const AABB& requested) {
    if (!m_finalCampaignLevel || !validFlag(requested)) return false;

    if (m_flag &&
        m_flag->min.x == requested.min.x && m_flag->min.y == requested.min.y &&
        m_flag->max.x == requested.max.x && m_flag->max.y == requested.max.y)
        return true;

    m_flag = requested;
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
    data.screenCount = m_layout.screenCount();

    // The initial ground is implicit in the editor document but remains
    // materialized in LevelData for backwards-compatible serialization.
    data.platforms.push_back(m_initialGround);
    data.platforms.reserve(m_platforms.size() + 1);
    for (const auto& platform : m_platforms) {
        data.platforms.push_back(platform.bounds);
    }

    if (m_flag) data.flag = *m_flag;
    return data;
}

bool LevelEditorDocument::restoreFromLevelData(const LevelData& data) {
    if (data.platforms.empty() || data.screenCount == 0) return false;

    const core::LevelLayout restoredLayout(data.screenCount);
    for (const AABB& platform : data.platforms) {
        if (!insideLayoutBounds(platform, restoredLayout) || !hasMinimumSize(platform)) return false;
    }

    if (data.spawnPosition &&
        (!std::isfinite(data.spawnPosition->x) || !std::isfinite(data.spawnPosition->y) ||
         data.spawnPosition->x < 0.0f ||
         data.spawnPosition->x > restoredLayout.width()))
        return false;

    if (data.flag &&
        (!m_finalCampaignLevel ||
         !insideLayoutBounds(*data.flag, restoredLayout) ||
         !hasMinimumSize(*data.flag) ||
         data.flag->min.y < restoredLayout.screenBottomY(restoredLayout.screenCount() - 1)))
        return false;

    m_initialGround = data.platforms.front();
    m_platforms.clear();
    m_platforms.reserve(data.platforms.size() - 1);
    for (std::size_t i = 1; i < data.platforms.size(); ++i) {
        m_platforms.push_back({data.platforms[i]});
    }

    m_layout = restoredLayout;
    m_spawnMinX = m_initialGround.min.x;
    m_spawnMaxX = std::max(m_spawnMinX, m_initialGround.max.x - config::PLAYER_WIDTH);
    m_spawnPosition = data.spawnPosition.value_or(Vec2{
        m_spawnMinX,
        m_initialGround.max.y,
    });
    m_flag = data.flag;
    bumpGeneration();
    return true;
}

bool LevelEditorDocument::restoreFromLevelData(const LevelData& data,
                                               bool finalCampaignLevel) {
    const bool previousFinal = m_finalCampaignLevel;
    m_finalCampaignLevel = finalCampaignLevel;
    if (restoreFromLevelData(data)) return true;
    m_finalCampaignLevel = previousFinal;
    return false;
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
