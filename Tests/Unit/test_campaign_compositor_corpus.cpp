#include "doctest/doctest.h"
#include "Core/CampaignLoader.h"
#include "Graphics/PlatformCompositor.h"
#include "Graphics/PlatformPresentationRasterizer.h"
#include "Logic/LevelDataIO.h"

#include <cmath>
#include <filesystem>

TEST_SUITE("Real campaign compositor corpus") {
    TEST_CASE("every active campaign level rasterizes its canonical platforms") {
        const std::filesystem::path campaignFile =
            "Game/Assets/Levels/campaign.txt";
        const std::filesystem::path levelsRoot =
            "Game/Assets/Levels";

        const auto levelPaths = core::CampaignLoader::load(campaignFile, levelsRoot);
        REQUIRE(levelPaths.size() >= 3);

        std::size_t totalRegions = 0;
        std::size_t totalCells = 0;

        for (const auto& levelPath : levelPaths) {
            const auto level = logic::LevelDataIO::load(levelPath);
            REQUIRE(level.has_value());
            REQUIRE(!level->platforms.empty());

            const auto rasterized = gfx::presentation::rasterizePlatforms(*level);
            REQUIRE(rasterized.valid);
            REQUIRE(rasterized.regions.size() == level->platforms.size());

            std::size_t expectedLevelCells = 0;
            for (std::size_t i = 0; i < level->platforms.size(); ++i) {
                const auto& platform = level->platforms[i];
                const float width = platform.width();
                const float height = platform.height();

                CHECK(width > 0.0f);
                CHECK(height > 0.0f);
                CHECK(std::fmod(width, static_cast<float>(gfx::compositor::CELL_SIZE)) ==
                      doctest::Approx(0.0f));
                CHECK(std::fmod(height, static_cast<float>(gfx::compositor::CELL_SIZE)) ==
                      doctest::Approx(0.0f));

                const auto& region = rasterized.regions[i];
                CHECK(region.x == doctest::Approx(platform.min.x));
                CHECK(region.y == doctest::Approx(platform.min.y));
                CHECK(region.width == doctest::Approx(width));
                CHECK(region.height == doctest::Approx(height));

                const auto composed = gfx::compositor::composeRegion(region);
                REQUIRE(composed.valid);
                REQUIRE(composed.cells.size() ==
                        static_cast<std::size_t>((width / 16.0f) * (height / 16.0f)));

                CHECK(composed.cells.front().worldX == doctest::Approx(platform.min.x));
                CHECK(composed.cells.front().worldY == doctest::Approx(platform.min.y));
                CHECK(composed.cells.back().worldX == doctest::Approx(
                    platform.max.x - static_cast<float>(gfx::compositor::CELL_SIZE)));
                CHECK(composed.cells.back().worldY == doctest::Approx(
                    platform.max.y - static_cast<float>(gfx::compositor::CELL_SIZE)));

                expectedLevelCells += composed.cells.size();
            }

            CHECK(expectedLevelCells >= level->platforms.size());
            totalRegions += rasterized.regions.size();
            totalCells += expectedLevelCells;
        }

        CHECK(totalRegions >= 10);
        CHECK(totalCells >= 100);
    }

    TEST_CASE("campaign corpus preserves world geometry while presentation is derived") {
        const auto levelPaths = core::CampaignLoader::load(
            "Game/Assets/Levels/campaign.txt",
            "Game/Assets/Levels");
        REQUIRE(levelPaths.size() >= 3);

        for (const auto& levelPath : levelPaths) {
            const auto level = logic::LevelDataIO::load(levelPath);
            REQUIRE(level.has_value());
            const auto original = level->platforms;

            const auto rasterized = gfx::presentation::rasterizePlatforms(*level);
            REQUIRE(rasterized.valid);

            CHECK(level->platforms.size() == original.size());
            for (std::size_t i = 0; i < original.size(); ++i) {
                CHECK(level->platforms[i].min.x == doctest::Approx(original[i].min.x));
                CHECK(level->platforms[i].min.y == doctest::Approx(original[i].min.y));
                CHECK(level->platforms[i].max.x == doctest::Approx(original[i].max.x));
                CHECK(level->platforms[i].max.y == doctest::Approx(original[i].max.y));
            }
        }
    }
}
