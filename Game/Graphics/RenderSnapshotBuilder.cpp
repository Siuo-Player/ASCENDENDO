#include "Graphics/RenderSnapshotBuilder.h"

#include "Graphics/PlatformPresentationRasterizer.h"
#include "Logic/Level.h"
#include "Logic/Player.h"

#include <vector>

namespace gfx {
namespace {

RenderRect toRenderRect(const logic::AABB& bounds) {
    return {
        bounds.min.x,
        bounds.min.y,
        bounds.width(),
        bounds.height()
    };
}

} // namespace

RenderSnapshot buildRenderSnapshot(const logic::Player& player,
                                   const logic::Level& level) {
    RenderSnapshot snapshot;

    snapshot.player.bounds = {
        player.position().x,
        player.position().y,
        player.body.width,
        player.body.height
    };
    snapshot.player.facingDirection = player.facingDirection;

    const auto& platforms = level.platforms();
    snapshot.platforms.reserve(platforms.size());

    std::vector<logic::AABB> platformBounds;
    platformBounds.reserve(platforms.size());
    for (const auto& platform : platforms) {
        snapshot.platforms.push_back(toRenderRect(platform.bounds));
        platformBounds.push_back(platform.bounds);
    }

    const auto rasterized = presentation::rasterizePlatforms(platformBounds);
    if (rasterized.valid) {
        std::vector<compositor::RegionCompositionResult> composedRegions;
        composedRegions.reserve(rasterized.regions.size());

        bool valid = true;
        for (const auto& region : rasterized.regions) {
            const auto composed = compositor::composeRegion(region);
            if (!composed.valid) {
                valid = false;
                break;
            }
            composedRegions.push_back(composed);
        }

        if (valid) {
            for (std::size_t lhs = 0; lhs < rasterized.regions.size(); ++lhs) {
                for (std::size_t rhs = lhs + 1; rhs < rasterized.regions.size(); ++rhs) {
                    const auto contacts = compositor::findRegionContacts(
                        rasterized.regions[lhs], rasterized.regions[rhs]);
                    if (!compositor::applyRegionContacts(
                            composedRegions[lhs], composedRegions[rhs], contacts)) {
                        valid = false;
                        break;
                    }
                }
                if (!valid) break;
            }
        }

        if (valid) {
            std::size_t cellCount = 0;
            for (const auto& region : composedRegions) cellCount += region.cells.size();
            snapshot.semanticPlatformCells.reserve(cellCount);
            for (const auto& region : composedRegions) {
                snapshot.semanticPlatformCells.insert(
                    snapshot.semanticPlatformCells.end(),
                    region.cells.begin(),
                    region.cells.end());
            }
            snapshot.semanticPlatformsValid = true;
        }
    }

    snapshot.flag.visible = level.hasFlag;
    if (level.hasFlag) {
        snapshot.flag.bounds = toRenderRect(level.flagBounds);
    }

    return snapshot;
}

} // namespace gfx
