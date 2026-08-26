#include "doctest/doctest.h"
#include "Logic/Level.h"
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

using namespace logic;

namespace {

class TempLevelFile {
public:
    TempLevelFile(const std::string& stem, const std::string& contents) {
        static std::atomic<unsigned long long> counter{0};
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto suffix = counter.fetch_add(1, std::memory_order_relaxed);

        path_ = std::filesystem::temp_directory_path() /
                ("ascendendo_" + stem + "_" + std::to_string(timestamp) +
                 "_" + std::to_string(suffix) + ".lvl");

        std::ofstream file(path_);
        if (file) {
            file << contents;
            valid_ = file.good();
        }
    }

    ~TempLevelFile() {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
    }

    TempLevelFile(const TempLevelFile&) = delete;
    TempLevelFile& operator=(const TempLevelFile&) = delete;

    bool valid() const { return valid_; }
    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
    bool valid_ = false;
};

} // namespace

TEST_SUITE("Level / File Loading") {
    TEST_CASE("appendFromFile: ficheiro inexistente retorna offsetY inalterado") {
        Level level;
        float result = level.appendFromFile("nao_existe_mesmo.lvl", 640.0f, 0.0f);
        CHECK(result == doctest::Approx(0.0f));
        CHECK(level.platformCount() == 0);
    }

    TEST_CASE("appendFromFile: PLATFORM carregada com offset Y correto") {
        TempLevelFile tmp("platform", "# teste temporario\nNAME TestPlatform\nPLATFORM 50.0 100.0 100.0 15.0\n");
        REQUIRE(tmp.valid());

        Level level;
        level.appendFromFile(tmp.path().string(), 640.0f, 0.0f);
        REQUIRE(level.platformCount() == 1);
        CHECK(level.platforms()[0].bounds.min.x == doctest::Approx(50.0f));
        CHECK(level.platforms()[0].bounds.min.y == doctest::Approx(100.0f));
        CHECK(level.platforms()[0].bounds.max.y == doctest::Approx(115.0f));
    }

    TEST_CASE("appendFromFile: FLAG define hasFlag e flagBounds") {
        TempLevelFile tmp("flag", "FLAG 150.0 300.0 40.0 40.0\n");
        REQUIRE(tmp.valid());

        Level level;
        level.appendFromFile(tmp.path().string(), 640.0f, 0.0f);
        CHECK(level.hasFlag == true);
        CHECK(level.flagBounds.min.x == doctest::Approx(150.0f));
        CHECK(level.flagBounds.min.y == doctest::Approx(300.0f));
        CHECK(level.flagBounds.max.x == doctest::Approx(190.0f));
        CHECK(level.flagBounds.max.y == doctest::Approx(340.0f));
    }

    TEST_CASE("appendFromFile: segundo chunk usa offsetY = offsetY_anterior + LOGICAL_HEIGHT") {
        TempLevelFile tmp("stack", "PLATFORM 0.0 100.0 200.0 15.0\n");
        REQUIRE(tmp.valid());

        Level level;
        float nextY = level.appendFromFile(tmp.path().string(), 640.0f, 0.0f);
        CHECK(nextY == doctest::Approx(360.0f));
        level.appendFromFile(tmp.path().string(), 640.0f, nextY);
        REQUIRE(level.platformCount() == 2);
        CHECK(level.platforms()[1].bounds.min.y == doctest::Approx(460.0f));
    }

    TEST_CASE("appendFromFile: avanco do chunk e identico mesmo sem plataformas") {
        TempLevelFile tmpEmpty("empty", "# nivel sem plataformas\n");
        TempLevelFile tmpFull(
            "full",
            "PLATFORM 0.0 0.0 640.0 20.0\n"
            "PLATFORM 230.0 95.0 180.0 20.0\n"
            "PLATFORM 30.0 195.0 180.0 20.0\n"
            "PLATFORM 230.0 295.0 180.0 20.0\n");
        REQUIRE(tmpEmpty.valid());
        REQUIRE(tmpFull.valid());

        Level levelEmpty, levelFull;
        float nextEmpty = levelEmpty.appendFromFile(tmpEmpty.path().string(), 640.0f, 0.0f);
        float nextFull = levelFull.appendFromFile(tmpFull.path().string(), 640.0f, 0.0f);
        CHECK(nextEmpty == doctest::Approx(360.0f));
        CHECK(nextFull == doctest::Approx(360.0f));
        CHECK(nextEmpty == doctest::Approx(nextFull));
    }
}
