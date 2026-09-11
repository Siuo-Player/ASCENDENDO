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

    TEST_CASE("SPAWN e FLAG authored sao rejeitados pelo parser") {
        const auto path = uniqueTempLevelPath();
        {
            std::ofstream out(path);
            REQUIRE(out.is_open());
            out << "NAME Derived\n";
            out << "SPAWN 320 16\n";
        }
        CHECK_FALSE(logic::LevelDataIO::load(path).has_value());
        std::filesystem::remove(path);

        {
            std::ofstream out(path);
            REQUIRE(out.is_open());
            out << "NAME Derived\n";
            out << "PLATFORM 100 80 120 16\n";
            out << "FLAG 100 96 120 40\n";
        }
        CHECK_FALSE(logic::LevelDataIO::load(path).has_value());
        std::filesystem::remove(path);
    }

    TEST_CASE("save/load preserva apenas dados authored") {
        logic::LevelData original;
        original.name = "Round Trip";
        original.platforms = {
            {{128.0f, 96.0f}, {256.0f, 112.0f}},
            {{320.0f, 192.0f}, {448.0f, 208.0f}}
        };

        const auto path = uniqueTempLevelPath();
        REQUIRE(logic::LevelDataIO::save(original, path));

        const auto loaded = logic::LevelDataIO::load(path);
        REQUIRE(loaded.has_value());
        CHECK(loaded->name == original.name);
        REQUIRE(loaded->platforms.size() == original.platforms.size());
        CHECK_FALSE(loaded->spawnPosition.has_value());
        CHECK_FALSE(loaded->flag.has_value());
        for (std::size_t i = 0; i < original.platforms.size(); ++i) {
            CHECK(loaded->platforms[i].min.x == doctest::Approx(original.platforms[i].min.x));
            CHECK(loaded->platforms[i].min.y == doctest::Approx(original.platforms[i].min.y));
            CHECK(loaded->platforms[i].max.x == doctest::Approx(original.platforms[i].max.x));
            CHECK(loaded->platforms[i].max.y == doctest::Approx(original.platforms[i].max.y));
        }

        std::filesystem::remove(path);
    }

    TEST_CASE("editor document produz somente geometria authored") {
        logic::LevelEditorDocument document(
            true, {{0.0f, 0.0f}, {640.0f, 16.0f}});
        REQUIRE(document.addPlatform({{128.0f, 96.0f}, {256.0f, 116.0f}}));

        const logic::LevelData data = document.toLevelData("Editor");
        CHECK(data.name == "Editor");
        REQUIRE(data.platforms.size() == 1);
        CHECK(data.platforms[0].min.y == doctest::Approx(96.0f));
        CHECK_FALSE(data.spawnPosition.has_value());
        CHECK_FALSE(data.flag.has_value());
        CHECK(document.spawnPosition().x == doctest::Approx(320.0f));
        CHECK(document.spawnPosition().y == doctest::Approx(16.0f));
        REQUIRE(document.flag() != nullptr);
        CHECK(document.flag()->min.x == doctest::Approx(128.0f));
        CHECK(document.flag()->min.y == doctest::Approx(116.0f));
    }

    TEST_CASE("runtime materializa solo e spawn apenas no mundo inicial") {
        logic::LevelData data;
        data.name = "Chunk";
        data.platforms.push_back({{100.0f, 20.0f}, {150.0f, 30.0f}});

        logic::Level level;
        const float nextOffset = level.appendFromData(data, 640.0f, 0.0f, false);

        REQUIRE(level.platformCount() == 2); // implicit ground + authored
        CHECK(level.platforms()[0].bounds.min == logic::Vec2{0.0f, 0.0f});
        CHECK(level.platforms()[0].bounds.max == logic::Vec2{640.0f, 16.0f});
        CHECK(level.spawnPosition == logic::Vec2{320.0f, 16.0f});
        CHECK(nextOffset == doctest::Approx(360.0f));
        CHECK(data.platforms[0].min.y == doctest::Approx(20.0f));

        logic::Level second;
        logic::LevelData upper;
        upper.name = "Upper";
        upper.platforms.push_back({{100.0f, 20.0f}, {150.0f, 30.0f}});
        second.appendFromData(upper, 640.0f, 360.0f, false);
        CHECK(second.platformCount() == 1);
        CHECK(second.platforms()[0].bounds.min.y == doctest::Approx(380.0f));
    }

    TEST_CASE("runtime deriva objetivo apenas quando este chunk e final") {
        logic::LevelData data;
        data.name = "Final";
        data.platforms = {
            {{100.0f, 80.0f}, {160.0f, 96.0f}},
            {{300.0f, 200.0f}, {460.0f, 220.0f}},
        };

        logic::Level level;
        level.appendFromData(data, 640.0f, 0.0f, true);

        REQUIRE(level.hasFlag);
        CHECK(level.flagBounds.min.x == doctest::Approx(300.0f));
        CHECK(level.flagBounds.min.y == doctest::Approx(220.0f));
        CHECK(level.flagBounds.max.x == doctest::Approx(460.0f));
        CHECK(level.flagBounds.max.y == doctest::Approx(260.0f));
    }
}
