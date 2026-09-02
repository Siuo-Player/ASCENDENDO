// =============================================================================
// Game/Graphics/PlatformPresentationRasterizer.cpp
// =============================================================================
#include "Graphics/PlatformPresentationRasterizer.h"

#include <cmath>

namespace gfx::presentation {
namespace {

bool modularPositive(float value) {
    if (!std::isfinite(value) || value <= 0.0f)
        return false;

    const float quotient = value / static_cast<float>(compositor::CELL_SIZE);
    const float rounded = std::round(quotient);
    return rounded >= 1.0f && std::fabs(quotient - rounded) <= 1.0e-5f;
}

} // namespace

RasterizedPlatformRegions rasterizePlatforms(
    std::span<const logic::AABB> platforms,
    std::uint16_t material) {
    RasterizedPlatformRegions result;
    result.regions.reserve(platforms.size());

    for (const logic::AABB& platform : platforms) {
        const float width = platform.width();
        const float height = platform.height();
        if (!std::isfinite(platform.min.x) || !std::isfinite(platform.min.y) ||
            !modularPositive(width) || !modularPositive(height)) {
            result.valid = false;
            result.regions.clear();
            return result;
        }

        result.regions.push_back({
            platform.min.x,
            platform.min.y,
            width,
            height,
            material,
        });
    }

    return result;
}

RasterizedPlatformRegions rasterizePlatforms(
    const logic::LevelData& level,
    std::uint16_t material) {
    return rasterizePlatforms(level.platforms, material);
}

} // namespace gfx::presentation
