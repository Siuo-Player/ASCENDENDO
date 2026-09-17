#include "doctest/doctest.h"

#include "Core/CampaignCatalogue.h"
#include "Core/CampaignLoader.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TempDirectory {
public:
    TempDirectory() {
        const auto now = std::chrono::steady_clock::now().time_since_epoch().count();
        root_ = std::filesystem::temp_directory_path() /
                ("ascendendo-campaign-catalogue-" + std::to_string(now));
        std::filesystem::create_directories(root_);
    }

    ~TempDirectory() {
        std::error_code ec;
        std::filesystem::remove_all(root_, ec);
    }

    const std::filesystem::path& path() const { return root_; }

private:
    std::filesystem::path root_;
};

void writeText(const std::filesystem::path& path, const std::string& contents) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::trunc);
    REQUIRE(file.is_open());
    file << contents;
    REQUIRE(file.good());
}

} // namespace

TEST_SUITE("Campaign Catalogue") {

TEST_CASE("loads ordered campaign descriptors from the canonical catalogue format") {
    TempDirectory temp;
    const auto assets = temp.path() / "Assets";
    const auto catalogue = assets / "Campaigns" / "catalogue.txt";
    const auto firstPlaylist = assets / "Campaigns" / "01-easy" / "campaign.txt";
    const auto secondPlaylist = assets / "Campaigns" / "02-hard" / "campaign.txt";

    writeText(catalogue,
              "# id|name|playlist\n"
              "easy|Primeira Ascensao|01-easy/campaign.txt\n"
              "hard|Subida Final|02-hard/campaign.txt\n");
    writeText(firstPlaylist, "easy-01.lvl\n");
    writeText(secondPlaylist, "hard-01.lvl\n");

    const auto result = core::CampaignCatalogue::load(catalogue, assets);

    CHECK(result.exists);
    CHECK(result.valid());
    REQUIRE(result.campaigns.size() == 2);
    CHECK(result.campaigns[0].id == "easy");
    CHECK(result.campaigns[0].name == "Primeira Ascensao");
    CHECK(result.campaigns[0].playlistFile == std::filesystem::weakly_canonical(firstPlaylist));
    CHECK(result.campaigns[1].id == "hard");
}

TEST_CASE("rejects duplicate ids, duplicate playlists and path traversal") {
    TempDirectory temp;
    const auto assets = temp.path() / "Assets";
    const auto catalogue = assets / "Campaigns" / "catalogue.txt";
    const auto playlist = assets / "Campaigns" / "main" / "campaign.txt";

    writeText(catalogue,
              "one|Campanha 1|main/campaign.txt\n"
              "one|Campanha duplicada|main/campaign.txt\n"
              "../escape|Fora|../outside/campaign.txt\n");
    writeText(playlist, "level.lvl\n");

    const auto result = core::CampaignCatalogue::load(catalogue, assets);

    CHECK(result.exists);
    CHECK_FALSE(result.valid());
    CHECK(result.campaigns.size() == 1);
    CHECK(result.errors.size() == 2);
}

TEST_CASE("resolves campaign playlist level paths relative to the playlist") {
    TempDirectory temp;
    const auto campaignFile = temp.path() / "campaigns" / "demo" / "campaign.txt";
    const auto levelFile = campaignFile.parent_path() / "levels" / "level01.lvl";
    writeText(campaignFile, "levels/level01.lvl\n");
    writeText(levelFile, "PLATFORM 100 64 100 16\n");

    const auto levels = core::CampaignLoader::load(campaignFile);

    REQUIRE(levels.size() == 1);
    CHECK(levels.front() == std::filesystem::weakly_canonical(levelFile));
}

TEST_CASE("missing optional catalogue leaves migration fallback available") {
    TempDirectory temp;
    const auto result = core::CampaignCatalogue::load(
        temp.path() / "Assets" / "Campaigns" / "catalogue.txt",
        temp.path() / "Assets");

    CHECK_FALSE(result.exists);
    CHECK(result.valid());
    CHECK(result.campaigns.empty());
    CHECK(result.errors.empty());
}

} // TEST_SUITE
