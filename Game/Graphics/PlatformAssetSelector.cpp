// =============================================================================
// Game/Graphics/PlatformAssetSelector.cpp
// =============================================================================
#include "Graphics/PlatformAssetSelector.h"

#include <algorithm>
#include <cctype>
#include <string_view>
#include <tuple>

namespace gfx::assets {
namespace {

bool validRequest(const PlatformAssetRequest& request) {
    return request.widthCells > 0 && request.heightCells > 0 && request.scale > 0;
}

bool validSha256(const std::string& sha256) {
    if (sha256.size() != 64)
        return false;

    return std::all_of(sha256.begin(), sha256.end(), [](unsigned char c) {
        return std::isxdigit(c) != 0;
    });
}

bool validRuntimePath(std::string_view path) {
    if (path.empty())
        return false;

    // Runtime asset paths are repository-relative, portable across the
    // project's Windows/Linux builds, and must not contain traversal.
    if (path.front() == '/' || path.front() == '\\')
        return false;
    if (path.size() >= 2 && std::isalpha(static_cast<unsigned char>(path[0])) &&
        path[1] == ':')
        return false;

    std::size_t segmentStart = 0;
    for (std::size_t i = 0; i <= path.size(); ++i) {
        const bool separator = i == path.size() || path[i] == '/' || path[i] == '\\';
        if (!separator)
            continue;

        const std::size_t segmentLength = i - segmentStart;
        if (segmentLength == 0)
            return false;

        const std::string_view segment = path.substr(segmentStart, segmentLength);
        if (segment == "." || segment == "..")
            return false;

        segmentStart = i + 1;
    }

    return true;
}

bool exactFileIdentityPresent(const PlatformAssetCandidate& candidate) {
    return validRuntimePath(candidate.runtimePath) && validSha256(candidate.contentSha256);
}

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
    if (!validRequest(request))
        return false;
    if (candidate.assetId.empty())
        return false;
    if (!exactFileIdentityPresent(candidate))
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
           candidate.seamsAcceptable &&
           candidate.humanApproved;
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

PlatformAssetRequest makePlatformAssetRequest(
    const compositor::RegionCell& cell,
    bool mirrored,
    int scale) {
    PlatformAssetRequest request{};
    request.topology = cell.topology;
    request.widthCells = 1;
    request.heightCells = 1;
    request.material = cell.material;
    request.mirrored = mirrored;
    request.scale = scale;
    return request;
}

std::uint16_t topologyBit(compositor::TopologyClass topology) {
    return static_cast<std::uint16_t>(1u << static_cast<unsigned>(topology));
}

std::optional<std::string> selectBestPlatformAsset(
    std::span<const PlatformAssetCandidate> candidates,
    const PlatformAssetRequest& request) {
    if (!validRequest(request))
        return std::nullopt;

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
