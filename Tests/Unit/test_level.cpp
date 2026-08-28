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
}
