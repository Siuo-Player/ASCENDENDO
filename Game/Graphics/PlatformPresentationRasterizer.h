#pragma once
// =============================================================================
// Game/Graphics/PlatformPresentationRasterizer.h
//
// Presentation-only bridge from declarative LevelData platform rectangles to
// semantic 16x16 compositor regions. It never mutates gameplay data.
// =============================================================================

#include "Graphics/PlatformCompositor.h"
#include "Logic/LevelData.h"

#include <cstdint>
#include <span>
#include <vector>

namespace gfx::presentation {

struct RasterizedPlatformRegions {
    bool valid = true;
    std::vector<compositor::PlatformRegion> regions;
};

// Convert platform AABBs into compositor regions while preserving their exact
// continuous world-space origin. A non-modular footprint fails closed instead
// of snapping, cropping or otherwise changing gameplay geometry.
RasterizedPlatformRegions rasterizePlatforms(
    std::span<const logic::AABB> platforms,
    std::uint16_t material = 0);

// Convenience bridge for the declarative level model. Optional spawn/flag data
// are intentionally ignored because this boundary covers platform presentation
// only.
RasterizedPlatformRegions rasterizePlatforms(
    const logic::LevelData& level,
    std::uint16_t material = 0);

} // namespace gfx::presentation
