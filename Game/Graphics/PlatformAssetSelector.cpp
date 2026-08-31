// =============================================================================
// Game/Graphics/PlatformAssetSelector.cpp
// =============================================================================
#include "Graphics/PlatformAssetSelector.h"

#include <tuple>

namespace gfx::assets {
namespace {

int topologyMatchRank(const PlatformAssetCandidate& candidate,
                      const PlatformAssetRequest& request) {
    const std::uint16_t bit = topologyBit(request.topology);
    return (candidate.topologyMask & bit) != 0 ? 1 : 0;
}

int footprintMatchRank(const PlatformAssetCandidate& candidate,
                       const PlatformAssetRequest& request) {
    return candidate.widthCells == request.widthCells &&
                   candidate.heightCells == request.heightCells
               ? 1
               : 0;
}

int materialMatchRank(const PlatformAssetCandidate& candidate,
                      const PlatformAssetRequest& request) {
    return candidate.material.has_value() &&
                   *candidate.material == request.material
               ? 1
               : 0;
}

bool eligible(const PlatformAssetCandidate& candidate,
              const PlatformAssetRequest& request) {
    if (candidate.assetId.empty())
        return false;
    if (topologyMatchRank(candidate, request) == 0)
        return false;
    if (candidate.widthCells != request.widthCells ||
        candidate.heightCells != request.heightCells)
        return false;
    if (candidate.material.has_value() && *candidate.material != request.material)
        return false;
    if (request.mirrored && !candidate.flipAllowed)
        return false;
    if (candidate.scale != request.scale)
        return false;

    return candidate.provenanceVerified &&
           candidate.pixelScaleSafe &&
           candidate.contactReadable &&
           candidate.gameplayDecoupled &&
           candidate.seamsAcceptable;
}

bool better(const PlatformAssetCandidate& lhs,
            const PlatformAssetCandidate& rhs,
            const PlatformAssetRequest& request) {
    const auto lhsRank = std::tuple{
        topologyMatchRank(lhs, request),
        footprintMatchRank(lhs, request),
        materialMatchRank(lhs, request),
        lhs.variantRank,
    };
    const auto rhsRank = std::tuple{
        topologyMatchRank(rhs, request),
        footprintMatchRank(rhs, request),
        materialMatchRank(rhs, request),
        rhs.variantRank,
    };

    if (lhsRank != rhsRank)
        return lhsRank > rhsRank;
    return lhs.assetId < rhs.assetId;
}

} // namespace

std::uint16_t topologyBit(compositor::TopologyClass topology) {
    return static_cast<std::uint16_t>(1u << static_cast<unsigned>(topology));
}

std::optional<std::string> selectBestPlatformAsset(
    std::span<const PlatformAssetCandidate> candidates,
    const PlatformAssetRequest& request) {
    const PlatformAssetCandidate* best = nullptr;

    for (const auto& candidate : candidates) {
        if (!eligible(candidate, request))
            continue;
        if (best == nullptr || better(candidate, *best, request))
            best = &candidate;
    }

    if (best == nullptr)
        return std::nullopt;
    return best->assetId;
}

} // namespace gfx::assets
