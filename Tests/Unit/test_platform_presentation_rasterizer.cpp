#include "doctest/doctest.h"
#include "Graphics/PlatformPresentationRasterizer.h"

#include <array>

using namespace gfx;

TEST_SUITE("Platform presentation rasterizer") {

    TEST_CASE("LevelData platforms preserve continuous origins and footprint") {
        logic::LevelData level;
        level.name = "Inicio-like";
        level.platforms = {
            {{0.0f, 4.0f}, {640.0f, 20.0f}},
            {{436.0f, 89.0f}, {612.0f, 105.0f}},
            {{148.0f, 174.0f}, {324.0f, 190.0f}},
            {{436.0f, 259.0f}, {612.0f, 275.0f}},
        };
        const auto original = level.platforms;

        const auto result = presentation::rasterizePlatforms(level, 3);

        REQUIRE(result.valid);
        REQUIRE(result.regions.size() == 4);
        CHECK(result.regions[0].x == doctest::Approx(0.0f));
        CHECK(result.regions[0].y == doctest::Approx(4.0f));
        CHECK(result.regions[0].width == doctest::Approx(640.0f));
        CHECK(result.regions[0].height == doctest::Approx(16.0f));
        CHECK(result.regions[1].x == doctest::Approx(436.0f));
        CHECK(result.regions[1].y == doctest::Approx(89.0f));
        CHECK(result.regions[1].width == doctest::Approx(176.0f));
        CHECK(result.regions[1].height == doctest::Approx(16.0f));
        CHECK(result.regions[1].material == 3);

        REQUIRE(level.platforms.size() == original.size());
        for (std::size_t i = 0; i < original.size(); ++i) {
            CHECK(level.platforms[i].min.x == original[i].min.x);
            CHECK(level.platforms[i].min.y == original[i].min.y);
            CHECK(level.platforms[i].max.x == original[i].max.x);
            CHECK(level.platforms[i].max.y == original[i].max.y);
        }
    }

    TEST_CASE("non-modular footprint fails closed without snapping or cropping") {
        const std::array platforms = {
            logic::AABB{{10.25f, 20.5f}, {41.75f, 36.5f}},
        };

        const auto result = presentation::rasterizePlatforms(platforms);

        CHECK_FALSE(result.valid);
        CHECK(result.regions.empty());
    }

    TEST_CASE("zero, negative and non-finite dimensions fail closed") {
        const std::array invalidPlatforms = {
            logic::AABB{{0.0f, 0.0f}, {0.0f, 16.0f}},
        };
        CHECK_FALSE(presentation::rasterizePlatforms(invalidPlatforms).valid);

        const std::array negativePlatforms = {
            logic::AABB{{16.0f, 16.0f}, {0.0f, 32.0f}},
        };
        CHECK_FALSE(presentation::rasterizePlatforms(negativePlatforms).valid);

        const std::array nanPlatforms = {
            logic::AABB{{0.0f, 0.0f}, {16.0f, std::numeric_limits<float>::quiet_NaN()}},
        };
        CHECK_FALSE(presentation::rasterizePlatforms(nanPlatforms).valid);
    }

    TEST_CASE("already modular arbitrary world origin remains unchanged") {
        const std::array platforms = {
            logic::AABB{{436.25f, 89.5f}, {484.25f, 105.5f}},
        };

        const auto result = presentation::rasterizePlatforms(platforms, 9);

        REQUIRE(result.valid);
        REQUIRE(result.regions.size() == 1);
        CHECK(result.regions[0].x == doctest::Approx(436.25f));
        CHECK(result.regions[0].y == doctest::Approx(89.5f));
        CHECK(result.regions[0].width == doctest::Approx(48.0f));
        CHECK(result.regions[0].height == doctest::Approx(16.0f));
        CHECK(result.regions[0].material == 9);
    }
}
