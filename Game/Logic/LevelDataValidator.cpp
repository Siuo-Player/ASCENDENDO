#include "Logic/LevelDataValidator.h"

#include "Core/Config.h"

#include <cmath>
#include <limits>

namespace logic {

namespace {

bool hasFiniteCoordinates(const AABB& bounds) noexcept {
    return std::isfinite(bounds.min.x) && std::isfinite(bounds.min.y) &&
           std::isfinite(bounds.max.x) && std::isfinite(bounds.max.y);
}

bool hasPositiveExtent(const AABB& bounds) noexcept {
    return bounds.width() > 0.0f && bounds.height() > 0.0f;
}

bool isValidGeometry(const AABB& bounds, double levelHeight) noexcept {
    return hasFiniteCoordinates(bounds) && hasPositiveExtent(bounds) &&
           bounds.min.x >= 0.0f && bounds.max.x <= config::LOGICAL_WIDTH &&
           bounds.min.y >= 0.0f && static_cast<double>(bounds.max.y) <= levelHeight;
}

bool hasValidSpawn(const Vec2& spawn, double levelHeight) noexcept {
    return std::isfinite(spawn.x) && std::isfinite(spawn.y) &&
           spawn.x >= 0.0f && spawn.x <= config::LOGICAL_WIDTH &&
           spawn.y >= 0.0f && static_cast<double>(spawn.y) <= levelHeight;
}

bool hasValidScreenCount(std::size_t screenCount, double& levelHeight) noexcept {
    if (screenCount == 0) return false;

    constexpr std::size_t screenHeight =
        static_cast<std::size_t>(config::LOGICAL_HEIGHT);
    if (screenCount > std::numeric_limits<std::size_t>::max() / screenHeight) {
        return false;
    }

    levelHeight = static_cast<double>(screenCount) *
                  static_cast<double>(config::LOGICAL_HEIGHT);
    return std::isfinite(levelHeight) &&
           levelHeight <= static_cast<double>(std::numeric_limits<float>::max());
}

} // namespace

bool LevelDataValidator::validate(const LevelData& data) noexcept {
    double levelHeight = 0.0;
    if (!hasValidScreenCount(data.screenCount, levelHeight)) return false;

    for (const auto& platform : data.platforms) {
        if (!isValidGeometry(platform, levelHeight)) return false;
    }

    if (data.flag && !isValidGeometry(*data.flag, levelHeight)) return false;
    if (data.spawnPosition && !hasValidSpawn(*data.spawnPosition, levelHeight)) return false;

    return true;
}

} // namespace logic
