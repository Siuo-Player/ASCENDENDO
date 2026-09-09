#include "doctest/doctest.h"
#include "Core/LevelLayout.h"
#include "Logic/LevelDataIO.h"
#include "Logic/LevelEditor.h"

#include <filesystem>

TEST_SUITE("Foundation — vertical level layout") {

TEST_CASE("um Level tem largura fixa de 640 e uma altura composta por N screens") {
    const core::LevelLayout layout(5);

    CHECK(layout.screenCount() == 5);
    CHECK(layout.width() == doctest::Approx(640.0f));
    CHECK(layout.height() == doctest::Approx(1800.0f));
    CHECK(layout.screenBottomY(0) == doctest::Approx(0.0f));
    CHECK(layout.screenTopY(0) == doctest::Approx(360.0f));
    CHECK(layout.screenBottomY(4) == doctest::Approx(1440.0f));
    CHECK(layout.screenTopY(4) == doctest::Approx(1800.0f));
}

TEST_CASE("LevelLayout mantém a largura imutável e mapeia Y para a screen ascendente") {
    const core::LevelLayout layout(3);

    CHECK(layout.containsX(0.0f));
    CHECK(layout.containsX(640.0f));
    CHECK_FALSE(layout.containsX(-0.01f));
    CHECK_FALSE(layout.containsX(640.01f));

    CHECK(layout.validScreen(0));
    CHECK(layout.validScreen(2));
    CHECK_FALSE(layout.validScreen(3));

    CHECK(layout.screenIndexForY(0.0f) == 0);
    CHECK(layout.screenIndexForY(359.9f) == 0);
    CHECK(layout.screenIndexForY(360.0f) == 1);
    CHECK(layout.screenIndexForY(719.9f) == 1);
    CHECK(layout.screenIndexForY(720.0f) == 2);
    CHECK(layout.screenIndexForY(99999.0f) == 2);
}

TEST_CASE("LevelEditorDocument aceita N screens sem alterar a largura") {
    logic::LevelEditorDocument document(
        false,
        logic::AABB{{0.0f, 0.0f}, {640.0f, 20.0f}},
        5);

    CHECK(document.screenCount() == 5);
    CHECK(document.levelWidth() == doctest::Approx(640.0f));
    CHECK(document.levelHeight() == doctest::Approx(1800.0f));

    REQUIRE(document.addPlatform({{96.0f, 1320.0f}, {224.0f, 1340.0f}}));
    CHECK(document.platforms().back().bounds.min.y == doctest::Approx(1320.0f));
    CHECK(document.platforms().back().bounds.max.x == doctest::Approx(224.0f));

    CHECK_FALSE(document.addPlatform({{0.0f, 0.0f}, {644.0f, 20.0f}}));
    CHECK_FALSE(document.addPlatform({{0.0f, 1780.0f}, {64.0f, 24.0f}}));
}

TEST_CASE("FLAG de level final é restrita à última screen") {
    logic::LevelEditorDocument document(
        true,
        logic::AABB{{0.0f, 0.0f}, {640.0f, 20.0f}},
        5);

    CHECK_FALSE(document.setFlag({{320.0f, 100.0f}, {384.0f, 116.0f}}));
    REQUIRE(document.setFlag({{320.0f, 1440.0f}, {384.0f, 1456.0f}}));
    CHECK(document.hasFlag());
    CHECK(document.flag()->min.y >= doctest::Approx(1440.0f));
}

TEST_CASE("SCREENS é persistido e níveis antigos continuam a carregar como uma screen") {
    const auto path = std::filesystem::temp_directory_path() / "ascendendo-level-layout-test.lvl";
    const auto oldPath = std::filesystem::temp_directory_path() / "ascendendo-level-layout-old-test.lvl";
    std::error_code ec;
    std::filesystem::remove(path, ec);
    std::filesystem::remove(oldPath, ec);

    logic::LevelData data;
    data.name = "Vertical Test";
    data.screenCount = 5;
    data.spawnPosition = logic::Vec2{96.0f, 20.0f};
    data.platforms.push_back({{0.0f, 0.0f}, {640.0f, 20.0f}});
    data.platforms.push_back({{96.0f, 1320.0f}, {224.0f, 1340.0f}});

    REQUIRE(logic::LevelDataIO::save(data, path));
    const auto loaded = logic::LevelDataIO::load(path);
    REQUIRE(loaded.has_value());
    CHECK(loaded->screenCount == 5);

    {
        std::ofstream old(oldPath);
        REQUIRE(old.is_open());
        old << "NAME Legacy\n";
        old << "SPAWN 96 20\n";
        old << "PLATFORM 0 0 640 20\n";
    }
    const auto legacy = logic::LevelDataIO::load(oldPath);
    REQUIRE(legacy.has_value());
    CHECK(legacy->screenCount == 1);

    std::filesystem::remove(path, ec);
    std::filesystem::remove(oldPath, ec);
}

} // TEST_SUITE
