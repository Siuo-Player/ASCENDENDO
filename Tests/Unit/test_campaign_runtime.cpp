#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/CampaignRuntime.h"
#include "../../Game/Logic/Level.h"
#include "../../Game/Core/Config.h"

#include <filesystem>
#include <vector>

using namespace logic;

TEST_SUITE("CampaignRuntime") {
    TEST_CASE("Reset e nivel inicial preservam progresso deterministico") {
        CampaignRuntime runtime({
            "Game/Assets/Levels/inicio.lvl",
            "Game/Assets/Levels/zigzag.lvl",
            "Game/Assets/Levels/precipicio.lvl"
        });
        Level level;

        CHECK(runtime.currentLevelIndex() == 0);
        CHECK(runtime.currentSpawnY() == doctest::Approx(0.0f));
        CHECK(runtime.levelCount() == 3);

        REQUIRE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 1);
        CHECK(runtime.currentSpawnY() == doctest::Approx(config::LOGICAL_HEIGHT));
        CHECK(level.platformCount() > 0);
        CHECK(runtime.hasMoreLevels());
    }

    TEST_CASE("Streaming consome exatamente uma entrada por chamada") {
        CampaignRuntime runtime({
            "Game/Assets/Levels/inicio.lvl",
            "Game/Assets/Levels/zigzag.lvl",
            "Game/Assets/Levels/precipicio.lvl"
        });
        Level level;

        REQUIRE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        const int initialPlatforms = level.platformCount();
        const float initialSpawnY = runtime.currentSpawnY();

        REQUIRE(runtime.streamNextLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 2);
        CHECK(runtime.currentSpawnY() > initialSpawnY);
        CHECK(level.platformCount() > initialPlatforms);

        REQUIRE(runtime.streamNextLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 3);
        CHECK_FALSE(runtime.hasMoreLevels());

        CHECK_FALSE(runtime.streamNextLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 3);
    }

    TEST_CASE("Reset permite recomecar a campanha") {
        CampaignRuntime runtime({"Game/Assets/Levels/inicio.lvl"});
        Level level;

        REQUIRE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 1);

        runtime.reset();
        CHECK(runtime.currentLevelIndex() == 0);
        CHECK(runtime.currentSpawnY() == doctest::Approx(0.0f));
        CHECK(runtime.hasMoreLevels());
    }
}
