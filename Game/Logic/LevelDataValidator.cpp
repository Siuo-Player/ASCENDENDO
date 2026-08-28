#include "Logic/LevelDataValidator.h"

#include <cmath>

namespace logic {

namespace {

bool hasFiniteCoordinates(const AABB& bounds) noexcept {
    return std::isfinite(bounds.min.x) && std::isfinite(bounds.min.y) &&
           std::isfinite(bounds.max.x) && std::isfinite(bounds.max.y);
}

bool hasPositiveExtent(const AABB& bounds) noexcept {
    return bounds.width() > 0.0f && bounds.height() > 0.0f;
}

bool isValidGeometry(const AABB& bounds) noexcept {
    return hasFiniteCoordinates(bounds) && hasPositiveExtent(bounds);
}

} // namespace

bool LevelDataValidator::validate(const LevelData& data) noexcept {
    for (const auto& platform : data.platforms) {
        if (!isValidGeometry(platform)) return false;
    }

    return !data.flag || isValidGeometry(*data.flag);
}

} // namespace logic
