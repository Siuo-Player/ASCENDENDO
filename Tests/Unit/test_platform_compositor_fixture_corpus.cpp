// =============================================================================
// Tests/Unit/test_platform_compositor_fixture_corpus.cpp
//
// Explicit structural fixture corpus for the bounded 16x16 compositor.
// This is test evidence, not player-facing content.
// =============================================================================
#include "doctest/doctest.h"
#include "Graphics/PlatformAssetSelector.h"
#include "Graphics/PlatformCompositor.h"

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace {

using gfx::assets::PlatformAssetRequest;
using gfx::assets::selectBestPlatformAsset;
using gfx::compositor::compose;
using gfx::compositor::composeRegion;
using gfx::compositor::GridCell;
using gfx::compositor::None;
using gfx::compositor::Left;
using gfx::compositor::Right;
using gfx::compositor::Up;
using gfx::compositor::Down;
using gfx::compositor::TopologyClass;
using gfx::compositor::PlatformRegion;

struct ExpectedCell {
    int x;
    int y;
    std::uint8_t neighbours;
    TopologyClass topology;
};

struct Fixture {
    const char* id;
    const char* description;
    const char* seedConfig;
    const char* expectedFallback;
    bool requiresCandidate;
    std::vector<GridCell> cells;
    std::vector<ExpectedCell> expected;
};

std::vector<GridCell> cells(std::initializer_list<GridCell> values) {
    return {values.begin(), values.end()};
}

const gfx::compositor::ComposedCell* findCell(
    const gfx::compositor::CompositionResult& result, int x, int y) {
    for (const auto& cell : result.cells) {
        if (cell.cell.x == x && cell.cell.y == y)
            return &cell;
    }
    return nullptr;
}

std::vector<Fixture> fixtureCorpus() {
    return {
        {
            "F01", "isolated 16x16 cell", "fixture-v1", "NONE", true,
            cells({{0, 0, 1}}),
            {{0, 0, None, TopologyClass::Isolated}},
        },
        {
            "F02", "3x1 straight", "fixture-v1", "NONE", true,
            cells({{0, 0, 1}, {1, 0, 1}, {2, 0, 1}}),
            {
                {0, 0, Right, TopologyClass::LeftEnd},
                {1, 0, static_cast<std::uint8_t>(Left | Right), TopologyClass::Interior},
                {2, 0, Left, TopologyClass::RightEnd},
            },
        },
        {
            "F03", "left-end topology", "fixture-v1", "NONE", true,
            cells({{0, 0, 1}, {1, 0, 1}}),
            {{0, 0, Right, TopologyClass::LeftEnd}},
        },
        {
            "F04", "right-end topology", "fixture-v1", "NONE", true,
            cells({{0, 0, 1}, {1, 0, 1}}),
            {{1, 0, Left, TopologyClass::RightEnd}},
        },
        {
            "F05", "upper-left corner", "fixture-v1", "NONE", true,
            cells({{0, 0, 1}, {1, 0, 1}, {0, 1, 1}}),
            {{0, 0, static_cast<std::uint8_t>(Right | Down), TopologyClass::Corner}},
        },
        {
            "F06", "upper-right corner", "fixture-v1", "NONE", true,
            cells({{0, 0, 1}, {1, 0, 1}, {1, 1, 1}}),
            {{1, 0, static_cast<std::uint8_t>(Left | Down), TopologyClass::Corner}},
        },
        {
            "F07", "stepped profile", "fixture-v1", "NONE", true,
            cells({{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {2, 1, 1}}),
            {
                {1, 0, static_cast<std::uint8_t>(Left | Down), TopologyClass::Corner},
                {1, 1, static_cast<std::uint8_t>(Up | Right), TopologyClass::Corner},
            },
        },
        {
            "F08", "T junction", "fixture-v1", "NONE", true,
            cells({{0, 1, 1}, {1, 1, 1}, {2, 1, 1}, {1, 0, 1}}),
            {{1, 1, static_cast<std::uint8_t>(Left | Right | Up), TopologyClass::Junction}},
        },
        {
            "F09", "cross junction", "fixture-v1", "NONE", true,
            cells({{1, 0, 1}, {0, 1, 1}, {1, 1, 1}, {2, 1, 1}, {1, 2, 1}}),
            {{1, 1, static_cast<std::uint8_t>(Left | Right | Up | Down), TopologyClass::Junction}},
        },
        {
            "F10", "material transition", "fixture-v1", "NONE", true,
            cells({{0, 0, 1}, {1, 0, 2}}),
            {
                {0, 0, Right, TopologyClass::MaterialBoundary},
                {1, 0, Left, TopologyClass::MaterialBoundary},
            },
        },
        {
            "F11", "macro-versus-modular equivalent", "fixture-v1", "NONE", true,
            cells({{0, 0, 1}, {1, 0, 1}}),
            {
                {0, 0, Right, TopologyClass::LeftEnd},
                {1, 0, Left, TopologyClass::RightEnd},
            },
        },
        {
            "F12", "missing-art candidate selection fallback boundary", "fixture-v1", "NO_WINNER", false,
            cells({{0, 0, 1}}),
            {{0, 0, None, TopologyClass::Isolated}},
        },
    };
}

} // namespace

TEST_SUITE("16x16 semantic compositor — explicit fixture corpus v1") {

    TEST_CASE("F01-F10 and F12 expose declared topology expectations") {
        const auto corpus = fixtureCorpus();
        REQUIRE(corpus.size() == 12);

        for (const auto& fixture : corpus) {
            const auto result = compose(fixture.cells);
            REQUIRE(result.valid);
            REQUIRE(result.cells.size() == fixture.cells.size());

            for (const auto& expected : fixture.expected) {
                const auto* actual = findCell(result, expected.x, expected.y);
                REQUIRE(actual != nullptr);
                CHECK(actual->neighbours == expected.neighbours);
                CHECK(static_cast<int>(actual->topology) == static_cast<int>(expected.topology));
            }
        }
    }

    TEST_CASE("fixture corpus declares unique ids and stable execution metadata") {
        const auto corpus = fixtureCorpus();

        for (std::size_t i = 0; i < corpus.size(); ++i) {
            CHECK(std::string(corpus[i].id) == "F" + std::string(i < 9 ? "0" : "") + std::to_string(i + 1));
            CHECK(std::string(corpus[i].seedConfig) == "fixture-v1");
        }
    }

    TEST_CASE("F11 macro and modular representations preserve topology") {
        const auto macro = composeRegion(PlatformRegion{100.0f, 50.0f, 32.0f, 16.0f, 1});
        const auto modular = compose(cells({{0, 0, 1}, {1, 0, 1}}));

        REQUIRE(macro.valid);
        REQUIRE(modular.valid);
        REQUIRE(macro.cells.size() == modular.cells.size());

        for (std::size_t i = 0; i < modular.cells.size(); ++i) {
            CHECK(macro.cells[i].localX == modular.cells[i].cell.x);
            CHECK(macro.cells[i].localY == modular.cells[i].cell.y);
            CHECK(macro.cells[i].neighbours == modular.cells[i].neighbours);
            CHECK(static_cast<int>(macro.cells[i].topology) == static_cast<int>(modular.cells[i].topology));
        }

        CHECK(macro.cells.front().worldX == doctest::Approx(100.0f));
        CHECK(macro.cells.front().worldY == doctest::Approx(50.0f));
        CHECK(macro.cells.back().worldX == doctest::Approx(116.0f));
    }

    TEST_CASE("F12 missing candidate remains an explicit no-winner boundary") {
        const auto corpus = fixtureCorpus();
        const auto& fixture = corpus.back();
        REQUIRE(std::string(fixture.id) == "F12");
        CHECK_FALSE(fixture.requiresCandidate);
        CHECK(std::string(fixture.expectedFallback) == "NO_WINNER");

        const PlatformAssetRequest request{
            TopologyClass::Isolated, 1, 1, 1, false, 1};
        const std::array<gfx::assets::PlatformAssetCandidate, 0> noCandidates{};
        CHECK_FALSE(selectBestPlatformAsset(noCandidates, request).has_value());
    }
}
