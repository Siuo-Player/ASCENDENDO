#include "Graphics/RenderSnapshotBuilder.h"

#include "Logic/Level.h"
#include "Logic/Player.h"

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
    for (const auto& platform : platforms) {
        snapshot.platforms.push_back(toRenderRect(platform.bounds));
    }

    snapshot.flag.visible = level.hasFlag;
    if (level.hasFlag) {
        snapshot.flag.bounds = toRenderRect(level.flagBounds);
    }

    return snapshot;
}

} // namespace gfx
