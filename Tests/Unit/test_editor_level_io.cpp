#include "doctest/doctest.h"
#include "Logic/EditorLevelIO.h"
#include "Logic/LevelDataIO.h"
#include "Logic/LevelEditor.h"

#include <filesystem>
#include <fstream>
#include <string>

TEST_SUITE("Fase 9.5 — EditorLevelIO") {

TEST_CASE("saveEditorLevel grava uma representação canónica que pode ser reaberta") {
    const auto path = std::filesystem::temp_directory_path() / "ascendendo-editor-save-test.lvl";
    std::error_code ec;
    std::filesystem::remove(path, ec);

    logic::LevelEditorDocument document(false, logic::AABB{{0.0f, 0.0f}, {640.0f, 20.0f}});
    REQUIRE(document.addPlatform({{96.0f, 80.0f}, {224.0f, 100.0f}}));
    REQUIRE(document.setSpawnX(128.0f));

    REQUIRE(logic::saveEditorLevel(document, path.string(), "IO Roundtrip"));

    const auto loaded = logic::LevelDataIO::load(path);
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->name == "IO Roundtrip");
    REQUIRE(loaded->spawnPosition.has_value());
    REQUIRE(loaded->platforms.size() == 2);
    CHECK(loaded->platforms[1].min.x == doctest::Approx(96.0f));
    CHECK(loaded->platforms[1].min.y == doctest::Approx(80.0f));

    std::filesystem::remove(path, ec);
}

TEST_CASE("saveEditorLevel não deixa um ficheiro temporário depois de uma gravação válida") {
    const auto path = std::filesystem::temp_directory_path() / "ascendendo-editor-save-temp-test.lvl";
    const auto temporary = path.parent_path() / (path.filename().string() + ".tmp-save");
    std::error_code ec;
    std::filesystem::remove(path, ec);
    std::filesystem::remove(temporary, ec);

    logic::LevelEditorDocument document(false, logic::AABB{{0.0f, 0.0f}, {640.0f, 20.0f}});
    REQUIRE(logic::saveEditorLevel(document, path.string(), "Atomic Save"));

    CHECK(std::filesystem::exists(path));
    CHECK_FALSE(std::filesystem::exists(temporary));

    std::filesystem::remove(path, ec);
}

} // TEST_SUITE
