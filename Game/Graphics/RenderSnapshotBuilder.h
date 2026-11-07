#pragma once

#include "Graphics/RenderSnapshot.h"

namespace logic {
class Player;
class Level;
}

namespace gfx {

// Boundary adapter: copies the small set of runtime state required by the
// world presentation pass into a standalone presentation value object.
// If ghostPlayer is provided and active, copies its position and orientation
// into the snapshot for ghost overlay rendering.
RenderSnapshot buildRenderSnapshot(const logic::Player& player,
                                   const logic::Level& level,
                                   const logic::Player* ghostPlayer = nullptr,
                                   const RenderHudSnapshot& hud = RenderHudSnapshot{});

} // namespace gfx
