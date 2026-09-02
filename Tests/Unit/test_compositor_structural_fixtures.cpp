#include "doctest/doctest.h"
#include "Graphics/PlatformAssetSelector.h"
#include "Graphics/PlatformCompositor.h"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace {

using gfx::compositor::GridCell;
using gfx::compositor::TopologyClass;
using gfx::compositor::CELL_SIZE;
using gfx::compositor::compose;
using gfx::compositor::findRegionContacts;
using gfx::compositor::PlatformRegion;
using gfx::assets::PlatformAssetCandidate;
using gfx::assets::PlatformAssetRequest;
using gfx::assets::selectBestPlatformAsset;
using gfx::assets::topologyBit;

GridCell cell(int x, int y, std::uint16_t material = 1) {
    return {x, y, material};
}

const gfx::compositor::ComposedCell* findCell(
    const std::vector<gfx::compositor::ComposedCell>& cells,
    int x,
    int y) {
    for (const auto& item : cells) {
        if (item.cell.x == x && item.cell.y == y)
            return &item;
    }
    return nullptr;
}

PlatformAssetCandidate reviewedCandidate(const char* id,
                                          TopologyClass topology,
                                          std::optional<std::uint16_t> material = 1,
                                          int variantRank = 0) {
    PlatformAssetCandidate candidate;
    candidate.assetId = id;
    candidate.topologyMask = topologyBit(topology);
    candidate.material = material;
    candidate.variantRank = variantRank;
    candidate.provenanceVerified = true;
    candidate.pixelScaleSafe = true;
    candidate.contactReadable = true;
    candidate.gameplayDecoupled = true;
    candidate.seamsAcceptable = true;
    return candidate;
}

} // namespace

