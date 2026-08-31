// =============================================================================
// Tests/Unit/test_platform_compositor.cpp
//
// Structural pilot for the bounded 16x16 semantic platform compositor.
// =============================================================================
#include "doctest/doctest.h"
#include "Graphics/PlatformCompositor.h"

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
        CHECK(result.cells[0].topology == TopologyClass::Isolated);
    }

    TEST_CASE("F02 3x1 straight classifies middle as interior") {
        const auto result = compose(cells({
            {0, 0, 1}, {1, 0, 1}, {2, 0, 1}
        }));
        REQUIRE(result.valid);
        CHECK(findCell(result, 1, 0).topology == TopologyClass::Interior);
        CHECK(findCell(result, 1, 0).neighbours == (Left | Right));
    }

    TEST_CASE("F03 and F04 endpoints are directionally stable") {
        const auto result = compose(cells({
            {0, 0, 1}, {1, 0, 1}, {2, 0, 1}
        }));
        REQUIRE(result.valid);
        CHECK(findCell(result, 0, 0).topology == TopologyClass::LeftEnd);
        CHECK(findCell(result, 2, 0).topology == TopologyClass::RightEnd);
    }

    TEST_CASE("F05/F06 corners") {
        const auto result = compose(cells({
            {0, 0, 1}, {1, 0, 1}, {0, 1, 1}
        }));
        REQUIRE(result.valid);
        CHECK(findCell(result, 0, 0).topology == TopologyClass::Corner);
        CHECK(findCell(result, 1, 0).topology == TopologyClass::RightEnd);
        CHECK(findCell(result, 0, 1).topology == TopologyClass::VerticalEdge);
    }

    TEST_CASE("F07 stepped profile remains local") {
        const auto result = compose(cells({
            {0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {2, 1, 1}
        }));
        REQUIRE(result.valid);
        CHECK(findCell(result, 1, 0).topology == TopologyClass::Corner);
        CHECK(findCell(result, 1, 1).topology == TopologyClass::Corner);
    }

    TEST_CASE("F08 T junction") {
        const auto result = compose(cells({
            {0, 1, 1}, {1, 1, 1}, {2, 1, 1}, {1, 0, 1}
        }));
        REQUIRE(result.valid);
        CHECK(findCell(result, 1, 1).topology == TopologyClass::Junction);
    }

    TEST_CASE("F09 cross junction") {
        const auto result = compose(cells({
            {1, 0, 1}, {0, 1, 1}, {1, 1, 1}, {2, 1, 1}, {1, 2, 1}
        }));
        REQUIRE(result.valid);
        CHECK(findCell(result, 1, 1).topology == TopologyClass::Junction);
        CHECK(findCell(result, 1, 1).neighbours == (Left | Right | Up | Down));
    }

    TEST_CASE("F10 material boundary is explicit") {
        const auto result = compose(cells({
            {0, 0, 1}, {1, 0, 2}
        }));
        REQUIRE(result.valid);
        CHECK(findCell(result, 0, 0).topology == TopologyClass::MaterialBoundary);
        CHECK(findCell(result, 1, 0).topology == TopologyClass::MaterialBoundary);
    }

    TEST_CASE("F11 macro-vs-modular equivalent has identical structural output") {
        const auto modular = compose(cells({
            {0, 0, 1}, {1, 0, 1}, {2, 0, 1}
        }));
        const auto reordered = compose(cells({
            {2, 0, 1}, {0, 0, 1}, {1, 0, 1}
        }));

        REQUIRE(modular.valid);
        REQUIRE(reordered.valid);
        REQUIRE(modular.cells.size() == reordered.cells.size());

        for (std::size_t i = 0; i < modular.cells.size(); ++i) {
            CHECK(modular.cells[i].cell.x == reordered.cells[i].cell.x);
            CHECK(modular.cells[i].cell.y == reordered.cells[i].cell.y);
            CHECK(modular.cells[i].neighbours == reordered.cells[i].neighbours);
            CHECK(modular.cells[i].topology == reordered.cells[i].topology);
        }
    }

    TEST_CASE("F12 duplicate occupancy is rejected") {
        const auto result = compose(cells({
            {0, 0, 1}, {0, 0, 1}
        }));
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
            CHECK(first.cells[i].topology == second.cells[i].topology);
        }
    }
}
