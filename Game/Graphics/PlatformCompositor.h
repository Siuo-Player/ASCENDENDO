#pragma once
// =============================================================================
// Game/Graphics/PlatformCompositor.h
//
// Bounded structural pilot for the 16x16 semantic platform compositor.
// This layer consumes presentation-side semantic regions; it does not own
// gameplay geometry, collision state, asset loading or renderer policy.
// =============================================================================

#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace gfx::compositor {

constexpr int CELL_SIZE = 16;
constexpr std::uint16_t ANY_MATERIAL = 0xFFFFu;

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

// Minimal metadata-only candidate contract. This deliberately does not load
// binaries or declare asset provenance/approval; it only resolves an already
// described candidate set deterministically.
struct AssetCandidate {
    std::string_view id{};
    std::string_view semanticRole{};
    std::uint32_t topologyMask = 0;
    std::uint16_t material = ANY_MATERIAL;
    int widthCells = 0;
    int heightCells = 0;
    bool supportsFlip = false;
    int scale = 1;
    int preferredRank = 0;
};

struct CandidateRequest {
    std::string_view semanticRole{};
    TopologyClass topology = TopologyClass::Isolated;
    std::uint16_t material = 0;
    int widthCells = 0;
    int heightCells = 0;
    bool flipRequired = false;
    int scale = 1;
};

// Returns the selected candidate id, or fallbackId when no candidate satisfies
// every hard constraint. Enumeration order must not influence the result.
std::string_view selectCandidate(std::span<const AssetCandidate> candidates,
                                 const CandidateRequest& request,
                                 std::string_view fallbackId);

// Deterministic structural compositor for an already-defined semantic grid.
CompositionResult compose(std::span<const GridCell> input);

// Expand one modular semantic platform into a local visual lattice without
// snapping its continuous world-space origin.
RegionCompositionResult composeRegion(const PlatformRegion& region);

const char* toString(TopologyClass topology);

} // namespace gfx::compositor
