#include "doctest/doctest.h"
#include "Logic/Level.h"

using namespace logic;

TEST_SUITE("Level / Construcao") {

    TEST_CASE("nivel vazio tem zero plataformas") {
        Level level;
        CHECK(level.platformCount() == 0);
        CHECK(level.platforms().empty());
    }

    TEST_CASE("addPlatform cria plataforma com bounds corretos") {
        Level level;
        level.addPlatform(50.0f, 100.0f, 200.0f, 20.0f);

        REQUIRE(level.platformCount() == 1);
        const auto& p = level.platforms()[0];
        CHECK(p.bounds.min.x == doctest::Approx(50.0f));
        CHECK(p.bounds.min.y == doctest::Approx(100.0f));
        CHECK(p.bounds.max.x == doctest::Approx(250.0f));
        CHECK(p.bounds.max.y == doctest::Approx(120.0f));
    }

    TEST_CASE("multiplas plataformas guardadas na ordem de insercao") {
        Level level;
        level.addPlatform(0.0f,  50.0f, 100.0f, 10.0f);
        level.addPlatform(0.0f, 150.0f, 100.0f, 10.0f);
        level.addPlatform(0.0f, 250.0f, 100.0f, 10.0f);

        REQUIRE(level.platformCount() == 3);
        CHECK(level.platforms()[0].bounds.min.y == doctest::Approx(50.0f));
        CHECK(level.platforms()[1].bounds.min.y == doctest::Approx(150.0f));
        CHECK(level.platforms()[2].bounds.min.y == doctest::Approx(250.0f));
    }

    TEST_CASE("clear() remove todas as plataformas") {
        Level level;
        level.addPlatform(0.0f, 100.0f, 100.0f, 10.0f);
        level.addPlatform(0.0f, 200.0f, 100.0f, 10.0f);
        REQUIRE(level.platformCount() == 2);
        level.clear();
        CHECK(level.platformCount() == 0);
    }
}
