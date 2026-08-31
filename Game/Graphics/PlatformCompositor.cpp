// =============================================================================
// Game/Graphics/PlatformCompositor.cpp
// =============================================================================
#include "Graphics/PlatformCompositor.h"

#include <algorithm>
#include <array>
#include <map>

namespace gfx::compositor {
namespace {

struct CellKey {
    int x;
    int y;

    friend bool operator<(const CellKey& lhs, const CellKey& rhs) {
        return std::tie(lhs.y, lhs.x) < std::tie(rhs.y, rhs.x);
    }
};

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

} // namespace

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
