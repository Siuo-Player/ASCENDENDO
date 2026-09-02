#include "doctest/doctest.h"
#include "Graphics/PlatformCompositor.h"

#include <array>
#include <string_view>
#include <vector>

namespace {

using gfx::compositor::ANY_MATERIAL;
using gfx::compositor::AssetCandidate;
using gfx::compositor::CandidateRequest;
using gfx::compositor::GridCell;
using gfx::compositor::compose;
using gfx::compositor::selectCandidate;
using gfx::compositor::TopologyClass;
using gfx::compositor::CELL_SIZE;

GridCell cell(int x, int y, std::uint16_t material = 1) {
    return {x, y, material};
}

bool allTopology(const std::vector<gfx::compositor::ComposedCell>& cells,
                 TopologyClass topology) {
    for (const auto& item : cells) {
        if (item.topology != topology)
            return false;
    }
    return true;
}

CandidateRequest request(TopologyClass topology,
                         std::uint16_t material = 1,
                         bool flipRequired = false) {
    return {
        "platform",
        topology,
        material,
        1,
        1,
        flipRequired,
        1,
    };
}

} // namespace

TEST_SUITE("Compositor structural fixture pack v1") {

    TEST_CASE("F01 isolated 16x16 cell") {
        const std::array cells{cell(0, 0)};
        const auto result = compose(cells);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 1);
        CHECK(result.cells.front().neighbours == gfx::compositor::None);
        CHECK(result.cells.front().topology == TopologyClass::Isolated);
    }

    TEST_CASE("F02 3x1 straight exposes both ends and interior") {
        const std::array cells{cell(0, 0), cell(1, 0), cell(2, 0)};
        const auto result = compose(cells);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 3);
        CHECK(result.cells[0].topology == TopologyClass::LeftEnd);
        CHECK(result.cells[1].topology == TopologyClass::Interior);
        CHECK(result.cells[2].topology == TopologyClass::RightEnd);
    }

    TEST_CASE("F03 left end and F04 right end are orientation-sensitive") {
        const std::array cells{cell(0, 0), cell(1, 0)};
        const auto result = compose(cells);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 2);
        CHECK(result.cells[0].topology == TopologyClass::LeftEnd);
        CHECK(result.cells[1].topology == TopologyClass::RightEnd);
    }

    TEST_CASE("F05 upper-left and F06 upper-right corners classify as corners") {
        const std::array upperLeft{
            cell(0, 0), cell(1, 0), cell(0, 1)
        };
        const std::array upperRight{
            cell(0, 0), cell(1, 0), cell(1, 1)
        };

        const auto leftResult = compose(upperLeft);
        const auto rightResult = compose(upperRight);

        REQUIRE(leftResult.valid);
        REQUIRE(rightResult.valid);
        CHECK(leftResult.cells[0].topology == TopologyClass::Corner);
        CHECK(rightResult.cells[1].topology == TopologyClass::Corner);
    }

    TEST_CASE("F07 stepped profile preserves a single junction corner") {
        const std::array cells{
            cell(0, 0), cell(1, 0), cell(1, 1), cell(2, 1)
        };
        const auto result = compose(cells);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 4);
        CHECK(result.cells[1].topology == TopologyClass::Corner);
        CHECK(result.cells[2].topology == TopologyClass::Corner);
    }

    TEST_CASE("F08 T junction has degree-three centre") {
        const std::array cells{
            cell(0, 0), cell(1, 0), cell(2, 0), cell(1, 1)
        };
        const auto result = compose(cells);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 4);
        CHECK(result.cells[1].topology == TopologyClass::Junction);
    }

    TEST_CASE("F09 cross junction has degree-four centre") {
        const std::array cells{
            cell(1, 1), cell(0, 1), cell(2, 1), cell(1, 0), cell(1, 2)
        };
        const auto result = compose(cells);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 5);
        CHECK(result.cells.front().topology == TopologyClass::Junction);
    }

    TEST_CASE("F10 material transition marks both sides as material boundary") {
        const std::array cells{
            cell(0, 0, 1), cell(1, 0, 2)
        };
        const auto result = compose(cells);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 2);
        CHECK(result.cells[0].topology == TopologyClass::MaterialBoundary);
        CHECK(result.cells[1].topology == TopologyClass::MaterialBoundary);
    }

    TEST_CASE("F11 macro and modular representations preserve contact semantics") {
        const std::array cells{
            cell(0, 0), cell(1, 0), cell(2, 0)
        };
        const auto modular = compose(cells);
        const gfx::compositor::PlatformRegion region{
            436.0f, 89.0f, 48.0f, 16.0f, 1
        };
        const auto macro = gfx::compositor::composeRegion(region);

        REQUIRE(modular.valid);
        REQUIRE(macro.valid);
        REQUIRE(macro.cells.size() == modular.cells.size());

        for (std::size_t i = 0; i < modular.cells.size(); ++i) {
            CHECK(macro.cells[i].localX == modular.cells[i].cell.x);
            CHECK(macro.cells[i].localY == modular.cells[i].cell.y);
            CHECK(macro.cells[i].topology == modular.cells[i].topology);
            CHECK(macro.cells[i].worldX == doctest::Approx(
                region.x + static_cast<float>(i * CELL_SIZE)));
            CHECK(macro.cells[i].worldY == doctest::Approx(region.y));
        }
    }

    TEST_CASE("F12 empty or incompatible candidate sets use explicit fallback") {
        constexpr std::string_view fallback = "fallback/platform-default";
        const std::array noCandidates{};
        CHECK(selectCandidate(noCandidates, request(TopologyClass::Isolated), fallback) == fallback);

        const std::array wrongRole{
            AssetCandidate{"wrong", "other", 1u << static_cast<int>(TopologyClass::Isolated),
                           1, 1, 1, false, 1, 0}
        };
        CHECK(selectCandidate(wrongRole, request(TopologyClass::Isolated), fallback) == fallback);
    }

    TEST_CASE("T16 candidate ranking is invariant to manifest enumeration order") {
        const auto isolatedBit = 1u << static_cast<int>(TopologyClass::Isolated);
        const auto candidates = std::array{
            AssetCandidate{"z-generic", "platform", isolatedBit | (1u << static_cast<int>(TopologyClass::Corner)),
                           ANY_MATERIAL, 1, 1, false, 1, 50},
            AssetCandidate{"b-exact", "platform", isolatedBit, 1, 1, false, 1, 99},
            AssetCandidate{"a-exact", "platform", isolatedBit, 1, 1, false, 1, 99},
        };
        const auto permuted = std::array{candidates[2], candidates[0], candidates[1]};
        const auto query = request(TopologyClass::Isolated);

        CHECK(selectCandidate(candidates, query, "fallback") == "a-exact");
        CHECK(selectCandidate(permuted, query, "fallback") == "a-exact");
    }

    TEST_CASE("T17 ineligible candidates are rejected before ranking") {
        const auto isolatedBit = 1u << static_cast<int>(TopologyClass::Isolated);
        const std::array candidates{
            AssetCandidate{"no-flip", "platform", isolatedBit, 1, 1, false, 1, 0},
            AssetCandidate{"wrong-material", "platform", isolatedBit, 2, 1, true, 1, 0},
            AssetCandidate{"wrong-role", "other", isolatedBit, 1, 1, true, 1, 0},
        };

        CHECK(selectCandidate(candidates, request(TopologyClass::Isolated, 1, true),
                              "fallback") == "fallback");
    }

    TEST_CASE("T18 exact topology and material beat broad or wildcard candidates") {
        const auto isolatedBit = 1u << static_cast<int>(TopologyClass::Isolated);
        const std::array candidates{
            AssetCandidate{"broad-exact-material", "platform", isolatedBit | (1u << static_cast<int>(TopologyClass::Corner)),
                           1, 1, true, 1, 0},
            AssetCandidate{"exact-wildcard-material", "platform", isolatedBit,
                           ANY_MATERIAL, 1, 1, true, 1, 10},
            AssetCandidate{"exact-exact", "platform", isolatedBit,
                           1, 1, true, 1, 100},
        };

        CHECK(selectCandidate(candidates, request(TopologyClass::Isolated, 1, true),
                              "fallback") == "exact-exact");
    }

    TEST_CASE("duplicate semantic cells remain invalid") {
        const std::array cells{cell(0, 0), cell(0, 0)};
        const auto result = compose(cells);

        CHECK_FALSE(result.valid);
        CHECK(result.cells.empty());
    }

    TEST_CASE("fixture helper has no accidental universal-topology assumption") {
        const std::array cells{
            cell(0, 0), cell(1, 0), cell(2, 0)
        };
        const auto result = compose(cells);

        REQUIRE(result.valid);
        CHECK(!allTopology(result.cells, TopologyClass::Isolated));
    }
}
