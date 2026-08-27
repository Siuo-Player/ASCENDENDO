#include "doctest/doctest.h"
#include "Logic/Level.h"
#include "Logic/LevelDataIO.h"

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

bool appendFile(Level& level, const std::filesystem::path& path,
                float maxWidth, float offsetY, float* nextOffset = nullptr) {
    const auto data = LevelDataIO::load(path);
    if (!data) return false;
    const float next = level.appendFromData(*data, maxWidth, offsetY);
    if (nextOffset) *nextOffset = next;
    return true;
}

} // namespace

TEST_SUITE("Level / File Loading") {
    TEST_CASE("ficheiro inexistente nao cria LevelData") {
        const auto data = LevelDataIO::load("nao_existe_mesmo.lvl");
        CHECK(!data.has_value());
    }

    TEST_CASE("PLATFORM e carregada com offset Y correto") {
        TempLevelFile tmp("platform", "# teste temporario\nNAME TestPlatform\nPLATFORM 50.0 100.0 100.0 15.0\n");
        REQUIRE(tmp.valid());

        const auto data = LevelDataIO::load(tmp.path());
        REQUIRE(data.has_value());

        Level level;
        level.appendFromData(*data, 640.0f, 0.0f);
        REQUIRE(level.platformCount() == 1);
        CHECK(level.platforms()[0].bounds.min.x == doctest::Approx(50.0f));
        CHECK(level.platforms()[0].bounds.min.y == doctest::Approx(100.0f));
        CHECK(level.platforms()[0].bounds.max.y == doctest::Approx(115.0f));
    }

    TEST_CASE("FLAG define hasFlag e flagBounds") {
        TempLevelFile tmp("flag", "FLAG 150.0 300.0 40.0 40.0\n");
        REQUIRE(tmp.valid());

        const auto data = LevelDataIO::load(tmp.path());
        REQUIRE(data.has_value());
        REQUIRE(data->flag.has_value());

        Level level;
        level.appendFromData(*data, 640.0f, 0.0f);
        CHECK(level.hasFlag == true);
        CHECK(level.flagBounds.min.x == doctest::Approx(150.0f));
        CHECK(level.flagBounds.min.y == doctest::Approx(300.0f));
        CHECK(level.flagBounds.max.x == doctest::Approx(190.0f));
        CHECK(level.flagBounds.max.y == doctest::Approx(340.0f));
    }

    TEST_CASE("segundo chunk usa offsetY = offsetY anterior + LOGICAL_HEIGHT") {
        TempLevelFile tmp("stack", "PLATFORM 0.0 100.0 200.0 15.0\n");
        REQUIRE(tmp.valid());

        Level level;
        float nextY = 0.0f;
        REQUIRE(appendFile(level, tmp.path(), 640.0f, 0.0f, &nextY));
        CHECK(nextY == doctest::Approx(360.0f));
        REQUIRE(appendFile(level, tmp.path(), 640.0f, nextY));
        REQUIRE(level.platformCount() == 2);
        CHECK(level.platforms()[1].bounds.min.y == doctest::Approx(460.0f));
    }

    TEST_CASE("avanco do chunk e identico mesmo sem plataformas") {
        TempLevelFile tmpEmpty("empty", "# nivel sem plataformas\n");
        TempLevelFile tmpFull(
            "full",
            "PLATFORM 0.0 0.0 640.0 20.0\n"
            "PLATFORM 230.0 95.0 180.0 20.0\n"
            "PLATFORM 30.0 195.0 180.0 20.0\n"
            "PLATFORM 230.0 295.0 180.0 20.0\n");
        REQUIRE(tmpEmpty.valid());
        REQUIRE(tmpFull.valid());

        const auto emptyData = LevelDataIO::load(tmpEmpty.path());
        const auto fullData = LevelDataIO::load(tmpFull.path());
        REQUIRE(emptyData.has_value());
        REQUIRE(fullData.has_value());

        Level levelEmpty, levelFull;
        const float nextEmpty = levelEmpty.appendFromData(*emptyData, 640.0f, 0.0f);
        const float nextFull = levelFull.appendFromData(*fullData, 640.0f, 0.0f);
        CHECK(nextEmpty == doctest::Approx(360.0f));
        CHECK(nextFull == doctest::Approx(360.0f));
        CHECK(nextEmpty == doctest::Approx(nextFull));
    }
}
