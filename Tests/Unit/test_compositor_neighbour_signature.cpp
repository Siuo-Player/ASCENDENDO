#include "doctest/doctest.h"
#include "Graphics/PlatformCompositor.h"

#include <array>
#include <cstdint>

namespace {

using gfx::compositor::DownRight;
using gfx::compositor::GridCell;
using gfx::compositor::UpLeft;
using gfx::compositor::TopologyClass;
using gfx::compositor::compose;

GridCell cell(int x, int y) {
    return {x, y, 1};
}

} // namespace

TEST_SUITE("16x16 semantic compositor — eight-neighbour signature") {

    TEST_CASE("diagonal-only adjacency is reciprocal and cardinally isolated") {
        const std::array cells{cell(0, 0), cell(1, 1)};
        const auto result = compose(cells);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 2);

        const auto& origin = result.cells[0];
        const auto& diagonal = result.cells[1];

        CHECK((origin.neighbours & DownRight) != 0);
        CHECK((diagonal.neighbours & UpLeft) != 0);

        const auto originCardinal = static_cast<std::uint8_t>(
            origin.neighbours & static_cast<std::uint8_t>(
                gfx::compositor::Left | gfx::compositor::Right |
                gfx::compositor::Up | gfx::compositor::Down));
        const auto diagonalCardinal = static_cast<std::uint8_t>(
            diagonal.neighbours & static_cast<std::uint8_t>(
                gfx::compositor::Left | gfx::compositor::Right |
                gfx::compositor::Up | gfx::compositor::Down));
        CHECK(originCardinal == gfx::compositor::None);
        CHECK(diagonalCardinal == gfx::compositor::None);
        CHECK(static_cast<int>(origin.topology) ==
              static_cast<int>(TopologyClass::Isolated));
        CHECK(static_cast<int>(diagonal.topology) ==
              static_cast<int>(TopologyClass::Isolated));
    }

    TEST_CASE("cardinal topology remains unchanged when diagonal neighbours exist") {
        const std::array cells{
            cell(0, 0), cell(1, 0), cell(1, 1)
        };
        const auto result = compose(cells);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 3);

        const auto& corner = result.cells[0];
        CHECK((corner.neighbours & gfx::compositor::Right) != 0);
        CHECK((corner.neighbours & gfx::compositor::DownRight) != 0);
        CHECK(static_cast<int>(corner.topology) ==
              static_cast<int>(TopologyClass::LeftEnd));
    }
}
