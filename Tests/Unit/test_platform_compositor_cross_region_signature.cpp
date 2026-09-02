// =============================================================================
// Tests/Unit/test_platform_compositor_cross_region_signature.cpp
//
// Cross-region contacts must enrich only the semantic neighbour signature.
// =============================================================================
#include "doctest/doctest.h"
#include "Graphics/PlatformCompositor.h"

#include <array>
#include <stdexcept>

using namespace gfx::compositor;

namespace {

const RegionCell& cellAt(const RegionCompositionResult& result, int x, int y) {
    for (const RegionCell& cell : result.cells) {
        if (cell.localX == x && cell.localY == y)
            return cell;
    }
    throw std::runtime_error("expected region cell not found");
}

} // namespace

TEST_SUITE("16x16 semantic compositor — cross-region signatures") {

    TEST_CASE("T19 horizontal contact enriches opposing edge signatures") {
        PlatformRegion lhs{0.0f, 32.0f, 32.0f, 16.0f, 1};
        PlatformRegion rhs{32.0f, 32.0f, 32.0f, 16.0f, 1};

        RegionCompositionResult lhsResult = composeRegion(lhs);
        RegionCompositionResult rhsResult = composeRegion(rhs);
        REQUIRE(lhsResult.valid);
        REQUIRE(rhsResult.valid);

        const auto contacts = findRegionContacts(lhs, rhs);
        REQUIRE(contacts.size() == 1);
        CHECK(contacts.front().lhsLocalX == 1);
        CHECK(contacts.front().rhsLocalX == 0);
        CHECK(contacts.front().lhsNeighbour == Right);
        CHECK(contacts.front().rhsNeighbour == Left);

        const bool applied = applyRegionContacts(lhsResult, rhsResult, contacts);
        CHECK(applied);
        CHECK((cellAt(lhsResult, 1, 0).neighbours & Right) != 0);
        CHECK((cellAt(rhsResult, 0, 0).neighbours & Left) != 0);

        const bool lhsTopologyIsLeftEnd =
            static_cast<int>(cellAt(lhsResult, 1, 0).topology) ==
            static_cast<int>(TopologyClass::LeftEnd);
        const bool rhsTopologyIsRightEnd =
            static_cast<int>(cellAt(rhsResult, 0, 0).topology) ==
            static_cast<int>(TopologyClass::RightEnd);
        CHECK(lhsTopologyIsLeftEnd);
        CHECK(rhsTopologyIsRightEnd);
    }

    TEST_CASE("T20 partial vertical overlap enriches every overlapping cell pair") {
        PlatformRegion lhs{10.0f, 0.0f, 16.0f, 32.0f, 1};
        PlatformRegion rhs{10.0f, 32.0f, 16.0f, 32.0f, 1};

        RegionCompositionResult lhsResult = composeRegion(lhs);
        RegionCompositionResult rhsResult = composeRegion(rhs);
        REQUIRE(lhsResult.valid);
        REQUIRE(rhsResult.valid);

        const auto contacts = findRegionContacts(lhs, rhs);
        REQUIRE(contacts.size() == 1);
        CHECK(contacts.front().lhsLocalX == 0);
        CHECK(contacts.front().lhsLocalY == 1);
        CHECK(contacts.front().rhsLocalX == 0);
        CHECK(contacts.front().rhsLocalY == 0);

        CHECK(applyRegionContacts(lhsResult, rhsResult, contacts));
        CHECK((cellAt(lhsResult, 0, 1).neighbours & Down) != 0);
        CHECK((cellAt(rhsResult, 0, 0).neighbours & Up) != 0);
    }

    TEST_CASE("T21 invalid contact is fail-closed and leaves both regions unchanged") {
        PlatformRegion lhs{0.0f, 0.0f, 16.0f, 16.0f, 1};
        PlatformRegion rhs{16.0f, 0.0f, 16.0f, 16.0f, 1};

        RegionCompositionResult lhsResult = composeRegion(lhs);
        RegionCompositionResult rhsResult = composeRegion(rhs);
        REQUIRE(lhsResult.valid);
        REQUIRE(rhsResult.valid);

        const auto lhsBefore = lhsResult;
        const auto rhsBefore = rhsResult;
        const std::array<RegionContact, 1> invalid = {{{
            99, 0, 0, 0, Right, Left
        }}};

        CHECK_FALSE(applyRegionContacts(lhsResult, rhsResult, invalid));
        CHECK(lhsResult.cells[0].neighbours == lhsBefore.cells[0].neighbours);
        CHECK(rhsResult.cells[0].neighbours == rhsBefore.cells[0].neighbours);

        const bool lhsTopologyUnchanged =
            static_cast<int>(lhsResult.cells[0].topology) ==
            static_cast<int>(lhsBefore.cells[0].topology);
        const bool rhsTopologyUnchanged =
            static_cast<int>(rhsResult.cells[0].topology) ==
            static_cast<int>(rhsBefore.cells[0].topology);
        CHECK(lhsTopologyUnchanged);
        CHECK(rhsTopologyUnchanged);
    }

    TEST_CASE("T22 repeated contact application is idempotent") {
        PlatformRegion lhs{0.0f, 0.0f, 16.0f, 16.0f, 1};
        PlatformRegion rhs{16.0f, 0.0f, 16.0f, 16.0f, 1};
        RegionCompositionResult lhsResult = composeRegion(lhs);
        RegionCompositionResult rhsResult = composeRegion(rhs);
        const auto contacts = findRegionContacts(lhs, rhs);
        REQUIRE(contacts.size() == 1);

        CHECK(applyRegionContacts(lhsResult, rhsResult, contacts));
        const std::uint8_t lhsMask = cellAt(lhsResult, 0, 0).neighbours;
        const std::uint8_t rhsMask = cellAt(rhsResult, 0, 0).neighbours;
        CHECK(applyRegionContacts(lhsResult, rhsResult, contacts));
        CHECK(cellAt(lhsResult, 0, 0).neighbours == lhsMask);
        CHECK(cellAt(rhsResult, 0, 0).neighbours == rhsMask);
    }
}
