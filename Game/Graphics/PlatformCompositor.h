#pragma once
// =============================================================================
// Game/Graphics/PlatformCompositor.h
//
// Bounded structural pilot for the 16x16 semantic platform compositor.
// This layer consumes semantic grid cells only; it does not own gameplay
// geometry, collision state, asset selection or world->grid rasterization.
// =============================================================================

#include <cstdint>
#include <span>
#include <vector>

namespace gfx::compositor {

constexpr int CELL_SIZE = 16;

enum class TopologyClass : std::uint8_t {
    Isolated,
    LeftEnd,
    RightEnd,
    Interior,
    VerticalEdge,
    Corner,
    Junction,
    MaterialBoundary,
};

enum NeighbourMask : std::uint8_t {
    None  = 0,
    Left  = 1u << 0,
    Right = 1u << 1,
    Up    = 1u << 2,
    Down  = 1u << 3,
};

struct GridCell {
    int x = 0;
    int y = 0;
    std::uint16_t material = 0;
};

struct ComposedCell {
    GridCell cell{};
    std::uint8_t neighbours = None;
    TopologyClass topology = TopologyClass::Isolated;
};

struct CompositionResult {
    bool valid = true;
    std::vector<ComposedCell> cells;
};

// Deterministic structural compositor.
//
// Input coordinates are semantic 16x16 cell coordinates, not world pixels.
// The caller is responsible for defining the world->cell mapping; this keeps
// continuous LevelData coordinates out of the pilot until that contract is
// explicitly specified.
CompositionResult compose(std::span<const GridCell> input);

const char* toString(TopologyClass topology);

} // namespace gfx::compositor
