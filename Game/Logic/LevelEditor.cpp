#include "Logic/LevelEditor.h"
#include "Logic/Level.h"
#include "Core/Config.h"

#include <algorithm>
#include <cmath>

namespace logic {

namespace {
constexpr float EPS = 0.0001f;
constexpr float MIN_PLATFORM_WIDTH = 4.0f;
constexpr float MIN_PLATFORM_HEIGHT = 4.0f;
constexpr float AUTO_FLAG_HEIGHT = 40.0f;

bool hasMinimumSize(const AABB& rect) {
    return rect.width() >= MIN_PLATFORM_WIDTH - EPS &&
           rect.height() >= MIN_PLATFORM_HEIGHT - EPS;
}

bool insideLayoutBounds(const AABB& rect, const core::LevelLayout& layout) {
    return std::isfinite(rect.min.x) && std::isfinite(rect.min.y) &&
           std::isfinite(rect.max.x) && std::isfinite(rect.max.y) &&
           rect.min.x >= -EPS &&
           rect.min.y >= Level::AUTO_GROUND_HEIGHT - EPS &&
           rect.max.x <= layout.width() + EPS &&
           rect.max.y <= layout.height() + EPS;
}
}

LevelEditorDocument::LevelEditorDocument(bool finalCampaignLevel,
                                         const AABB& initialGround,
                                         std::size_t screenCount)
    : m_finalCampaignLevel(finalCampaignLevel),
      m_initialGround({{0.0f, 0.0f},
                       {config::LOGICAL_WIDTH, Level::AUTO_GROUND_HEIGHT}}),
      m_layout(screenCount) {
    (void)initialGround;
    m_spawnMinX = Level::AUTO_GROUND_SPAWN_X;
    m_spawnMaxX = Level::AUTO_GROUND_SPAWN_X;
    m_spawnPosition = {
        Level::AUTO_GROUND_SPAWN_X,
        Level::AUTO_GROUND_HEIGHT,
    };
    refreshAutomaticFlag();
}

void LevelEditorDocument::bumpGeneration() {
    ++m_generation;
}

void LevelEditorDocument::refreshAutomaticFlag() {
    m_flag.reset();
    if (!m_finalCampaignLevel || m_platforms.empty()) return;

    const AABB* highest = &m_platforms.front().bounds;
    for (const auto& platform : m_platforms) {
        const AABB& candidate = platform.bounds;
        if (candidate.max.y > highest->max.y ||
            (candidate.max.y == highest->max.y && candidate.width() > highest->width()) ||
            (candidate.max.y == highest->max.y && candidate.width() == highest->width() &&
             candidate.min.x < highest->min.x)) {
            highest = &candidate;
        }
    }

    m_flag = AABB{
        {highest->min.x, highest->max.y},
        {highest->max.x, highest->max.y + AUTO_FLAG_HEIGHT}
    };
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
    refreshAutomaticFlag();
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
    refreshAutomaticFlag();
    bumpGeneration();
    return true;
}

bool LevelEditorDocument::removePlatform(std::size_t index) {
    if (index >= m_platforms.size()) return false;
    m_platforms.erase(m_platforms.begin() + static_cast<std::ptrdiff_t>(index));
    refreshAutomaticFlag();
    bumpGeneration();
    return true;
}

bool LevelEditorDocument::setSpawnX(float requestedX) {
    (void)requestedX;
    return false;
}

bool LevelEditorDocument::validFlag(const AABB& rect) const {
    (void)rect;
    return false;
}

bool LevelEditorDocument::setFlag(const AABB& requested) {
    (void)requested;
    return false;
}

void LevelEditorDocument::removeFlag() {
    // Automatic campaign goal cannot be manually removed.
}

LevelData LevelEditorDocument::toLevelData(const std::string& name) const {
    LevelData data;
    data.name = name;
    data.screenCount = m_layout.screenCount();
    data.finalCampaignLevel = m_finalCampaignLevel;
    data.platforms.reserve(m_platforms.size());
    for (const auto& platform : m_platforms)
        data.platforms.push_back(platform.bounds);
    // Ground, spawn and campaign goal are derived and never serialized.
    return data;
}

bool LevelEditorDocument::restoreFromLevelData(const LevelData& data) {
    if (data.screenCount == 0 || data.flag.has_value() || data.spawnPosition.has_value())
        return false;

    const core::LevelLayout restoredLayout(data.screenCount);
    for (const AABB& platform : data.platforms) {
        if (!insideLayoutBounds(platform, restoredLayout) || !hasMinimumSize(platform))
            return false;
    }

    m_initialGround = {
        {0.0f, 0.0f},
        {config::LOGICAL_WIDTH, Level::AUTO_GROUND_HEIGHT}
    };
    m_platforms.clear();
    m_platforms.reserve(data.platforms.size());
    for (const AABB& platform : data.platforms)
        m_platforms.push_back({platform});

    m_layout = restoredLayout;
    m_spawnMinX = Level::AUTO_GROUND_SPAWN_X;
    m_spawnMaxX = Level::AUTO_GROUND_SPAWN_X;
    m_spawnPosition = {
        Level::AUTO_GROUND_SPAWN_X,
        Level::AUTO_GROUND_HEIGHT,
    };
    refreshAutomaticFlag();
    bumpGeneration();
    return true;
}

bool LevelEditorDocument::restoreFromLevelData(const LevelData& data,
                                               bool finalCampaignLevel) {
    const bool previousFinal = m_finalCampaignLevel;
    m_finalCampaignLevel = finalCampaignLevel;
    if (restoreFromLevelData(data)) return true;
    m_finalCampaignLevel = previousFinal;
    refreshAutomaticFlag();
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
