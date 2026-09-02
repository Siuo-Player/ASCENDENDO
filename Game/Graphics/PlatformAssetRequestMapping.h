#pragma once

#include "Graphics/PlatformCompositor.h"
#include "Graphics/PlatformAssetSelector.h"

namespace gfx::assets {

// Build a selector request directly from compositor presentation semantics.
PlatformAssetRequest makePlatformAssetRequest(
    const compositor::RegionCell& cell,
    std::uint16_t material = 0,
    int scale = 1,
    bool mirrored = false);

} // namespace gfx::assets
