#include "Logic/LevelDataValidator.h"

namespace logic {

namespace {

bool hasPositiveExtent(const AABB& bounds) noexcept {
    return bounds.width() > 0.0f && bounds.height() > 0.0f;
}

} // namespace

bool LevelDataValidator::validate(const LevelData& data) noexcept {
    for (const auto& platform : data.platforms) {
        if (!hasPositiveExtent(platform)) return false;
    }

    return !data.flag || hasPositiveExtent(*data.flag);
}

} // namespace logic
