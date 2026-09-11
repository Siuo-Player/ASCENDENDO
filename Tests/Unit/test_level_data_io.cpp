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
                ("ascendendo-level-io-" + std::to_string(stamp) + ".lvl");
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

TEST_SUITE("LevelDataIO current grammar") {

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
        TempLevelFile file("PLATFORM 20 20 10 10 unexpected\n");
        CHECK_FALSE(logic::LevelDataIO::load(file.path()).has_value());
    }

    TEST_CASE("authored flag is rejected") {
        TempLevelFile file("FLAG 20 20 8 8\n");
        CHECK_FALSE(logic::LevelDataIO::load(file.path()).has_value());
    }

    TEST_CASE("authored spawn is rejected") {
        TempLevelFile file("SPAWN 320 16\n");
        CHECK_FALSE(logic::LevelDataIO::load(file.path()).has_value());
    }

    TEST_CASE("valid current grammar remains accepted") {
        TempLevelFile file(
            "NAME Valid\n"
            "PLATFORM 100 40 100 20\n");

        const auto data = logic::LevelDataIO::load(file.path());
        REQUIRE(data.has_value());
        CHECK(data->name == "Valid");
        CHECK_FALSE(data->spawnPosition.has_value());
        CHECK_FALSE(data->flag.has_value());
        REQUIRE(data->platforms.size() == 1);
        CHECK(data->platforms.front().min.y == 40.0f);
    }
}
