// =============================================================================
// Tests/Unit/test_render_snapshot.cpp
// =============================================================================
#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/RenderSnapshot.h"
#include "../../Game/Graphics/RenderSnapshotBuilder.h"
#include "../../Game/Logic/Level.h"
#include "../../Game/Logic/LevelDataIO.h"
#include "../../Game/Logic/Player.h"

#include <filesystem>
#include <type_traits>

using namespace gfx;
using namespace logic;

TEST_SUITE("RenderSnapshot") {
    TEST_CASE("é composto apenas por dados de apresentação") {
        static_assert(std::is_default_constructible_v<RenderSnapshot>);
        static_assert(std::is_copy_constructible_v<RenderSnapshot>);
        static_assert(std::is_copy_assignable_v<RenderSnapshot>);

        RenderSnapshot snapshot;
        snapshot.player.bounds = {10.0f, 20.0f, 32.0f, 32.0f};
        snapshot.player.facingDirection = -1.0f;
        snapshot.platforms.push_back({0.0f, 300.0f, 640.0f, 16.0f});
        snapshot.semanticPlatformsValid = true;
        snapshot.semanticPlatformCells.push_back({0, 0, 0.0f, 300.0f, 0, compositor::Right, compositor::TopologyClass::RightEnd});
        snapshot.flag.visible = true;
        snapshot.flag.bounds = {600.0f, 260.0f, 16.0f, 64.0f};

        const RenderSnapshot copy = snapshot;
        const int topology = static_cast<int>(copy.semanticPlatformCells[0].topology);
        CHECK(copy.player.bounds.x == 10.0f);
        CHECK(copy.player.facingDirection == -1.0f);
        CHECK(copy.platforms.size() == 1);
        CHECK(copy.semanticPlatformsValid);
        CHECK(copy.semanticPlatformCells.size() == 1);
        CHECK(copy.semanticPlatformCells[0].worldX == 0.0f);
        CHECK(topology == static_cast<int>(compositor::TopologyClass::RightEnd));
        CHECK(copy.flag.visible);
        CHECK(copy.flag.bounds.height == 64.0f);
    }

    TEST_CASE("builder copia apenas os dados usados pelo world pass") {
        Player player;
        Level level;

        player.body.position = {32.0f, 64.0f};
        player.facingDirection = -1.0f;
        level.addPlatform(0.0f, 100.0f, 120.0f, 20.0f);
        level.addPlatform(50.0f, 160.0f, 80.0f, 16.0f);
        level.hasFlag = true;
        level.flagBounds = {{90.0f, 200.0f}, {110.0f, 260.0f}};

        const RenderSnapshot snapshot = buildRenderSnapshot(player, level);

        REQUIRE(snapshot.platforms.size() == 2);
        CHECK(snapshot.platforms[0].x == doctest::Approx(0.0f));
        CHECK(snapshot.platforms[0].y == doctest::Approx(100.0f));
        CHECK(snapshot.platforms[0].width == doctest::Approx(120.0f));
        CHECK(snapshot.platforms[0].height == doctest::Approx(20.0f));
        CHECK(snapshot.platforms[1].x == doctest::Approx(50.0f));
        CHECK(snapshot.platforms[1].y == doctest::Approx(160.0f));
        CHECK(snapshot.platforms[1].width == doctest::Approx(80.0f));
        CHECK(snapshot.platforms[1].height == doctest::Approx(16.0f));
        CHECK_FALSE(snapshot.semanticPlatformsValid);
        CHECK(snapshot.flag.visible);
        CHECK(snapshot.flag.bounds.x == doctest::Approx(90.0f));
        CHECK(snapshot.flag.bounds.y == doctest::Approx(200.0f));
        CHECK(snapshot.flag.bounds.width == doctest::Approx(20.0f));
        CHECK(snapshot.flag.bounds.height == doctest::Approx(60.0f));
        CHECK(snapshot.player.bounds.x == doctest::Approx(32.0f));
        CHECK(snapshot.player.bounds.y == doctest::Approx(64.0f));
        CHECK(snapshot.player.facingDirection == doctest::Approx(-1.0f));
    }

    TEST_CASE("builder compõe plataformas modulares sem perder a origem contínua") {
        Player player;
        Level level;
        player.body.position = {20.25f, 30.5f};
        level.addPlatform(436.25f, 89.5f, 48.0f, 16.0f);

        const RenderSnapshot snapshot = buildRenderSnapshot(player, level);

        REQUIRE(snapshot.semanticPlatformsValid);
        REQUIRE(snapshot.semanticPlatformCells.size() == 3);
        CHECK(snapshot.semanticPlatformCells[0].worldX == doctest::Approx(436.25f));
        CHECK(snapshot.semanticPlatformCells[0].worldY == doctest::Approx(89.5f));
        CHECK(snapshot.semanticPlatformCells[1].worldX == doctest::Approx(452.25f));
        CHECK(snapshot.semanticPlatformCells[2].worldX == doctest::Approx(468.25f));
        const int firstTopology = static_cast<int>(snapshot.semanticPlatformCells[0].topology);
        const int lastTopology = static_cast<int>(snapshot.semanticPlatformCells[2].topology);
        CHECK(firstTopology == static_cast<int>(compositor::TopologyClass::LeftEnd));
        CHECK(lastTopology == static_cast<int>(compositor::TopologyClass::RightEnd));
    }

    TEST_CASE("builder propaga contacto entre regiões modulares") {
        Player player;
        Level level;
        level.addPlatform(0.25f, 100.5f, 32.0f, 16.0f);
        level.addPlatform(32.25f, 100.5f, 16.0f, 16.0f);

        const RenderSnapshot snapshot = buildRenderSnapshot(player, level);

        REQUIRE(snapshot.semanticPlatformsValid);
        REQUIRE(snapshot.semanticPlatformCells.size() == 3);
        CHECK((snapshot.semanticPlatformCells[1].neighbours & compositor::Right) != 0);
        CHECK((snapshot.semanticPlatformCells[2].neighbours & compositor::Left) != 0);
        CHECK(snapshot.semanticPlatformCells[0].worldX == doctest::Approx(0.25f));
        CHECK(snapshot.semanticPlatformCells[2].worldX == doctest::Approx(32.25f));
    }

    TEST_CASE("snapshot real de campanha preserva geometria e composição modular") {
        Player player;
        const std::filesystem::path root = "Game/Assets/Levels";

        struct Case {
            const char* file;
            std::size_t platformCount;
            std::size_t semanticCellCount;
        };

        const Case cases[] = {
            {"inicio.lvl", 4, 73},
            {"precipicio.lvl", 4, 26},
            {"zigzag.lvl", 4, 36},
        };

        for (const auto& test : cases) {
            const auto path = root / test.file;
            const auto data = LevelDataIO::load(path);
            REQUIRE_MESSAGE(data.has_value(), "failed to load campaign level: " << path.string());

            Level level;
            const float nextOffset = level.appendFromData(*data, 640.0f, 0.0f);
            REQUIRE(level.platformCount() == test.platformCount);
            CHECK(data->platforms.size() == test.platformCount);
            CHECK(nextOffset > 0.0f);

            const RenderSnapshot snapshot = buildRenderSnapshot(player, level);

            REQUIRE(snapshot.semanticPlatformsValid);
            CHECK(snapshot.semanticPlatformCells.size() == test.semanticCellCount);
            REQUIRE(snapshot.platforms.size() == test.platformCount);

            for (std::size_t i = 0; i < snapshot.platforms.size(); ++i) {
                CHECK(snapshot.platforms[i].x == doctest::Approx(data->platforms[i].min.x));
                CHECK(snapshot.platforms[i].y == doctest::Approx(data->platforms[i].min.y));
                CHECK(snapshot.platforms[i].width == doctest::Approx(data->platforms[i].width()));
                CHECK(snapshot.platforms[i].height == doctest::Approx(data->platforms[i].height()));
            }

            CHECK(snapshot.semanticPlatformCells.front().worldX == doctest::Approx(data->platforms.front().min.x));
            CHECK(snapshot.semanticPlatformCells.front().worldY == doctest::Approx(data->platforms.front().min.y));
        }
    }

    TEST_CASE("snapshot fica independente de alterações posteriores no runtime") {
        Player player;
        Level level;
        player.body.position = {1.0f, 2.0f};
        level.addPlatform(3.0f, 4.0f, 5.0f, 6.0f);

        const RenderSnapshot snapshot = buildRenderSnapshot(player, level);

        player.body.position = {100.0f, 200.0f};
        level.addPlatform(20.0f, 30.0f, 40.0f, 50.0f);

        REQUIRE(snapshot.platforms.size() == 1);
        CHECK(snapshot.platforms[0].x == doctest::Approx(3.0f));
        CHECK(snapshot.platforms[0].y == doctest::Approx(4.0f));
        CHECK(snapshot.player.bounds.x == doctest::Approx(1.0f));
        CHECK(snapshot.player.bounds.y == doctest::Approx(2.0f));
        CHECK_FALSE(snapshot.semanticPlatformsValid);
        CHECK(snapshot.semanticPlatformCells.empty());
    }
}
