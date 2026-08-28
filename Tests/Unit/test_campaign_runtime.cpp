#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/CampaignRuntime.h"
#include "../../Game/Logic/Level.h"
#include "../../Game/Logic/LevelDataIO.h"
#include "../../Game/Core/Config.h"

#include <filesystem>
#include <fstream>
#include <vector>

using namespace logic;

namespace {

void writeLevel(const std::filesystem::path& path, const char* contents) {
    std::ofstream out(path);
    REQUIRE(out.is_open());
    out << contents;
    REQUIRE(out.good());
}

} // namespace

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

    TEST_CASE("Nivel inexistente nao e consumido") {
        CampaignRuntime runtime({
            "Game/Assets/Levels/inicio.lvl",
            "Game/Assets/Levels/nao-existe.lvl"
        });
        Level level;

        REQUIRE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        const auto indexBefore = runtime.currentLevelIndex();
        const float spawnBefore = runtime.currentSpawnY();
        const int platformsBefore = level.platformCount();

        CHECK_FALSE(runtime.streamNextLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == indexBefore);
        CHECK(runtime.currentSpawnY() == doctest::Approx(spawnBefore));
        CHECK(level.platformCount() == platformsBefore);
        CHECK(runtime.hasMoreLevels());
    }

    TEST_CASE("Nivel inicial inexistente nao altera o progresso") {
        CampaignRuntime runtime({"Game/Assets/Levels/nao-existe.lvl"});
        Level level;
        level.addPlatform(0.0f, 0.0f, 16.0f, 16.0f);

        CHECK_FALSE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 0);
        CHECK(runtime.currentSpawnY() == doctest::Approx(0.0f));
        CHECK(level.platformCount() == 0);
        CHECK(runtime.hasMoreLevels());
    }

    TEST_CASE("Nivel inicial semanticamente invalido e rejeitado") {
        const auto path = std::filesystem::temp_directory_path() /
            "ascendendo-semantic-invalid-initial.lvl";
        writeLevel(path, "NAME Invalid\nPLATFORM 0 0 -16 20\n");

        CampaignRuntime runtime({path});
        Level level;

        CHECK_FALSE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 0);
        CHECK(runtime.currentSpawnY() == doctest::Approx(0.0f));
        CHECK(level.platformCount() == 0);
        CHECK(runtime.hasMoreLevels());

        std::error_code ec;
        std::filesystem::remove(path, ec);
    }

    TEST_CASE("Chunk semanticamente invalido nao e consumido") {
        const auto invalidPath = std::filesystem::temp_directory_path() /
            "ascendendo-semantic-invalid-chunk.lvl";
        writeLevel(invalidPath, "NAME Invalid\nPLATFORM 0 0 20 0\n");

        CampaignRuntime runtime({
            "Game/Assets/Levels/inicio.lvl",
            invalidPath
        });
        Level level;

        REQUIRE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        const auto indexBefore = runtime.currentLevelIndex();
        const float spawnBefore = runtime.currentSpawnY();
        const int platformsBefore = level.platformCount();

        CHECK_FALSE(runtime.streamNextLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == indexBefore);
        CHECK(runtime.currentSpawnY() == doctest::Approx(spawnBefore));
        CHECK(runtime.levelCount() == 2);
        CHECK(level.platformCount() == platformsBefore);
        CHECK(runtime.hasMoreLevels());

        std::error_code ec;
        std::filesystem::remove(invalidPath, ec);
    }

    TEST_CASE("LevelDataIO rejects unknown directives") {
        const auto path = std::filesystem::temp_directory_path() / "ascendendo-unknown-directive.lvl";
        writeLevel(path, "NAME Valid\nPLATFORM 0 0 16 16\nUNKNOWN 1 2 3\n");

        CHECK_FALSE(LevelDataIO::load(path).has_value());
        std::error_code ec;
        std::filesystem::remove(path, ec);
    }

    TEST_CASE("LevelDataIO rejects trailing tokens") {
        const auto path = std::filesystem::temp_directory_path() / "ascendendo-trailing-tokens.lvl";
        writeLevel(path, "NAME Valid\nPLATFORM 0 0 16 16 extra\n");

        CHECK_FALSE(LevelDataIO::load(path).has_value());
        std::error_code ec;
        std::filesystem::remove(path, ec);
    }
}
