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
        CHECK(level.hasFlag);

        CHECK_FALSE(runtime.streamNextLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 3);
    }

    TEST_CASE("SCREENS 2 ocupa duas screens antes do proximo chunk") {
        const auto tallPath = std::filesystem::temp_directory_path() /
            "ascendendo-two-screen-runtime.lvl";
        const auto finalPath = std::filesystem::temp_directory_path() /
            "ascendendo-two-screen-final.lvl";

        writeLevel(tallPath,
                   "NAME Tall\n"
                   "SCREENS 2\n"
                   "PLATFORM 100 680 160 20\n");
        writeLevel(finalPath,
                   "NAME Final\n"
                   "SCREENS 1\n"
                   "PLATFORM 100 16 160 20\n");

        CampaignRuntime runtime({tallPath, finalPath});
        Level level;

        REQUIRE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 1);
        CHECK(runtime.currentSpawnY() == doctest::Approx(720.0f));
        REQUIRE(level.platformCount() == 2); // implicit ground + authored platform
        CHECK(level.platforms()[1].bounds.min.y == doctest::Approx(680.0f));

        REQUIRE(runtime.streamNextLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 2);
        CHECK(runtime.currentSpawnY() == doctest::Approx(1080.0f));
        REQUIRE(level.platformCount() == 3); // existing + final authored platform
        CHECK(level.platforms()[2].bounds.min.y == doctest::Approx(736.0f));
        CHECK(level.hasFlag);
        CHECK(level.flagBounds.min.x == doctest::Approx(100.0f));
        CHECK(level.flagBounds.min.y == doctest::Approx(756.0f));

        std::error_code ec;
        std::filesystem::remove(tallPath, ec);
        std::filesystem::remove(finalPath, ec);
    }

    TEST_CASE("Reset permite recomecar a campanha") {
        CampaignRuntime runtime({"Game/Assets/Levels/precipicio.lvl"});
        Level level;

        REQUIRE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 1);
        CHECK_FALSE(runtime.hasMoreLevels());
        CHECK(level.hasFlag);

        runtime.reset();
        CHECK(runtime.currentLevelIndex() == 0);
        CHECK(runtime.currentSpawnY() == doctest::Approx(0.0f));
        CHECK(runtime.hasMoreLevels());
    }

    TEST_CASE("Nivel inexistente nao e consumido") {
        CampaignRuntime runtime({
            "Game/Assets/Levels/inicio.lvl",
            "Game/Assets/Levels/nao-existe.lvl",
            "Game/Assets/Levels/precipicio.lvl"
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
        level.addPlatform(32.0f, 16.0f, 16.0f, 16.0f);

        CHECK_FALSE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 0);
        CHECK(runtime.currentSpawnY() == doctest::Approx(0.0f));
        CHECK(level.platformCount() == 0);
        CHECK(runtime.hasMoreLevels());
    }

    TEST_CASE("Nivel inicial semanticamente invalido e rejeitado") {
        const auto path = std::filesystem::temp_directory_path() /
            "ascendendo-semantic-invalid-initial.lvl";
        writeLevel(path, "NAME Invalid\nPLATFORM 0 16 -16 20\n");

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

    TEST_CASE("SPAWN e FLAG authored sao rejeitados") {
        const auto path = std::filesystem::temp_directory_path() /
            "ascendendo-derived-directives.lvl";
        writeLevel(path, "NAME Invalid\nSPAWN 320 16\nPLATFORM 0 16 100 20\n");

        CampaignRuntime runtime({path});
        Level level;
        CHECK_FALSE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));

        std::error_code ec;
        std::filesystem::remove(path, ec);
    }

    TEST_CASE("nivel final recebe objetivo derivado sem FLAG authored") {
        const auto path = std::filesystem::temp_directory_path() /
            "ascendendo-final-derived-goal.lvl";
        writeLevel(path, "NAME Final\nPLATFORM 100 80 120 20\nPLATFORM 300 180 160 20\n");

        CampaignRuntime runtime({path});
        Level level;

        REQUIRE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        CHECK_FALSE(runtime.hasMoreLevels());
        CHECK(level.hasFlag);
        CHECK(level.flagBounds.min.x == doctest::Approx(300.0f));
        CHECK(level.flagBounds.min.y == doctest::Approx(200.0f));
        CHECK(level.flagBounds.max.x == doctest::Approx(460.0f));
        CHECK(level.flagBounds.max.y == doctest::Approx(240.0f));

        std::error_code ec;
        std::filesystem::remove(path, ec);
    }

    TEST_CASE("nivel final sem plataformas authored e rejeitado") {
        const auto path = std::filesystem::temp_directory_path() /
            "ascendendo-final-without-platform.lvl";
        writeLevel(path, "NAME MissingGoal\n");

        CampaignRuntime runtime({path});
        Level level;

        CHECK_FALSE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == 0);
        CHECK(runtime.currentSpawnY() == doctest::Approx(0.0f));
        CHECK_FALSE(level.hasFlag);

        std::error_code ec;
        std::filesystem::remove(path, ec);
    }

    TEST_CASE("Chunk semanticamente invalido nao e consumido") {
        const auto invalidPath = std::filesystem::temp_directory_path() /
            "ascendendo-semantic-invalid-chunk.lvl";
        writeLevel(invalidPath, "NAME Invalid\nPLATFORM 0 16 20 0\n");

        CampaignRuntime runtime({
            "Game/Assets/Levels/inicio.lvl",
            invalidPath,
            "Game/Assets/Levels/precipicio.lvl"
        });
        Level level;

        REQUIRE(runtime.loadInitialLevel(level, config::LOGICAL_WIDTH));
        const auto indexBefore = runtime.currentLevelIndex();
        const float spawnBefore = runtime.currentSpawnY();
        const int platformsBefore = level.platformCount();

        CHECK_FALSE(runtime.streamNextLevel(level, config::LOGICAL_WIDTH));
        CHECK(runtime.currentLevelIndex() == indexBefore);
        CHECK(runtime.currentSpawnY() == doctest::Approx(spawnBefore));
        CHECK(runtime.levelCount() == 3);
        CHECK(level.platformCount() == platformsBefore);
        CHECK(runtime.hasMoreLevels());

        std::error_code ec;
        std::filesystem::remove(invalidPath, ec);
    }

    TEST_CASE("LevelDataIO rejects unknown directives") {
        const auto path = std::filesystem::temp_directory_path() / "ascendendo-unknown-directive.lvl";
        writeLevel(path, "NAME Valid\nPLATFORM 0 16 16 16\nUNKNOWN 1 2 3\n");

        CHECK_FALSE(LevelDataIO::load(path).has_value());
        std::error_code ec;
        std::filesystem::remove(path, ec);
    }

    TEST_CASE("LevelDataIO rejects trailing tokens") {
        const auto path = std::filesystem::temp_directory_path() / "ascendendo-trailing-tokens.lvl";
        writeLevel(path, "NAME Valid\nPLATFORM 0 16 16 16 extra\n");

        CHECK_FALSE(LevelDataIO::load(path).has_value());
        std::error_code ec;
        std::filesystem::remove(path, ec);
    }
}
