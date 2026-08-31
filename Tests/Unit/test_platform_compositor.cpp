// =============================================================================
// Tests/Unit/test_platform_compositor.cpp
//
// Structural pilot for the bounded 16x16 semantic platform compositor.
// =============================================================================
#include "doctest/doctest.h"
#include "Graphics/PlatformCompositor.h"

#include <initializer_list>
#include <stdexcept>
#include <vector>

using namespace gfx::compositor;

namespace {

std::vector<GridCell> cells(std::initializer_list<GridCell> values) {
    return {values.begin(), values.end()};
}

const ComposedCell& findCell(const CompositionResult& result, int x, int y) {
    for (const auto& cell : result.cells) {
        if (cell.cell.x == x && cell.cell.y == y)
            return cell;
    }
    throw std::logic_error("cell not found");
}

} // namespace

TEST_SUITE("16x16 semantic compositor — structural pilot") {

    TEST_CASE("F01 isolated cell") {
        const auto result = compose(cells({{0, 0, 1}}));
        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 1);
        CHECK(result.cells[0].neighbours == None);
        CHECK(static_cast<int>(result.cells[0].topology) == static_cast<int>(TopologyClass::Isolated));
    }

    TEST_CASE("F02 3x1 straight classifies middle as interior") {
        const auto result = compose(cells({
            {0, 0, 1}, {1, 0, 1}, {2, 0, 1}
        }));
        REQUIRE(result.valid);
        CHECK(static_cast<int>(findCell(result, 1, 0).topology) == static_cast<int>(TopologyClass::Interior));
        CHECK(findCell(result, 1, 0).neighbours == (Left | Right));
    }

    TEST_CASE("F03 and F04 endpoints are directionally stable") {
        const auto result = compose(cells({
            {0, 0, 1}, {1, 0, 1}, {2, 0, 1}
        }));
        REQUIRE(result.valid);
        CHECK(static_cast<int>(findCell(result, 0, 0).topology) == static_cast<int>(TopologyClass::LeftEnd));
        CHECK(static_cast<int>(findCell(result, 2, 0).topology) == static_cast<int>(TopologyClass::RightEnd));
    }

    TEST_CASE("F05/F06 corners") {
        const auto result = compose(cells({
            {0, 0, 1}, {1, 0, 1}, {0, 1, 1}
        }));
        REQUIRE(result.valid);
        CHECK(static_cast<int>(findCell(result, 0, 0).topology) == static_cast<int>(TopologyClass::Corner));
        CHECK(static_cast<int>(findCell(result, 1, 0).topology) == static_cast<int>(TopologyClass::RightEnd));
        CHECK(static_cast<int>(findCell(result, 0, 1).topology) == static_cast<int>(TopologyClass::VerticalEdge));
    }

    TEST_CASE("F07 stepped profile remains local") {
        const auto result = compose(cells({
            {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {2, 1, 1}
        }));
        REQUIRE(result.valid);
        CHECK(static_cast<int>(findCell(result, 1, 0).topology) == static_cast<int>(TopologyClass::Corner));
        CHECK(static_cast<int>(findCell(result, 1, 1).topology) == static_cast<int>(TopologyClass::Corner));
    }

    TEST_CASE("F08 T junction") {
        const auto result = compose(cells({
            {0, 1, 1}, {1, 1, 1}, {2, 1, 1}, {1, 0, 1}
        }));
        REQUIRE(result.valid);
        CHECK(static_cast<int>(findCell(result, 1, 1).topology) == static_cast<int>(TopologyClass::Junction));
    }

    TEST_CASE("F09 cross junction") {
        const auto result = compose(cells({
            {1, 0, 1}, {0, 1, 1}, {1, 1, 1}, {2, 1, 1}, {1, 2, 1}
        }));
        REQUIRE(result.valid);
        CHECK(static_cast<int>(findCell(result, 1, 1).topology) == static_cast<int>(TopologyClass::Junction));
        CHECK(findCell(result, 1, 1).neighbours == (Left | Right | Up | Down));
    }

    TEST_CASE("F10 material boundary is explicit") {
        const auto result = compose(cells({
            {0, 0, 1}, {1, 0, 2}
        }));
        REQUIRE(result.valid);
        CHECK(static_cast<int>(findCell(result, 0, 0).topology) == static_cast<int>(TopologyClass::MaterialBoundary));
        CHECK(static_cast<int>(findCell(result, 1, 0).topology) == static_cast<int>(TopologyClass::MaterialBoundary));
    }

    TEST_CASE("F11 input ordering does not affect structural output") {
        const auto ordered = compose(cells({
            {0, 0, 1}, {1, 0, 1}, {2, 0, 1}
        }));
        const auto reordered = compose(cells({
            {2, 0, 1}, {0, 0, 1}, {1, 0, 1}
        }));

        REQUIRE(ordered.valid);
        REQUIRE(reordered.valid);
        REQUIRE(ordered.cells.size() == reordered.cells.size());

        for (std::size_t i = 0; i < ordered.cells.size(); ++i) {
            CHECK(ordered.cells[i].cell.x == reordered.cells[i].cell.x);
            CHECK(ordered.cells[i].cell.y == reordered.cells[i].cell.y);
            CHECK(ordered.cells[i].neighbours == reordered.cells[i].neighbours);
            CHECK(static_cast<int>(ordered.cells[i].topology) == static_cast<int>(reordered.cells[i].topology));
        }
    }

    TEST_CASE("F12 duplicate occupancy is rejected") {
        const auto result = compose(cells({
            {0, 0, 1}, {0, 0, 1}
        }));
        CHECK_FALSE(result.valid);
        CHECK(result.cells.empty());
    }

    TEST_CASE("T13 non-grid-aligned world origin is preserved") {
        const PlatformRegion region{436.0f, 89.0f, 176.0f, 16.0f, 1};
        const auto result = composeRegion(region);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 11);
        CHECK(result.cells.front().localX == 0);
        CHECK(result.cells.front().localY == 0);
        CHECK(result.cells.front().worldX == doctest::Approx(436.0f));
        CHECK(result.cells.front().worldY == doctest::Approx(89.0f));
        CHECK(result.cells.back().worldX == doctest::Approx(596.0f));
        CHECK(result.cells.back().worldY == doctest::Approx(89.0f));
    }

    TEST_CASE("T14 local lattice does not assume a global world grid") {
        const PlatformRegion first{436.0f, 89.0f, 176.0f, 16.0f, 1};
        const PlatformRegion second{448.0f, 121.0f, 176.0f, 16.0f, 1};
        const auto a = composeRegion(first);
        const auto b = composeRegion(second);

        REQUIRE(a.valid);
        REQUIRE(b.valid);
        REQUIRE(a.cells.size() == b.cells.size());
        CHECK(a.cells.front().localX == b.cells.front().localX);
        CHECK(a.cells.front().localY == b.cells.front().localY);
        CHECK(a.cells.front().worldX == doctest::Approx(436.0f));
        CHECK(b.cells.front().worldX == doctest::Approx(448.0f));
    }

    TEST_CASE("T15 non-modular platform dimensions are rejected") {
        const PlatformRegion region{436.0f, 89.0f, 175.0f, 16.0f, 1};
        const auto result = composeRegion(region);
        CHECK_FALSE(result.valid);
        CHECK(result.cells.empty());
    }

    TEST_CASE("same input is deterministic") {
        const auto first = compose(cells({
            {0, 0, 3}, {1, 0, 3}, {1, 1, 4}
        }));
        const auto second = compose(cells({
            {0, 0, 3}, {1, 0, 3}, {1, 1, 4}
        }));

        REQUIRE(first.valid);
        REQUIRE(second.valid);
        REQUIRE(first.cells.size() == second.cells.size());

        for (std::size_t i = 0; i < first.cells.size(); ++i) {
            CHECK(first.cells[i].cell.x == second.cells[i].cell.x);
            CHECK(first.cells[i].cell.y == second.cells[i].cell.y);
            CHECK(first.cells[i].cell.material == second.cells[i].cell.material);
            CHECK(first.cells[i].neighbours == second.cells[i].neighbours);
            CHECK(static_cast<int>(first.cells[i].topology) == static_cast<int>(second.cells[i].topology));
        }
    }
}
