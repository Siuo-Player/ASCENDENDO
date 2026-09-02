// =============================================================================
// Game/Graphics/PlatformCompositor.cpp
// =============================================================================
#include "Graphics/PlatformCompositor.h"

#include <array>
#include <cmath>
#include <map>
#include <tuple>

namespace gfx::compositor {
namespace {

struct CellKey {
    int x;
    int y;

    friend bool operator<(const CellKey& lhs, const CellKey& rhs) {
        return std::tie(lhs.y, lhs.x) < std::tie(rhs.y, rhs.x);
    }
};

std::uint32_t topologyBit(TopologyClass topology) {
    return 1u << static_cast<std::uint8_t>(topology);
}

std::uint8_t neighbourMask(const std::map<CellKey, std::uint16_t>& cells,
                           const GridCell& cell) {
    std::uint8_t mask = None;
    const std::array<std::pair<int, int>, 4> offsets = {{
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    }};
    const std::array<std::uint8_t, 4> bits = {{Left, Right, Up, Down}};

    for (std::size_t i = 0; i < offsets.size(); ++i) {
        const CellKey key{cell.x + offsets[i].first, cell.y + offsets[i].second};
        if (cells.contains(key))
            mask = static_cast<std::uint8_t>(mask | bits[i]);
    }

    return mask;
}

TopologyClass classify(const std::map<CellKey, std::uint16_t>& cells,
                       const GridCell& cell,
                       std::uint8_t mask) {
    const int degree = ((mask & Left) != 0) + ((mask & Right) != 0) +
                       ((mask & Up) != 0) + ((mask & Down) != 0);

    bool materialBoundary = false;
    const std::array<std::pair<int, int>, 4> offsets = {{
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    }};
    const std::array<std::uint8_t, 4> bits = {{Left, Right, Up, Down}};

    for (std::size_t i = 0; i < offsets.size(); ++i) {
        if ((mask & bits[i]) == 0)
            continue;

        const CellKey neighbour{cell.x + offsets[i].first,
                                cell.y + offsets[i].second};
        if (cells.at(neighbour) != cell.material) {
            materialBoundary = true;
            break;
        }
    }

    if (materialBoundary)
        return TopologyClass::MaterialBoundary;

    if (degree == 0)
        return TopologyClass::Isolated;

    if (degree >= 3)
        return TopologyClass::Junction;

    if ((mask & (Left | Right)) == (Left | Right) &&
        (mask & (Up | Down)) == 0)
        return TopologyClass::Interior;

    if ((mask & Left) != 0 && degree == 1)
        return TopologyClass::RightEnd;

    if ((mask & Right) != 0 && degree == 1)
        return TopologyClass::LeftEnd;

    if (degree == 2 &&
        ((mask & (Left | Right)) != 0) &&
        ((mask & (Up | Down)) != 0))
        return TopologyClass::Corner;

    return TopologyClass::VerticalEdge;
}

bool isModularDimension(float value, int& cellCount) {
    if (!std::isfinite(value) || value <= 0.0f)
        return false;

    const float quotient = value / static_cast<float>(CELL_SIZE);
    const float rounded = std::round(quotient);
    if (rounded < 1.0f || std::fabs(quotient - rounded) > 1.0e-5f)
        return false;

    cellCount = static_cast<int>(rounded);
    return cellCount > 0;
}

} // namespace

std::string_view selectCandidate(std::span<const AssetCandidate> candidates,
                                 const CandidateRequest& request,
                                 std::string_view fallbackId) {
    const std::uint32_t requestedTopology = topologyBit(request.topology);

    const AssetCandidate* best = nullptr;
    for (const AssetCandidate& candidate : candidates) {
        const bool topologyCovered = (candidate.topologyMask & requestedTopology) != 0u;
        const bool exactTopology = candidate.topologyMask == requestedTopology;
        const bool materialCompatible = candidate.material == ANY_MATERIAL ||
                                        candidate.material == request.material;
        const bool exactMaterial = candidate.material == request.material;

        if (candidate.semanticRole != request.semanticRole ||
            !topologyCovered ||
            !materialCompatible ||
            candidate.widthCells != request.widthCells ||
            candidate.heightCells != request.heightCells ||
            (request.flipRequired && !candidate.supportsFlip) ||
            candidate.scale != request.scale)
            continue;

        if (best == nullptr) {
            best = &candidate;
            continue;
        }

        const bool bestExactTopology = best->topologyMask == requestedTopology;
        const bool bestExactMaterial = best->material == request.material;
        const auto candidateRank = std::tuple{
            exactTopology ? 1 : 0,
            candidate.widthCells == request.widthCells &&
                candidate.heightCells == request.heightCells ? 1 : 0,
            exactMaterial ? 1 : 0,
            candidate.preferredRank,
            candidate.id,
        };
        const auto bestRank = std::tuple{
            bestExactTopology ? 1 : 0,
            best->widthCells == request.widthCells &&
                best->heightCells == request.heightCells ? 1 : 0,
            bestExactMaterial ? 1 : 0,
            best->preferredRank,
            best->id,
        };

        if (candidateRank < bestRank)
            continue;
        if (bestRank < candidateRank)
            best = &candidate;
    }

    return best != nullptr ? best->id : fallbackId;
}

CompositionResult compose(std::span<const GridCell> input) {
    CompositionResult result;

    std::map<CellKey, std::uint16_t> cells;
    for (const GridCell& cell : input) {
        const CellKey key{cell.x, cell.y};
        if (!cells.emplace(key, cell.material).second) {
            result.valid = false;
            result.cells.clear();
            return result;
        }
    }

    result.cells.reserve(input.size());
    for (const auto& [key, material] : cells) {
        const GridCell cell{key.x, key.y, material};
        const std::uint8_t mask = neighbourMask(cells, cell);
        result.cells.push_back({cell, mask, classify(cells, cell, mask)});
    }

    return result;
}

RegionCompositionResult composeRegion(const PlatformRegion& region) {
    RegionCompositionResult result;

    int widthCells = 0;
    int heightCells = 0;
    if (!std::isfinite(region.x) || !std::isfinite(region.y) ||
        !isModularDimension(region.width, widthCells) ||
        !isModularDimension(region.height, heightCells)) {
        result.valid = false;
        return result;
    }

    std::vector<GridCell> localCells;
    localCells.reserve(static_cast<std::size_t>(widthCells * heightCells));
    for (int y = 0; y < heightCells; ++y) {
        for (int x = 0; x < widthCells; ++x)
            localCells.push_back({x, y, region.material});
    }

    const CompositionResult composed = compose(localCells);
    if (!composed.valid) {
        result.valid = false;
        return result;
    }

    result.cells.reserve(composed.cells.size());
    for (const ComposedCell& cell : composed.cells) {
        result.cells.push_back({
            cell.cell.x,
            cell.cell.y,
            region.x + static_cast<float>(cell.cell.x * CELL_SIZE),
            region.y + static_cast<float>(cell.cell.y * CELL_SIZE),
            cell.cell.material,
            cell.neighbours,
            cell.topology,
        });
    }

    return result;
}

const char* toString(TopologyClass topology) {
    switch (topology) {
        case TopologyClass::Isolated:          return "isolated";
        case TopologyClass::LeftEnd:           return "left-end";
        case TopologyClass::RightEnd:          return "right-end";
        case TopologyClass::Interior:          return "interior";
        case TopologyClass::VerticalEdge:      return "vertical-edge";
        case TopologyClass::Corner:            return "corner";
        case TopologyClass::Junction:          return "junction";
        case TopologyClass::MaterialBoundary: return "material-boundary";
    }
    return "unknown";
}

} // namespace gfx::compositor
