// =============================================================================
// Tests/Unit/test_platform_asset_selector.cpp
//
// Deterministic platform asset candidate selection contract.
// =============================================================================
#include "doctest/doctest.h"
#include "Graphics/PlatformAssetSelector.h"

#include <initializer_list>
#include <vector>

using namespace gfx;
using namespace gfx::assets;

namespace {

constexpr std::uint16_t ALL_TOPOLOGIES = 0xFFFFu;

PlatformAssetCandidate candidate(
    const char* id,
    compositor::TopologyClass topology,
    std::optional<std::uint16_t> material,
    int variantRank,
    bool eligible = true) {
    PlatformAssetCandidate result;
    result.assetId = id;
    result.topologyMask = topologyBit(topology);
    result.material = material;
    result.variantRank = variantRank;
    result.provenanceVerified = eligible;
    result.pixelScaleSafe = eligible;
    result.contactReadable = eligible;
    result.gameplayDecoupled = eligible;
    result.seamsAcceptable = eligible;
    return result;
}

} // namespace

TEST_SUITE("16x16 semantic compositor — asset selection") {

    TEST_CASE("T16 ranking is invariant to manifest order") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 1};

        auto a = candidate("b", compositor::TopologyClass::Interior, 1, 0);
        auto b = candidate("a", compositor::TopologyClass::Interior, 1, 0);
        const std::vector<PlatformAssetCandidate> first{a, b};
        const std::vector<PlatformAssetCandidate> second{b, a};

        CHECK(selectBestPlatformAsset(first, request) == std::optional<std::string>("a"));
        CHECK(selectBestPlatformAsset(second, request) == std::optional<std::string>("a"));
    }

    TEST_CASE("T17 ineligible candidate is rejected") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 1};

        auto blocked = candidate("blocked", compositor::TopologyClass::Interior, 1, 99, false);
        CHECK_FALSE(selectBestPlatformAsset({&blocked, 1}, request).has_value());
    }

    TEST_CASE("T18 explicit tie-break uses asset id after metadata ranking") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 1};

        auto low = candidate("zeta", compositor::TopologyClass::Interior, 1, 3);
        auto high = candidate("alpha", compositor::TopologyClass::Interior, 1, 3);

        CHECK(selectBestPlatformAsset({&low, 2}, request) == std::optional<std::string>("alpha"));
    }

    TEST_CASE("exact material outranks generic candidate") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 7, false, 1};

        auto generic = candidate("generic", compositor::TopologyClass::Interior, std::nullopt, 5);
        auto exact = candidate("exact", compositor::TopologyClass::Interior, 7, 0);

        CHECK(selectBestPlatformAsset({&generic, 1}, request) == std::optional<std::string>("exact"));
    }

    TEST_CASE("mirrored request requires flip support") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, true, 1};

        auto candidateWithoutFlip = candidate("no-flip", compositor::TopologyClass::Interior, 1, 10);
        auto candidateWithFlip = candidate("flip", compositor::TopologyClass::Interior, 1, 0);
        candidateWithFlip.flipAllowed = true;

        CHECK(selectBestPlatformAsset({&candidateWithoutFlip, 1}, request).has_value() == false);
        CHECK(selectBestPlatformAsset({&candidateWithoutFlip, &candidateWithFlip, 2}, request) == std::optional<std::string>("flip"));
    }

    TEST_CASE("no winner is explicit when every candidate violates a hard gate") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 1};

        auto blockedA = candidate("a", compositor::TopologyClass::Interior, 1, 0, false);
        auto blockedB = candidate("b", compositor::TopologyClass::Interior, 1, 0, false);

        CHECK_FALSE(selectBestPlatformAsset({&blockedA, &blockedB, 2}, request).has_value());
    }

    TEST_CASE("topology mask can cover multiple classes") {
        PlatformAssetCandidate multi = candidate("multi", compositor::TopologyClass::Interior, 1, 0);
        multi.topologyMask = static_cast<std::uint16_t>(
            topologyBit(compositor::TopologyClass::Interior) |
            topologyBit(compositor::TopologyClass::LeftEnd));

        const PlatformAssetRequest request{
            compositor::TopologyClass::LeftEnd, 1, 1, 1, false, 1};

        CHECK(selectBestPlatformAsset({&multi, 1}, request) == std::optional<std::string>("multi"));
    }
}
