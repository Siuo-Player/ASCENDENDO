#include "doctest/doctest.h"
#include "Core/RuntimeBootstrap.h"

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
                ("ascendendo-runtime-bootstrap-" + std::to_string(stamp));
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

void writeFile(const std::filesystem::path& path, const std::string& contents) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream file(path);
    REQUIRE(file.is_open());
    file << contents;
    REQUIRE(file.good());
}

} // namespace

TEST_SUITE("Runtime Bootstrap") {
    TEST_CASE("prepares paths, campaign and user directories from explicit roots") {
        TempTree temp;
        const auto executableRoot = temp.root() / "install";
        const auto userDataRoot = temp.root() / "user-data";
        const auto levelsRoot = executableRoot / "Game" / "Assets" / "Levels";

        writeFile(levelsRoot / "campaign.txt", "inicio.lvl\n# comment\nsegundo.lvl\n");
        writeFile(levelsRoot / "inicio.lvl", "NAME Inicio\nPLATFORM 0 0 640 20\n");
        writeFile(levelsRoot / "segundo.lvl", "NAME Segundo\nPLATFORM 0 0 640 20\n");

        const auto result = core::RuntimeBootstrap::prepare(
            core::RuntimePaths(executableRoot, userDataRoot));

        CHECK(result.userDirectoriesReady);
        CHECK(result.paths.executableRoot() == executableRoot);
        CHECK(result.paths.userDataRoot() == userDataRoot);
        REQUIRE(result.campaign.size() == 2);
        CHECK(result.campaign[0] == levelsRoot / "inicio.lvl");
        CHECK(result.campaign[1] == levelsRoot / "segundo.lvl");
        CHECK(!result.campaignID.empty());
        CHECK(result.campaignID.size() == 16);
        CHECK(std::filesystem::is_directory(result.controlsFile().parent_path()));
        CHECK(std::filesystem::is_directory(result.runsFile().parent_path()));
    }

    TEST_CASE("same campaign bytes produce the same bootstrap identity") {
        TempTree temp;
        const auto executableRoot = temp.root() / "install";
        const auto levelsRoot = executableRoot / "Game" / "Assets" / "Levels";
        const auto userA = temp.root() / "user-a";
        const auto userB = temp.root() / "user-b";

        writeFile(levelsRoot / "campaign.txt", "a.lvl\nb.lvl\n");
        writeFile(levelsRoot / "a.lvl", "NAME A\nPLATFORM 0 0 640 20\n");
        writeFile(levelsRoot / "b.lvl", "NAME B\nPLATFORM 0 100 640 20\n");

        const auto first = core::RuntimeBootstrap::prepare(
            core::RuntimePaths(executableRoot, userA));
        const auto second = core::RuntimeBootstrap::prepare(
            core::RuntimePaths(executableRoot, userB));

        REQUIRE(!first.campaignID.empty());
        CHECK(first.campaignID == second.campaignID);
    }

    TEST_CASE("missing campaign remains observable as an empty campaign") {
        TempTree temp;
        const auto result = core::RuntimeBootstrap::prepare(
            core::RuntimePaths(temp.root() / "install", temp.root() / "user-data"));

        CHECK(result.userDirectoriesReady);
        CHECK(result.campaign.empty());
        CHECK(result.campaignID.empty());
    }
}
