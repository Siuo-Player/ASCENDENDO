#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/CampaignRuntime.h"
#include "../../Game/Logic/Level.h"
#include "../../Game/Core/Config.h"

#include <filesystem>

using namespace logic;

TEST_SUITE("Deterministic capture level loading") {
    TEST_CASE("Seleciona um unico nivel sem acumular os anteriores") {
        CampaignRuntime runtime({
            "Game/Assets/Levels/inicio.lvl",
            "Game/Assets/Levels/zigzag.lvl",
            "Game/Assets/Levels/precipicio.lvl"
        });
        Level level;

        REQUIRE(runtime.loadLevelAt(level, 1, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 2);
        CHECK(level.platformCount() > 0);
        CHECK(runtime.currentSpawnY() == doctest::Approx(config::LOGICAL_HEIGHT));
    }

    TEST_CASE("Indice fora da campanha nao altera estado") {
        CampaignRuntime runtime({"Game/Assets/Levels/inicio.lvl"});
        Level level;
        REQUIRE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));

        const int platformsBefore = level.platformCount();
        const float spawnBefore = runtime.currentSpawnY();
        const auto indexBefore = runtime.currentLevelIndex();

        CHECK_FALSE(runtime.loadLevelAt(level, 1, config::LOGICAL_WIDTH));
        CHECK(level.platformCount() == platformsBefore);
        CHECK(runtime.currentSpawnY() == doctest::Approx(spawnBefore));
        CHECK(runtime.currentLevelIndex() == indexBefore);
    }

    TEST_CASE("Nivel inexistente nao e carregado nem consumido") {
        CampaignRuntime runtime({"Game/Assets/Levels/nao-existe.lvl"});
        Level level;

        CHECK_FALSE(runtime.loadLevelAt(level, 0, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 0);
        CHECK(runtime.currentSpawnY() == doctest::Approx(0.0f));
        CHECK(level.platformCount() == 0);
    }
}
