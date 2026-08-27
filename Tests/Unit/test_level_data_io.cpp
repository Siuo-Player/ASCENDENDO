#include "doctest/doctest.h"
#include "Logic/LevelDataIO.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TempLevelFile {
public:
    explicit TempLevelFile(const std::string& contents) {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
                ("ascendendo-malformed-" + std::to_string(stamp) + ".lvl");
        std::ofstream out(path_);
        REQUIRE(out.is_open());
        out << contents;
        REQUIRE(out.good());
    }

    ~TempLevelFile() {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }

    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

} // namespace

TEST_SUITE("LevelDataIO malformed syntax") {

    TEST_CASE("unknown line type is rejected") {
        TempLevelFile file("NAME Test\nUNKNOWN 1 2 3\n");
        CHECK_FALSE(logic::LevelDataIO::load(file.path()).has_value());
    }

    TEST_CASE("invalid numeric token is rejected") {
        TempLevelFile file("PLATFORM nope 0 10 10\n");
        CHECK_FALSE(logic::LevelDataIO::load(file.path()).has_value());
    }

    TEST_CASE("truncated platform is rejected") {
        TempLevelFile file("PLATFORM 0 0 10\n");
        CHECK_FALSE(logic::LevelDataIO::load(file.path()).has_value());
    }

    TEST_CASE("trailing token after platform is rejected") {
        TempLevelFile file("PLATFORM 0 0 10 10 unexpected\n");
        CHECK_FALSE(logic::LevelDataIO::load(file.path()).has_value());
    }

    TEST_CASE("trailing token after flag is rejected") {
        TempLevelFile file("FLAG 0 0 8 8 unexpected\n");
        CHECK_FALSE(logic::LevelDataIO::load(file.path()).has_value());
    }

    TEST_CASE("trailing token after spawn is rejected") {
        TempLevelFile file("SPAWN 12 24 unexpected\n");
        CHECK_FALSE(logic::LevelDataIO::load(file.path()).has_value());
    }

    TEST_CASE("valid current grammar remains accepted") {
        TempLevelFile file(
            "NAME Valid\n"
            "SPAWN 16 32\n"
            "PLATFORM 0 40 100 20\n"
            "FLAG 80 20 8 20\n");

        const auto data = logic::LevelDataIO::load(file.path());
        REQUIRE(data.has_value());
        CHECK(data->name == "Valid");
        REQUIRE(data->spawnPosition.has_value());
        CHECK(data->spawnPosition->x == 16.0f);
        CHECK(data->spawnPosition->y == 32.0f);
        REQUIRE(data->platforms.size() == 1);
        REQUIRE(data->flag.has_value());
    }
}
