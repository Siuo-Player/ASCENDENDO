#include "doctest/doctest.h"
#include "Logic/Level.h"

#include <cstdio>
#include <fstream>
#include <string>

using namespace logic;

TEST_SUITE("Level / File Loading") {
    // appendFromFile imprime [ERRO] em stderr se o ficheiro nao existir — esperado.

    TEST_CASE("appendFromFile: ficheiro inexistente retorna offsetY inalterado") {
        Level level;
        float result = level.appendFromFile("nao_existe_mesmo.lvl", 640.0f, 0.0f);

        CHECK(result == doctest::Approx(0.0f));
        CHECK(level.platformCount() == 0);
    }

    TEST_CASE("appendFromFile: PLATFORM carregada com offset Y correto") {
        const std::string tmpPath = "tmp_lvl_platform.lvl";
        {
            std::ofstream f(tmpPath);
            f << "# teste temporario\n";
            f << "NAME TestPlatform\n";
            f << "PLATFORM 50.0 100.0 100.0 15.0\n";
        }

        Level level;
        level.appendFromFile(tmpPath, 640.0f, 0.0f);

        REQUIRE(level.platformCount() == 1);
        CHECK(level.platforms()[0].bounds.min.x == doctest::Approx(50.0f));
        CHECK(level.platforms()[0].bounds.min.y == doctest::Approx(100.0f));
        CHECK(level.platforms()[0].bounds.max.y == doctest::Approx(115.0f));

        std::remove(tmpPath.c_str());
    }

    TEST_CASE("appendFromFile: FLAG define hasFlag e flagBounds") {
        const std::string tmpPath = "tmp_lvl_flag.lvl";
        {
            std::ofstream f(tmpPath);
            f << "FLAG 150.0 300.0 40.0 40.0\n";
        }

        Level level;
        level.appendFromFile(tmpPath, 640.0f, 0.0f);

        CHECK(level.hasFlag == true);
        CHECK(level.flagBounds.min.x == doctest::Approx(150.0f));
        CHECK(level.flagBounds.min.y == doctest::Approx(300.0f));
        CHECK(level.flagBounds.max.x == doctest::Approx(190.0f));
        CHECK(level.flagBounds.max.y == doctest::Approx(340.0f));

        std::remove(tmpPath.c_str());
    }

    TEST_CASE("appendFromFile: segundo chunk usa offsetY = offsetY_anterior + LOGICAL_HEIGHT") {
        const std::string tmpPath = "tmp_lvl_stack.lvl";
        {
            std::ofstream f(tmpPath);
            f << "PLATFORM 0.0 100.0 200.0 15.0\n";
        }

        Level level;
        float nextY = level.appendFromFile(tmpPath, 640.0f, 0.0f);
        CHECK(nextY == doctest::Approx(360.0f));

        level.appendFromFile(tmpPath, 640.0f, nextY);
        REQUIRE(level.platformCount() == 2);
        CHECK(level.platforms()[1].bounds.min.y == doctest::Approx(460.0f));

        std::remove(tmpPath.c_str());
    }

    TEST_CASE("appendFromFile: avanco do chunk e identico mesmo sem plataformas") {
        const std::string tmpEmpty = "tmp_lvl_empty.lvl";
        const std::string tmpFull  = "tmp_lvl_full.lvl";
        {
            std::ofstream f(tmpEmpty);
            f << "# nivel sem plataformas\n";
        }
        {
            std::ofstream f(tmpFull);
            f << "PLATFORM 0.0 0.0 640.0 20.0\n";
            f << "PLATFORM 230.0 95.0 180.0 20.0\n";
            f << "PLATFORM 30.0 195.0 180.0 20.0\n";
            f << "PLATFORM 230.0 295.0 180.0 20.0\n";
        }

        Level levelEmpty, levelFull;
        float nextEmpty = levelEmpty.appendFromFile(tmpEmpty, 640.0f, 0.0f);
        float nextFull  = levelFull.appendFromFile(tmpFull,  640.0f, 0.0f);

        CHECK(nextEmpty == doctest::Approx(360.0f));
        CHECK(nextFull  == doctest::Approx(360.0f));
        CHECK(nextEmpty == doctest::Approx(nextFull));

        std::remove(tmpEmpty.c_str());
        std::remove(tmpFull.c_str());
    }
}
