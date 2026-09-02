#pragma once
// =============================================================================
// Game/Graphics/PlatformCompositor.h
//
// Bounded structural pilot for the 16x16 semantic platform compositor.
// This layer consumes presentation-side semantic regions; it does not own
// gameplay geometry, collision state, asset selection or renderer policy.
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
    None      = 0,
    Left      = 1u << 0,
    Right     = 1u << 1,
    Up        = 1u << 2,
    Down      = 1u << 3,
    UpLeft    = 1u << 4,
    UpRight   = 1u << 5,
    DownLeft  = 1u << 6,
    DownRight = 1u << 7,
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

// A semantic platform region keeps its own 16x16 visual lattice anchored at
// the region's continuous world origin. x/y are presentation inputs here and
// are never snapped or rewritten.
struct PlatformRegion {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
    std::uint16_t material = 0;
};

struct RegionCell {
    int localX = 0;
    int localY = 0;
    float worldX = 0.0f;
    float worldY = 0.0f;
    std::uint16_t material = 0;
    std::uint8_t neighbours = None;
    TopologyClass topology = TopologyClass::Isolated;
};

struct RegionCompositionResult {
    bool valid = true;
    std::vector<RegionCell> cells;
};

// A contact is a presentation-only adjacency between cells belonging to two
// independent semantic regions. It is derived from world-space rectangles,
// never from a shared global cell index.
struct RegionContact {
    int lhsLocalX = 0;
    int lhsLocalY = 0;
    int rhsLocalX = 0;
    int rhsLocalY = 0;
    std::uint8_t lhsNeighbour = None;
    std::uint8_t rhsNeighbour = None;
};

struct InvalidationCell {
    int x = 0;
    int y = 0;
};

// Return exactly the visual dependency neighbourhood of one changed semantic
// cell: that cell plus existing cells within Chebyshev distance one. The
// changed cell is retained even when it no longer exists in `input` (deletion).
// Output is deterministic and contains no duplicate coordinates.
std::vector<InvalidationCell> affectedCells(std::span<const GridCell> input,
                                             int changedX,
                                             int changedY);

// Deterministic structural compositor for an already-defined semantic grid.
CompositionResult compose(std::span<const GridCell> input);

// Expand one modular semantic platform into a local visual lattice without
// snapping its continuous world-space origin.
RegionCompositionResult composeRegion(const PlatformRegion& region);

// Derive presentation-only contacts between two modular semantic regions.
// Adjacency is computed from their continuous world-space rectangles using the
// supplied tolerance. Gameplay geometry is never modified.
std::vector<RegionContact> findRegionContacts(const PlatformRegion& lhs,
                                              const PlatformRegion& rhs,
                                              float tolerance = 1.0e-4f);

// Merge already-derived cross-region contacts into the 8-neighbour semantic
// signatures of two composed regions. The operation is fail-closed and atomic:
// invalid contact coordinates leave both results unchanged and return false.
// TopologyClass remains region-local and is intentionally not recomputed here.
bool applyRegionContacts(RegionCompositionResult& lhs,
                          RegionCompositionResult& rhs,
                          std::span<const RegionContact> contacts);

const char* toString(TopologyClass topology);

} // namespace gfx::compositor
