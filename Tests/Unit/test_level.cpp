#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/Level.h"

using namespace logic;

TEST_SUITE("Level") {
    TEST_CASE("clear limpa geometria e metadata") {
        Level level;
        level.name = "Chunk antigo";
        level.hasFlag = true;
        level.flagBounds = {{1.0f, 2.0f}, {3.0f, 4.0f}};
        level.addPlatform(0.0f, 0.0f, 10.0f, 1.0f);

        REQUIRE(level.platformCount() == 1);

        level.clear();

        CHECK(level.platformCount() == 0);
        CHECK(level.name == "Sem Nome");
        CHECK_FALSE(level.hasFlag);
        CHECK(level.flagBounds.min == Vec2{});
        CHECK(level.flagBounds.max == Vec2{});
    }

    TEST_CASE("final campaign level derives flag from highest platform") {
        LevelData data;
        data.name = "Final";
        data.platforms = {
            AABB{{0.0f, 0.0f}, {640.0f, 16.0f}},
            AABB{{120.0f, 80.0f}, {280.0f, 96.0f}},
            AABB{{360.0f, 240.0f}, {520.0f, 256.0f}},
        };

        Level level;
        level.appendFromData(data, 640.0f, 0.0f, true);

        REQUIRE(level.hasFlag);
        CHECK(level.flagBounds.min.x == 360.0f);
        CHECK(level.flagBounds.min.y == 256.0f);
        CHECK(level.flagBounds.max.x == 520.0f);
        CHECK(level.flagBounds.max.y == 296.0f);
    }

    TEST_CASE("non-final campaign level never gets a flag") {
        LevelData data;
        data.name = "Intermediate";
        data.platforms = {
            AABB{{0.0f, 0.0f}, {640.0f, 16.0f}},
            AABB{{300.0f, 240.0f}, {500.0f, 256.0f}},
        };

        Level level;
        level.appendFromData(data, 640.0f, 0.0f, false);

        CHECK_FALSE(level.hasFlag);
        CHECK(level.flagBounds.min == Vec2{});
        CHECK(level.flagBounds.max == Vec2{});
    }

    TEST_CASE("higher platform wins regardless of declaration order") {
        LevelData data;
        data.platforms = {
            AABB{{400.0f, 200.0f}, {520.0f, 216.0f}},
            AABB{{100.0f, 100.0f}, {300.0f, 116.0f}},
            AABB{{50.0f, 300.0f}, {250.0f, 316.0f}},
        };

        Level level;
        level.appendFromData(data, 640.0f, 0.0f, true);

        REQUIRE(level.hasFlag);
        CHECK(level.flagBounds.min.x == 50.0f);
        CHECK(level.flagBounds.min.y == 316.0f);
        CHECK(level.flagBounds.max.x == 250.0f);
        CHECK(level.flagBounds.max.y == 356.0f);
    }
}
