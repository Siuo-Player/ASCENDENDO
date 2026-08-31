// =============================================================================
// Tests/Unit/test_platform_asset_selector.cpp
//
// Deterministic platform asset candidate selection contract.
// =============================================================================
#include "doctest/doctest.h"
#include "Graphics/PlatformAssetSelector.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

using namespace gfx;
using namespace gfx::assets;

namespace {

PlatformAssetCandidate candidate(
    const char* id,
    compositor::TopologyClass topology,
    std::optional<std::uint16_t> material,
    int variantRank,
    bool isEligible = true) {
    PlatformAssetCandidate result;
    result.assetId = id;
    result.topologyMask = topologyBit(topology);
    result.material = material;
    result.variantRank = variantRank;
    result.provenanceVerified = isEligible;
    result.pixelScaleSafe = isEligible;
    result.contactReadable = isEligible;
    result.gameplayDecoupled = isEligible;
    result.seamsAcceptable = isEligible;
    return result;
}

} // namespace

TEST_SUITE("16x16 semantic compositor — asset selection") {

    TEST_CASE("T16 ranking is invariant to manifest order") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 1};

        const std::array<PlatformAssetCandidate, 2> first = {
            candidate("b", compositor::TopologyClass::Interior, 1, 0),
            candidate("a", compositor::TopologyClass::Interior, 1, 0)};
        const std::array<PlatformAssetCandidate, 2> second = {
            first[1], first[0]};

        CHECK(selectBestPlatformAsset(first, request) == std::optional<std::string>("a"));
        CHECK(selectBestPlatformAsset(second, request) == std::optional<std::string>("a"));
    }

    TEST_CASE("T17 ineligible candidate is rejected") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 1};

        const auto blocked = candidate("blocked", compositor::TopologyClass::Interior, 1, 99, false);
        const std::array<PlatformAssetCandidate, 1> candidates = {blocked};
        CHECK_FALSE(selectBestPlatformAsset(candidates, request).has_value());
    }

    TEST_CASE("T18 explicit tie-break uses asset id after metadata ranking") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 1};

        const auto low = candidate("zeta", compositor::TopologyClass::Interior, 1, 3);
        const auto high = candidate("alpha", compositor::TopologyClass::Interior, 1, 3);
        const std::array<PlatformAssetCandidate, 2> candidates = {low, high};

        CHECK(selectBestPlatformAsset(candidates, request) == std::optional<std::string>("alpha"));
    }

    TEST_CASE("exact material outranks generic candidate") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 7, false, 1};

        const auto generic = candidate("generic", compositor::TopologyClass::Interior, std::nullopt, 5);
        const auto exact = candidate("exact", compositor::TopologyClass::Interior, 7, 0);
        const std::array<PlatformAssetCandidate, 2> candidates = {generic, exact};

        CHECK(selectBestPlatformAsset(candidates, request) == std::optional<std::string>("exact"));
    }

    TEST_CASE("mirrored request requires flip support") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, true, 1};

        const auto candidateWithoutFlip = candidate("no-flip", compositor::TopologyClass::Interior, 1, 10);
        auto candidateWithFlip = candidate("flip", compositor::TopologyClass::Interior, 1, 0);
        candidateWithFlip.flipAllowed = true;

        const std::array<PlatformAssetCandidate, 1> blocked = {candidateWithoutFlip};
        const std::array<PlatformAssetCandidate, 2> candidates = {
            candidateWithoutFlip, candidateWithFlip};

        CHECK_FALSE(selectBestPlatformAsset(blocked, request).has_value());
        CHECK(selectBestPlatformAsset(candidates, request) == std::optional<std::string>("flip"));
    }

    TEST_CASE("no winner is explicit when every candidate violates a hard gate") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 1};

        const auto blockedA = candidate("a", compositor::TopologyClass::Interior, 1, 0, false);
        const auto blockedB = candidate("b", compositor::TopologyClass::Interior, 1, 0, false);
        const std::array<PlatformAssetCandidate, 2> candidates = {blockedA, blockedB};

        CHECK_FALSE(selectBestPlatformAsset(candidates, request).has_value());
    }

    TEST_CASE("topology mask can cover multiple classes") {
        auto multi = candidate("multi", compositor::TopologyClass::Interior, 1, 0);
        multi.topologyMask = static_cast<std::uint16_t>(
            topologyBit(compositor::TopologyClass::Interior) |
            topologyBit(compositor::TopologyClass::LeftEnd));

        const PlatformAssetRequest request{
            compositor::TopologyClass::LeftEnd, 1, 1, 1, false, 1};
        const std::array<PlatformAssetCandidate, 1> candidates = {multi};

        CHECK(selectBestPlatformAsset(candidates, request) == std::optional<std::string>("multi"));
    }
}
