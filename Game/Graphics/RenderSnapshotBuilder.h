#pragma once

#include "Graphics/RenderSnapshot.h"

namespace logic {
class Player;
class Level;
}

namespace gfx {

// Boundary adapter: copies the small set of runtime state required by the
// world presentation pass into a standalone presentation value object.
RenderSnapshot buildRenderSnapshot(const logic::Player& player,
                                   const logic::Level& level);

} // namespace gfx
