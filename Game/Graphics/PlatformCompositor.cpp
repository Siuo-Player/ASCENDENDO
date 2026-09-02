// =============================================================================
// Game/Graphics/PlatformCompositor.cpp
// =============================================================================
#include "Graphics/PlatformCompositor.h"

#include <algorithm>
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

bool validRegion(const PlatformRegion& region, int& widthCells, int& heightCells) {
    return std::isfinite(region.x) && std::isfinite(region.y) &&
           isModularDimension(region.width, widthCells) &&
           isModularDimension(region.height, heightCells);
}

float overlapLength(float lhsStart, float lhsEnd,
                    float rhsStart, float rhsEnd) {
    return std::min(lhsEnd, rhsEnd) - std::max(lhsStart, rhsStart);
}

} // namespace

std::vector<InvalidationCell> affectedCells(std::span<const GridCell> input,
                                             int changedX,
                                             int changedY) {
    std::vector<InvalidationCell> result;
    result.reserve(10);

    result.push_back({changedX, changedY});
    for (const GridCell& cell : input) {
        if (std::abs(cell.x - changedX) <= 1 &&
            std::abs(cell.y - changedY) <= 1 &&
            !(cell.x == changedX && cell.y == changedY))
            result.push_back({cell.x, cell.y});
    }

    std::sort(result.begin(), result.end(), [](const InvalidationCell& lhs,
                                               const InvalidationCell& rhs) {
        return std::tie(lhs.y, lhs.x) < std::tie(rhs.y, rhs.x);
    });
    return result;
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
    if (!validRegion(region, widthCells, heightCells)) {
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

std::vector<RegionContact> findRegionContacts(const PlatformRegion& lhs,
                                              const PlatformRegion& rhs,
                                              float tolerance) {
    std::vector<RegionContact> contacts;

    if (!std::isfinite(tolerance) || tolerance < 0.0f)
        return contacts;

    int lhsWidth = 0;
    int lhsHeight = 0;
    int rhsWidth = 0;
    int rhsHeight = 0;
    if (!validRegion(lhs, lhsWidth, lhsHeight) ||
        !validRegion(rhs, rhsWidth, rhsHeight))
        return contacts;

    const float lhsRight = lhs.x + lhs.width;
    const float lhsBottom = lhs.y + lhs.height;
    const float rhsRight = rhs.x + rhs.width;
    const float rhsBottom = rhs.y + rhs.height;

    const bool lhsIsLeft = std::fabs(lhsRight - rhs.x) <= tolerance;
    const bool rhsIsLeft = std::fabs(rhsRight - lhs.x) <= tolerance;
    if (lhsIsLeft || rhsIsLeft) {
        const float overlapStart = std::max(lhs.y, rhs.y);
        const float overlapEnd = std::min(lhsBottom, rhsBottom);
        if (overlapEnd > overlapStart + tolerance) {
            for (int lhsY = 0; lhsY < lhsHeight; ++lhsY) {
                const float lhsCellStart = lhs.y + lhsY * CELL_SIZE;
                const float lhsCellEnd = lhsCellStart + CELL_SIZE;
                const float localOverlapStart = std::max(lhsCellStart, overlapStart);
                const float localOverlapEnd = std::min(lhsCellEnd, overlapEnd);
                if (localOverlapEnd <= localOverlapStart + tolerance)
                    continue;

                for (int rhsY = 0; rhsY < rhsHeight; ++rhsY) {
                    const float rhsCellStart = rhs.y + rhsY * CELL_SIZE;
                    const float rhsCellEnd = rhsCellStart + CELL_SIZE;
                    if (overlapLength(localOverlapStart, localOverlapEnd,
                                      rhsCellStart, rhsCellEnd) <= tolerance)
                        continue;

                    const bool lhsLeftOfRhs = lhsIsLeft;
                    contacts.push_back({
                        lhsLeftOfRhs ? lhsWidth - 1 : 0,
                        lhsY,
                        lhsLeftOfRhs ? 0 : rhsWidth - 1,
                        rhsY,
                        static_cast<std::uint8_t>(lhsLeftOfRhs ? Right : Left),
                        static_cast<std::uint8_t>(lhsLeftOfRhs ? Left : Right),
                    });
                }
            }
        }
    }

    const bool lhsIsAbove = std::fabs(lhsBottom - rhs.y) <= tolerance;
    const bool rhsIsAbove = std::fabs(rhsBottom - lhs.y) <= tolerance;
    if (lhsIsAbove || rhsIsAbove) {
        const float overlapStart = std::max(lhs.x, rhs.x);
        const float overlapEnd = std::min(lhsRight, rhsRight);
        if (overlapEnd > overlapStart + tolerance) {
            for (int lhsX = 0; lhsX < lhsWidth; ++lhsX) {
                const float lhsCellStart = lhs.x + lhsX * CELL_SIZE;
                const float lhsCellEnd = lhsCellStart + CELL_SIZE;
                const float localOverlapStart = std::max(lhsCellStart, overlapStart);
                const float localOverlapEnd = std::min(lhsCellEnd, overlapEnd);
                if (localOverlapEnd <= localOverlapStart + tolerance)
                    continue;

                for (int rhsX = 0; rhsX < rhsWidth; ++rhsX) {
                    const float rhsCellStart = rhs.x + rhsX * CELL_SIZE;
                    const float rhsCellEnd = rhsCellStart + CELL_SIZE;
                    if (overlapLength(localOverlapStart, localOverlapEnd,
                                      rhsCellStart, rhsCellEnd) <= tolerance)
                        continue;

                    const bool lhsAboveRhs = lhsIsAbove;
                    contacts.push_back({
                        lhsX,
                        lhsAboveRhs ? lhsHeight - 1 : 0,
                        rhsX,
                        lhsAboveRhs ? 0 : rhsHeight - 1,
                        static_cast<std::uint8_t>(lhsAboveRhs ? Down : Up),
                        static_cast<std::uint8_t>(lhsAboveRhs ? Up : Down),
                    });
                }
            }
        }
    }

    return contacts;
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