TEST_SUITE("Compositor structural fixture pack v1") {

    TEST_CASE("F01 isolated cell") {
        const std::array cells{cell(0, 0)};
        const auto result = compose(cells);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 1);
        CHECK(result.cells.front().neighbours == gfx::compositor::None);
        CHECK(static_cast<int>(result.cells.front().topology) ==
              static_cast<int>(TopologyClass::Isolated));
    }

    TEST_CASE("F02 3x1 straight exposes both ends and interior") {
        const std::array cells{cell(0, 0), cell(1, 0), cell(2, 0)};
        const auto result = compose(cells);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 3);
        CHECK(static_cast<int>(findCell(result.cells, 0, 0)->topology) ==
              static_cast<int>(TopologyClass::LeftEnd));
        CHECK(static_cast<int>(findCell(result.cells, 1, 0)->topology) ==
              static_cast<int>(TopologyClass::Interior));
        CHECK(static_cast<int>(findCell(result.cells, 2, 0)->topology) ==
              static_cast<int>(TopologyClass::RightEnd));
    }

    TEST_CASE("F03 and F04 two-cell orientation remains explicit") {
        const std::array cells{cell(0, 0), cell(1, 0)};
        const auto result = compose(cells);

        REQUIRE(result.valid);
        CHECK(static_cast<int>(findCell(result.cells, 0, 0)->topology) ==
              static_cast<int>(TopologyClass::LeftEnd));
        CHECK(static_cast<int>(findCell(result.cells, 1, 0)->topology) ==
              static_cast<int>(TopologyClass::RightEnd));
    }

    TEST_CASE("F05 and F06 corners classify by local neighbourhood") {
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
        CHECK(static_cast<int>(findCell(leftResult.cells, 0, 0)->topology) ==
              static_cast<int>(TopologyClass::Corner));
        CHECK(static_cast<int>(findCell(rightResult.cells, 1, 0)->topology) ==
              static_cast<int>(TopologyClass::Corner));
    }

    TEST_CASE("F07 stepped profile exposes its two turns") {
        const std::array cells{
            cell(0, 0), cell(1, 0), cell(1, 1), cell(2, 1)
        };
        const auto result = compose(cells);

        REQUIRE(result.valid);
        CHECK(static_cast<int>(findCell(result.cells, 1, 0)->topology) ==
              static_cast<int>(TopologyClass::Corner));
        CHECK(static_cast<int>(findCell(result.cells, 1, 1)->topology) ==
              static_cast<int>(TopologyClass::Corner));
    }

    TEST_CASE("F08 T junction has degree-three centre") {
        const std::array cells{
            cell(0, 0), cell(1, 0), cell(2, 0), cell(1, 1)
        };
        const auto result = compose(cells);

        REQUIRE(result.valid);
        const auto* centre = findCell(result.cells, 1, 0);
        REQUIRE(centre != nullptr);
        CHECK(static_cast<int>(centre->topology) ==
              static_cast<int>(TopologyClass::Junction));
    }

    TEST_CASE("F09 cross junction has degree-four centre") {
        const std::array cells{
            cell(1, 1), cell(0, 1), cell(2, 1), cell(1, 0), cell(1, 2)
        };
        const auto result = compose(cells);

        REQUIRE(result.valid);
        const auto* centre = findCell(result.cells, 1, 1);
        REQUIRE(centre != nullptr);
        CHECK(static_cast<int>(centre->topology) ==
              static_cast<int>(TopologyClass::Junction));
    }

    TEST_CASE("F10 material transition produces semantic boundary") {
        const std::array cells{
            cell(0, 0, 1), cell(1, 0, 2)
        };
        const auto result = compose(cells);

        REQUIRE(result.valid);
        CHECK(static_cast<int>(findCell(result.cells, 0, 0)->topology) ==
              static_cast<int>(TopologyClass::MaterialBoundary));
        CHECK(static_cast<int>(findCell(result.cells, 1, 0)->topology) ==
              static_cast<int>(TopologyClass::MaterialBoundary));
    }

    TEST_CASE("F11 macro region preserves local modular composition") {
        const std::array localCells{cell(0, 0), cell(1, 0), cell(2, 0)};
        const auto local = compose(localCells);
        const gfx::compositor::PlatformRegion region{
            436.0f, 89.0f, 48.0f, 16.0f, 1
        };
        const auto macro = gfx::compositor::composeRegion(region);

        REQUIRE(local.valid);
        REQUIRE(macro.valid);
        REQUIRE(macro.cells.size() == local.cells.size());
        for (std::size_t i = 0; i < local.cells.size(); ++i) {
            CHECK(macro.cells[i].localX == local.cells[i].cell.x);
            CHECK(macro.cells[i].localY == local.cells[i].cell.y);
            CHECK(static_cast<int>(macro.cells[i].topology) ==
                  static_cast<int>(local.cells[i].topology));
            CHECK(macro.cells[i].worldX == doctest::Approx(
                region.x + static_cast<float>(i * CELL_SIZE)));
            CHECK(macro.cells[i].worldY == doctest::Approx(region.y));
        }
    }

    TEST_CASE("F12 missing reviewed asset is an explicit fallback condition") {
        const PlatformAssetRequest request{
            TopologyClass::Interior, 1, 1, 7, false, 1};
        auto blocked = reviewedCandidate("blocked", TopologyClass::Interior, 7);
        blocked.provenanceVerified = false;
        const std::array<PlatformAssetCandidate, 1> candidates = {blocked};

        // The compositor remains asset-agnostic. A failed reviewed selection is
        // the explicit handoff condition for the presentation fallback policy.
        CHECK_FALSE(selectBestPlatformAsset(candidates, request).has_value());
    }

    TEST_CASE("duplicate semantic cells remain invalid") {
        const std::array cells{cell(0, 0), cell(0, 0)};
        const auto result = compose(cells);

        CHECK_FALSE(result.valid);
        CHECK(result.cells.empty());
    }

    TEST_CASE("T13 and T15 preserve non-grid-aligned continuous origin") {
        const PlatformRegion region{436.25f, 89.5f, 48.0f, 16.0f, 1};
        const PlatformRegion original = region;
        const auto result = gfx::compositor::composeRegion(region);

        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 3);
        CHECK(result.cells.front().worldX == doctest::Approx(436.25f));
        CHECK(result.cells.front().worldY == doctest::Approx(89.5f));
        CHECK(result.cells.back().worldX == doctest::Approx(468.25f));
        CHECK(result.cells.back().worldY == doctest::Approx(89.5f));
        CHECK(region.x == original.x);
        CHECK(region.y == original.y);
        CHECK(region.width == original.width);
        CHECK(region.height == original.height);
        CHECK(region.material == original.material);
    }

    TEST_CASE("T14 horizontal contact uses world rectangles with different local origins") {
        const PlatformRegion lhs{436.25f, 89.5f, 176.0f, 16.0f, 1};
        const PlatformRegion rhs{612.25f, 95.0f, 176.0f, 16.0f, 1};

        const auto contacts = findRegionContacts(lhs, rhs);
        REQUIRE(contacts.size() == 1);
        CHECK(contacts.front().lhsLocalX == 10);
        CHECK(contacts.front().lhsLocalY == 0);
        CHECK(contacts.front().rhsLocalX == 0);
        CHECK(contacts.front().rhsLocalY == 0);
        CHECK(contacts.front().lhsNeighbour == gfx::compositor::Right);
        CHECK(contacts.front().rhsNeighbour == gfx::compositor::Left);
    }

    TEST_CASE("T14 vertical contact maps overlapping local cells") {
        const PlatformRegion lhs{100.0f, 100.0f, 32.0f, 16.0f, 1};
        const PlatformRegion rhs{112.0f, 116.0f, 48.0f, 16.0f, 1};

        const auto contacts = findRegionContacts(lhs, rhs);
        REQUIRE(contacts.size() == 2);
        CHECK(contacts[0].lhsLocalX == 0);
        CHECK(contacts[0].lhsLocalY == 0);
        CHECK(contacts[0].rhsLocalX == 0);
        CHECK(contacts[0].rhsLocalY == 0);
        CHECK(contacts[0].lhsNeighbour == gfx::compositor::Down);
        CHECK(contacts[0].rhsNeighbour == gfx::compositor::Up);
        CHECK(contacts[1].lhsLocalX == 1);
        CHECK(contacts[1].rhsLocalX == 0);
    }

    TEST_CASE("T14 tolerance admits only the requested visual adjacency") {
        const PlatformRegion lhs{0.0f, 0.0f, 16.0f, 16.0f, 1};
        const PlatformRegion rhs{16.0005f, 0.0f, 16.0f, 16.0f, 1};

        CHECK(findRegionContacts(lhs, rhs).empty());
        CHECK(findRegionContacts(lhs, rhs, 0.001f).size() == 1);
    }

    TEST_CASE("T14 separated regions with no edge contact stay disconnected") {
        const PlatformRegion lhs{0.0f, 0.0f, 32.0f, 16.0f, 1};
        const PlatformRegion rhs{40.0f, 0.0f, 32.0f, 16.0f, 1};

        CHECK(findRegionContacts(lhs, rhs).empty());
    }

    TEST_CASE("T14 invalid or non-modular regions produce no contacts") {
        const PlatformRegion invalid{-10.0f, 20.0f, 15.5f, 16.0f, 1};
        const PlatformRegion valid{5.5f, 20.0f, 16.0f, 16.0f, 1};

        CHECK(findRegionContacts(invalid, valid).empty());
        CHECK(findRegionContacts(valid, invalid).empty());
    }
}
