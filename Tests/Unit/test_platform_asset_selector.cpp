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

constexpr const char* kTestSha256 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";

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
    result.runtimePath = std::string("Game/Assets/Sprites/ART_APPROVAL_INBOX/") + id + ".png";
    result.contentSha256 = kTestSha256;
    result.provenanceVerified = isEligible;
    result.pixelScaleSafe = isEligible;
    result.contactReadable = isEligible;
    result.gameplayDecoupled = isEligible;
    result.seamsAcceptable = isEligible;
    result.humanApproved = isEligible;
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

    TEST_CASE("explicit human approval is required even when all technical gates pass") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 1};

        auto reviewed = candidate("reviewed", compositor::TopologyClass::Interior, 1, 0);
        reviewed.humanApproved = false;
        const std::array<PlatformAssetCandidate, 1> unapproved = {reviewed};
        CHECK_FALSE(selectBestPlatformAsset(unapproved, request).has_value());

        reviewed.humanApproved = true;
        const std::array<PlatformAssetCandidate, 1> approved = {reviewed};
        CHECK(selectBestPlatformAsset(approved, request) == std::optional<std::string>("reviewed"));
    }

    TEST_CASE("exact runtime identity is required") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 1};

        auto missingPath = candidate("missing-path", compositor::TopologyClass::Interior, 1, 0);
        missingPath.runtimePath.clear();
        auto missingHash = candidate("missing-hash", compositor::TopologyClass::Interior, 1, 0);
        missingHash.contentSha256.clear();
        auto malformedHash = candidate("malformed-hash", compositor::TopologyClass::Interior, 1, 0);
        malformedHash.contentSha256 = "not-a-sha256";

        const std::array<PlatformAssetCandidate, 3> blocked = {
            missingPath, missingHash, malformedHash};
        CHECK_FALSE(selectBestPlatformAsset(blocked, request).has_value());

        const auto valid = candidate("valid", compositor::TopologyClass::Interior, 1, 0);
        const std::array<PlatformAssetCandidate, 1> accepted = {valid};
        CHECK(selectBestPlatformAsset(accepted, request) == std::optional<std::string>("valid"));
    }

    TEST_CASE("runtime path must be repository-relative and traversal-free") {
        const PlatformAssetRequest request{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 1};

        auto absoluteUnix = candidate("absolute-unix", compositor::TopologyClass::Interior, 1, 0);
        absoluteUnix.runtimePath = "/Game/Assets/platform.png";
        auto absoluteWindows = candidate("absolute-windows", compositor::TopologyClass::Interior, 1, 0);
        absoluteWindows.runtimePath = "C:\\Game\\Assets\\platform.png";
        auto leadingSlash = candidate("leading-slash", compositor::TopologyClass::Interior, 1, 0);
        leadingSlash.runtimePath = "\\Game\\Assets\\platform.png";
        auto traversal = candidate("traversal", compositor::TopologyClass::Interior, 1, 0);
        traversal.runtimePath = "Game/Assets/../platform.png";
        auto trailingSeparator = candidate("trailing-separator", compositor::TopologyClass::Interior, 1, 0);
        trailingSeparator.runtimePath = "Game/Assets/platform/";
        auto doubleSeparator = candidate("double-separator", compositor::TopologyClass::Interior, 1, 0);
        doubleSeparator.runtimePath = "Game//Assets/platform.png";

        const std::array<PlatformAssetCandidate, 6> blocked = {
            absoluteUnix, absoluteWindows, leadingSlash,
            traversal, trailingSeparator, doubleSeparator};
        CHECK_FALSE(selectBestPlatformAsset(blocked, request).has_value());

        auto valid = candidate("valid-relative", compositor::TopologyClass::Interior, 1, 0);
        valid.runtimePath = "Game/Assets/Sprites/platform.png";
        const std::array<PlatformAssetCandidate, 1> accepted = {valid};
        CHECK(selectBestPlatformAsset(accepted, request) == std::optional<std::string>("valid-relative"));
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

    TEST_CASE("invalid request footprint fails closed") {
        const auto valid = candidate("valid", compositor::TopologyClass::Interior, 1, 0);
        const std::array<PlatformAssetCandidate, 1> candidates = {valid};

        const PlatformAssetRequest zeroWidth{
            compositor::TopologyClass::Interior, 0, 1, 1, false, 1};
        const PlatformAssetRequest negativeHeight{
            compositor::TopologyClass::Interior, 1, -1, 1, false, 1};

        CHECK_FALSE(selectBestPlatformAsset(candidates, zeroWidth).has_value());
        CHECK_FALSE(selectBestPlatformAsset(candidates, negativeHeight).has_value());
    }

    TEST_CASE("invalid request scale fails closed") {
        const auto valid = candidate("valid", compositor::TopologyClass::Interior, 1, 0);
        const std::array<PlatformAssetCandidate, 1> candidates = {valid};

        const PlatformAssetRequest zeroScale{
            compositor::TopologyClass::Interior, 1, 1, 1, false, 0};
        const PlatformAssetRequest negativeScale{
            compositor::TopologyClass::Interior, 1, 1, 1, false, -2};

        CHECK_FALSE(selectBestPlatformAsset(candidates, zeroScale).has_value());
        CHECK_FALSE(selectBestPlatformAsset(candidates, negativeScale).has_value());
    }
}
