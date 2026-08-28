#pragma once

#include "Core/RenderSnapshot.h"

namespace logic {
class Player;
class Level;

// Converts mutable simulation state into the immutable data shape consumed by
// the presentation world pass. No gameplay or physics decisions are made here.
core::RenderSnapshot buildRenderSnapshot(const Player& player, const Level& level);

} // namespace logic
