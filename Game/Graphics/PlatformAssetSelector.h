#pragma once
// =============================================================================
// Game/Graphics/PlatformAssetSelector.h
//
// Deterministic selection contract for reviewed platform-art candidates.
// This component does not load files, inspect pixels or approve assets.
// =============================================================================

#include "Graphics/PlatformCompositor.h"

#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace gfx::assets {

struct PlatformAssetCandidate {
    std::string assetId;
    std::uint16_t topologyMask = 0;
    int widthCells = 1;
    int heightCells = 1;
    std::optional<std::uint16_t> material;
    bool flipAllowed = false;
    int scale = 1;
    int variantRank = 0;

    // Hard-gate evidence supplied by the asset registry/review process.
    bool provenanceVerified = false;
    bool pixelScaleSafe = false;
    bool contactReadable = false;
    bool gameplayDecoupled = false;
    bool seamsAcceptable = false;
};

struct PlatformAssetRequest {
    compositor::TopologyClass topology = compositor::TopologyClass::Isolated;
    int widthCells = 1;
    int heightCells = 1;
    std::uint16_t material = 0;
    bool mirrored = false;
    int scale = 1;
};

// Build the canonical selector request for one compositor-produced cell.
// World position and neighbour masks are intentionally not copied: asset
// selection is a topology/footprint/material presentation concern.
PlatformAssetRequest makePlatformAssetRequest(
    const compositor::RegionCell& cell,
    bool mirrored = false,
    int scale = 1);

// Select the highest-ranked eligible candidate using only documented metadata.
// Returns std::nullopt when no candidate passes every hard constraint.
std::optional<std::string> selectBestPlatformAsset(
    std::span<const PlatformAssetCandidate> candidates,
    const PlatformAssetRequest& request);

std::uint16_t topologyBit(compositor::TopologyClass topology);

} // namespace gfx::assets
