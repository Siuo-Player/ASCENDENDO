#include "doctest/doctest.h"
#include "Logic/LevelDataIO.h"
#include "Logic/Level.h"
#include "Logic/LevelEditor.h"

#include <chrono>
#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path uniqueTempLevelPath() {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           ("ascendendo_leveldata_" + std::to_string(stamp) + ".lvl");
}

} // namespace

TEST_SUITE("LevelData") {

    TEST_CASE("carrega formato historico sem SPAWN") {
        const auto path = uniqueTempLevelPath();
        {
            std::ofstream out(path);
            REQUIRE(out.is_open());
            out << "NAME Legacy\n";
            out << "PLATFORM 0 0 640 20\n";
            out << "FLAG 600 320 16 16\n";
        }

        const auto data = logic::LevelDataIO::load(path);
        REQUIRE(data.has_value());
        CHECK(data->name == "Legacy");
        CHECK(data->platforms.size() == 1);
        CHECK(data->flag.has_value());
        CHECK(!data->spawnPosition.has_value());

        std::filesystem::remove(path);
    }

    TEST_CASE("preserva SPAWN 0 0 como valor explicito") {
        const auto path = uniqueTempLevelPath();
        {
            std::ofstream out(path);
            REQUIRE(out.is_open());
            out << "NAME Origin\n";
            out << "SPAWN 0 0\n";
            out << "PLATFORM 0 0 640 20\n";
        }

        const auto data = logic::LevelDataIO::load(path);
        REQUIRE(data.has_value());
        REQUIRE(data->spawnPosition.has_value());
        CHECK(data->spawnPosition->x == doctest::Approx(0.0f));
        CHECK(data->spawnPosition->y == doctest::Approx(0.0f));

        std::filesystem::remove(path);
    }

    TEST_CASE("save/load preserva dados declarativos") {
        logic::LevelData original;
        original.name = "Round Trip";
        original.platforms = {
            {{0.0f, 0.0f}, {640.0f, 20.0f}},
            {{128.0f, 96.0f}, {256.0f, 112.0f}}
        };
        original.spawnPosition = logic::Vec2{48.0f, 20.0f};
        original.flag = logic::AABB{{600.0f, 320.0f}, {616.0f, 336.0f}};

        const auto path = uniqueTempLevelPath();
        REQUIRE(logic::LevelDataIO::save(original, path));

        const auto loaded = logic::LevelDataIO::load(path);
        REQUIRE(loaded.has_value());
        CHECK(loaded->name == original.name);
        REQUIRE(loaded->platforms.size() == original.platforms.size());
        for (std::size_t i = 0; i < original.platforms.size(); ++i) {
            CHECK(loaded->platforms[i].min.x == doctest::Approx(original.platforms[i].min.x));
            CHECK(loaded->platforms[i].min.y == doctest::Approx(original.platforms[i].min.y));
            CHECK(loaded->platforms[i].max.x == doctest::Approx(original.platforms[i].max.x));
            CHECK(loaded->platforms[i].max.y == doctest::Approx(original.platforms[i].max.y));
        }
        REQUIRE(loaded->spawnPosition.has_value());
        CHECK(loaded->spawnPosition->x == doctest::Approx(original.spawnPosition->x));
        CHECK(loaded->spawnPosition->y == doctest::Approx(original.spawnPosition->y));
        REQUIRE(loaded->flag.has_value());
        CHECK(loaded->flag->min.x == doctest::Approx(original.flag->min.x));
        CHECK(loaded->flag->max.y == doctest::Approx(original.flag->max.y));

        std::filesystem::remove(path);
    }

    TEST_CASE("editor document produz LevelData sem perder ground, spawn e flag") {
        logic::LevelEditorDocument document(
            true, {{0.0f, 0.0f}, {640.0f, 20.0f}});
        REQUIRE(document.setSpawnX(48.0f));
        REQUIRE(document.addPlatform({{128.0f, 96.0f}, {256.0f, 116.0f}}));
        REQUIRE(document.setFlag({{600.0f, 320.0f}, {616.0f, 336.0f}}));

        const logic::LevelData data = document.toLevelData("Editor");
        CHECK(data.name == "Editor");
        REQUIRE(data.platforms.size() == 2);
        CHECK(data.platforms[0].min.y == doctest::Approx(0.0f));
        REQUIRE(data.spawnPosition.has_value());
        CHECK(data.spawnPosition->x == doctest::Approx(48.0f));
        CHECK(data.spawnPosition->y == doctest::Approx(20.0f));
        CHECK(data.flag.has_value());
    }

    TEST_CASE("runtime aplica offset sem alterar dados locais") {
        logic::LevelData data;
        data.name = "Chunk";
        data.platforms.push_back({{10.0f, 20.0f}, {50.0f, 30.0f}});
        data.flag = logic::AABB{{100.0f, 300.0f}, {116.0f, 316.0f}};

        logic::Level level;
        const float nextOffset = level.appendFromData(data, 640.0f, 360.0f);

        REQUIRE(level.platformCount() == 1);
        CHECK(level.platforms()[0].bounds.min.y == doctest::Approx(380.0f));
        REQUIRE(level.hasFlag);
        CHECK(level.flagBounds.min.y == doctest::Approx(660.0f));
        CHECK(nextOffset == doctest::Approx(720.0f));
        CHECK(data.platforms[0].min.y == doctest::Approx(20.0f));
        CHECK(data.flag->min.y == doctest::Approx(300.0f));
    }

    TEST_CASE("runtime caracteriza metadata singular por chunk") {
        logic::Level firstChunk;
        logic::LevelData first;
        first.name = "First";
        first.flag = logic::AABB{{10.0f, 20.0f}, {20.0f, 30.0f}};
        first.spawnPosition = logic::Vec2{32.0f, 30.0f};
        first.platforms.push_back({{0.0f, 0.0f}, {640.0f, 20.0f}});

        logic::LevelData second;
        second.name = "Second";
        second.flag = logic::AABB{{100.0f, 40.0f}, {110.0f, 50.0f}};
        second.platforms.push_back({{0.0f, 0.0f}, {640.0f, 20.0f}});

        const float nextOffset = firstChunk.appendFromData(first, 640.0f, 0.0f);
        REQUIRE(firstChunk.hasFlag);
        CHECK(firstChunk.flagBounds.min.y == doctest::Approx(20.0f));
        CHECK(first.spawnPosition->y == doctest::Approx(30.0f));

        firstChunk.appendFromData(second, 640.0f, nextOffset);
        REQUIRE(firstChunk.hasFlag);
        CHECK(firstChunk.flagBounds.min.y == doctest::Approx(nextOffset + 40.0f));
        CHECK(firstChunk.platformCount() == 2);
    }

    TEST_CASE("runtime nao materializa spawn como metadata de mundo") {
        logic::LevelData data;
        data.name = "Spawned";
        data.spawnPosition = logic::Vec2{48.0f, 20.0f};
        data.platforms.push_back({{0.0f, 0.0f}, {640.0f, 20.0f}});

        logic::Level level;
        const float nextOffset = level.appendFromData(data, 640.0f, 360.0f);

        CHECK(nextOffset == doctest::Approx(720.0f));
        CHECK(level.platformCount() == 1);
        CHECK_FALSE(level.hasFlag);
        CHECK(data.spawnPosition->x == doctest::Approx(48.0f));
        CHECK(data.spawnPosition->y == doctest::Approx(20.0f));
    }
}
