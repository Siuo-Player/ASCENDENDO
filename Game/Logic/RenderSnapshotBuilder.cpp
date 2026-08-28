#include "Logic/RenderSnapshotBuilder.h"

#include "Logic/Level.h"
#include "Logic/Player.h"

namespace logic {
namespace {

core::RenderRect toRenderRect(const AABB& bounds) {
    return {
        bounds.min.x,
        bounds.min.y,
        bounds.width(),
        bounds.height()
    };
}

} // namespace

core::RenderSnapshot buildRenderSnapshot(const Player& player, const Level& level) {
    core::RenderSnapshot snapshot;

    snapshot.player.bounds = {
        player.position().x,
        player.position().y,
        player.body.width,
        player.body.height
    };
    snapshot.player.facingLeft = player.facingDirection < 0.0f;

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

} // namespace logic
