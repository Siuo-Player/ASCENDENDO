#include "doctest/doctest.h"
#include "Core/CampaignLoader.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TempTree {
public:
    TempTree() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        root_ = std::filesystem::temp_directory_path() /
                ("ascendendo-campaign-loader-" + std::to_string(stamp));
        std::filesystem::create_directories(root_);
    }

    ~TempTree() {
        std::error_code ec;
        std::filesystem::remove_all(root_, ec);
    }

    const std::filesystem::path& root() const { return root_; }

private:
    std::filesystem::path root_;
};

} // namespace

TEST_SUITE("Campaign Loader") {
    TEST_CASE("preserves order and ignores comments and blank lines") {
        TempTree temp;
        const auto campaignFile = temp.root() / "campaign.txt";
        const auto levelsRoot = temp.root() / "Levels";

        std::ofstream file(campaignFile);
        REQUIRE(file.is_open());
        file << "# header\n"
             << "\n"
             << "inicio.lvl\n"
             << "zigzag.lvl\r\n"
             << "# ignored\n"
             << "precipicio.lvl\n";
        file.close();

        const auto levels = core::CampaignLoader::load(campaignFile, levelsRoot);

        REQUIRE(levels.size() == 3);
        CHECK(levels[0] == levelsRoot / "inicio.lvl");
        CHECK(levels[1] == levelsRoot / "zigzag.lvl");
        CHECK(levels[2] == levelsRoot / "precipicio.lvl");
    }

    TEST_CASE("missing campaign file yields an empty campaign") {
        TempTree temp;

        const auto levels = core::CampaignLoader::load(
            temp.root() / "missing.txt",
            temp.root() / "Levels");

        CHECK(levels.empty());
    }

    TEST_CASE("explicit roots keep results independent of current directory") {
        const auto levels = core::CampaignLoader::load(
            "/install/ascendendo/Game/Assets/Levels/campaign.txt",
            "/install/ascendendo/Game/Assets/Levels");

        CHECK(levels.empty());
    }
}
