#include "doctest/doctest.h"
#include "Graphics/PlatformCompositor.h"

#include <array>
#include <cstddef>
#include <vector>

namespace {

using gfx::compositor::composeRegion;
using gfx::compositor::PlatformRegion;
using gfx::compositor::TopologyClass;

struct RealFixture {
    const char* id;
    PlatformRegion region;
    std::size_t expectedCells;
};

constexpr std::array<RealFixture, 4> kInicioCorpus{{
    {"R1", {0.0f, 4.0f, 640.0f, 16.0f, 0}, 40},
    {"R2", {436.0f, 89.0f, 176.0f, 16.0f, 0}, 11},
    {"R3", {148.0f, 174.0f, 176.0f, 16.0f, 0}, 11},
    {"R4", {436.0f, 259.0f, 176.0f, 16.0f, 0}, 11},
}};

} // namespace

TEST_SUITE("Inicio compositor execution corpus") {

    TEST_CASE("R1-R4 expand to the exact local 16x16 visual footprint") {
        for (const auto& fixture : kInicioCorpus) {
            const auto result = composeRegion(fixture.region);

            REQUIRE(result.valid);
            REQUIRE(result.cells.size() == fixture.expectedCells);

            if (fixture.id == "R1") {
                CHECK(result.cells.front().localX == 0);
                CHECK(result.cells.front().localY == 0);
                CHECK(result.cells.back().localX == 39);
                CHECK(result.cells.back().localY == 0);
                CHECK(result.cells.front().topology == TopologyClass::LeftEnd);
                CHECK(result.cells.back().topology == TopologyClass::RightEnd);
            } else {
                CHECK(result.cells.front().localX == 0);
                CHECK(result.cells.front().localY == 0);
                CHECK(result.cells.back().localX == 10);
                CHECK(result.cells.back().localY == 0);
                CHECK(result.cells.front().topology == TopologyClass::LeftEnd);
                CHECK(result.cells.back().topology == TopologyClass::RightEnd);
            }
        }
    }

    TEST_CASE("real Inicio origins remain continuous world coordinates") {
        for (const auto& fixture : kInicioCorpus) {
            const auto result = composeRegion(fixture.region);
            REQUIRE(result.valid);

            CHECK(result.cells.front().worldX == doctest::Approx(fixture.region.x));
            CHECK(result.cells.front().worldY == doctest::Approx(fixture.region.y));
            CHECK(result.cells.back().worldX == doctest::Approx(
                fixture.region.x + static_cast<float>(fixture.expectedCells - 1) * 16.0f));
            CHECK(result.cells.back().worldY == doctest::Approx(fixture.region.y));
        }
    }

    TEST_CASE("full-width floor uses world-content boundaries, not viewport isolation") {
        const auto result = composeRegion(kInicioCorpus[0].region);
        REQUIRE(result.valid);
        REQUIRE(result.cells.size() == 40);

        CHECK(result.cells.front().worldX == doctest::Approx(0.0f));
        CHECK(result.cells.back().worldX == doctest::Approx(624.0f));
        CHECK(result.cells.front().topology == TopologyClass::LeftEnd);
        CHECK(result.cells.back().topology == TopologyClass::RightEnd);
    }

    TEST_CASE("real fixtures do not mutate their gameplay-space inputs") {
        const std::vector<PlatformRegion> original{
            kInicioCorpus[0].region,
            kInicioCorpus[1].region,
            kInicioCorpus[2].region,
            kInicioCorpus[3].region,
        };
        auto observed = original;

        for (auto& region : observed) {
            const auto result = composeRegion(region);
            REQUIRE(result.valid);
        }

        CHECK(observed == original);
    }
}
