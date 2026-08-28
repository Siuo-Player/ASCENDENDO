#include "doctest/doctest.h"
#include "Core/RuntimePaths.h"

#include <chrono>
#include <filesystem>
#include <string>

namespace {

class TempTree {
public:
    TempTree() {
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        root_ = std::filesystem::temp_directory_path() /
                ("ascendendo-runtime-paths-" + std::to_string(stamp));
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

class CurrentPathGuard {
public:
    explicit CurrentPathGuard(const std::filesystem::path& path)
        : previous_(std::filesystem::current_path()) {
        std::filesystem::current_path(path);
    }

    ~CurrentPathGuard() {
        std::error_code ec;
        std::filesystem::current_path(previous_, ec);
    }

private:
    std::filesystem::path previous_;
};

} // namespace

TEST_SUITE("Runtime Paths") {
    TEST_CASE("asset and user-data roots are distinct") {
        TempTree temp;
        const auto executableRoot = temp.root() / "app";
        const auto userDataRoot = temp.root() / "user";
        const core::RuntimePaths paths(executableRoot, userDataRoot);

        CHECK(paths.assetsRoot() == executableRoot / "Game" / "Assets");
        CHECK(paths.levelsRoot() == executableRoot / "Game" / "Assets" / "Levels");
        CHECK(paths.campaignFile() == executableRoot / "Game" / "Assets" / "Levels" / "campaign.txt");
        CHECK(paths.playerSprite() == executableRoot / "Game" / "Assets" / "Sprites" / "personagem.png");
        CHECK(paths.controlsFile() == userDataRoot / "Settings" / "controls.cfg");
        CHECK(paths.runsFile() == userDataRoot / "Runs" / "runs.csv");
        CHECK(paths.controlsFile().string().find("Development") == std::string::npos);
        CHECK(paths.runsFile().string().find("Development") == std::string::npos);
    }

    TEST_CASE("user-data directories are created outside the asset tree") {
        TempTree temp;
        const core::RuntimePaths paths(temp.root() / "app", temp.root() / "user-data");

        REQUIRE(paths.ensureUserDirectories());

        CHECK(std::filesystem::is_directory(paths.controlsFile().parent_path()));
        CHECK(std::filesystem::is_directory(paths.runsFile().parent_path()));
        CHECK(std::filesystem::is_directory(paths.assetsRoot()) == false);
    }

    TEST_CASE("explicit roots make path resolution independent of current directory") {
        const core::RuntimePaths paths("/install/ascendendo", "/user/ascendendo");

        CHECK(paths.campaignFile() == std::filesystem::path("/install/ascendendo/Game/Assets/Levels/campaign.txt"));
        CHECK(paths.controlsFile() == std::filesystem::path("/user/ascendendo/Settings/controls.cfg"));
    }

    TEST_CASE("process paths are independent of current directory") {
        TempTree temp;
        const auto firstWorkingDirectory = temp.root() / "first-cwd";
        const auto secondWorkingDirectory = temp.root() / "second-cwd";
        REQUIRE(std::filesystem::create_directories(firstWorkingDirectory));
        REQUIRE(std::filesystem::create_directories(secondWorkingDirectory));

        core::RuntimePaths firstPaths = [&] {
            CurrentPathGuard guard(firstWorkingDirectory);
            return core::RuntimePaths::fromProcess(nullptr);
        }();
        core::RuntimePaths secondPaths = [&] {
            CurrentPathGuard guard(secondWorkingDirectory);
            return core::RuntimePaths::fromProcess(nullptr);
        }();

        CHECK(firstPaths.executableRoot() == secondPaths.executableRoot());
        CHECK(firstPaths.assetsRoot() == secondPaths.assetsRoot());
        CHECK(firstPaths.levelsRoot() == secondPaths.levelsRoot());
        CHECK(firstPaths.campaignFile() == secondPaths.campaignFile());
        CHECK(firstPaths.playerSprite() == secondPaths.playerSprite());
    }
}
