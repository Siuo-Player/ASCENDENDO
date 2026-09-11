#include "doctest/doctest.h"
#include "Logic/LevelEditor.h"
#include "Core/Config.h"

using namespace logic;

TEST_SUITE("Fase 9.4 — LevelEditorDocument") {

TEST_CASE("plataforma authored preserva coordenadas e fica acima do chão") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    std::size_t index = 999;

    const AABB authored{{3.25f,21.5f},{131.75f,41.25f}};
    REQUIRE(doc.addPlatform(authored, &index));
    CHECK(index == 0);
    CHECK(doc.platformCount() == 1);
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(3.25f));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(21.5f));
    CHECK(doc.platforms()[0].bounds.max.x == doctest::Approx(131.75f));
    CHECK(doc.platforms()[0].bounds.max.y == doctest::Approx(41.25f));
}

TEST_CASE("plataforma que invade o chão é rejeitada") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    CHECK_FALSE(doc.addPlatform(AABB{{0,0},{64,16}}));
    CHECK_FALSE(doc.addPlatform(AABB{{0,15.9f},{64,31.9f}}));
    CHECK(doc.addPlatform(AABB{{0,16},{64,32}}));
    CHECK(doc.platformCount() == 1);
}

TEST_CASE("editor usa solo e spawn canónicos, ignorando chão legado") {
    LevelEditorDocument doc(false, AABB{{32,12},{320,28}});

    CHECK(doc.spawnPosition().x == doctest::Approx(config::LOGICAL_WIDTH / 2.0f));
    CHECK(doc.spawnPosition().y == doctest::Approx(16.0f));
    CHECK(doc.spawnMinX() == doctest::Approx(320.0f));
    CHECK(doc.spawnMaxX() == doctest::Approx(320.0f));

    CHECK_FALSE(doc.setSpawnX(96.25f));
    CHECK(doc.spawnPosition().x == doctest::Approx(320.0f));
    CHECK(doc.spawnPosition().y == doctest::Approx(16.0f));
}

TEST_CASE("FLAG é sempre derivada e segue a plataforma authored mais alta") {
    LevelEditorDocument finalLevel(true, AABB{{0,0},{640,20}});
    REQUIRE(finalLevel.addPlatform(AABB{{100,80},{228,96}}));
    REQUIRE(finalLevel.addPlatform(AABB{{360,240},{520,256}}));

    REQUIRE(finalLevel.hasFlag());
    CHECK(finalLevel.flag()->min.x == doctest::Approx(360.0f));
    CHECK(finalLevel.flag()->min.y == doctest::Approx(256.0f));
    CHECK(finalLevel.flag()->max.x == doctest::Approx(520.0f));
    CHECK(finalLevel.flag()->max.y == doctest::Approx(296.0f));

    CHECK_FALSE(finalLevel.setFlag(AABB{{10,100},{80,140}}));
    finalLevel.removeFlag();
    CHECK(finalLevel.hasFlag());

    REQUIRE(finalLevel.movePlatform(1, {300,320}));
    REQUIRE(finalLevel.hasFlag());
    CHECK(finalLevel.flag()->min.x == doctest::Approx(300.0f));
    CHECK(finalLevel.flag()->min.y == doctest::Approx(336.0f));
    CHECK(finalLevel.flag()->max.x == doctest::Approx(520.0f));
    CHECK(finalLevel.flag()->max.y == doctest::Approx(376.0f));
}

TEST_CASE("nível não final não tem objetivo derivado") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    REQUIRE(doc.addPlatform(AABB{{300,240},{500,256}}));
    CHECK_FALSE(doc.hasFlag());
    CHECK(doc.flag() == nullptr);
}

TEST_CASE("toLevelData serializa apenas geometria authored") {
    LevelEditorDocument doc(true, AABB{{0,0},{640,20}});
    REQUIRE(doc.addPlatform(AABB{{100,80},{228,100}}));

    const LevelData data = doc.toLevelData("Teste");
    CHECK(data.name == "Teste");
    CHECK(data.platforms.size() == 1);
    CHECK(data.platforms.front().min.y == doctest::Approx(80.0f));
    CHECK_FALSE(data.spawnPosition.has_value());
    CHECK_FALSE(data.flag.has_value());
}

TEST_CASE("restore rejeita SPAWN e FLAG authored e aceita geometria pura") {
    LevelEditorDocument doc(true, AABB{{0,0},{640,20}});

    LevelData invalidSpawn;
    invalidSpawn.platforms = {AABB{{100,80},{228,100}}};
    invalidSpawn.spawnPosition = Vec2{96,16};
    CHECK_FALSE(doc.restoreFromLevelData(invalidSpawn));

    LevelData invalidFlag;
    invalidFlag.platforms = {AABB{{100,80},{228,100}}};
    invalidFlag.flag = AABB{{100,100},{228,140}};
    CHECK_FALSE(doc.restoreFromLevelData(invalidFlag));

    LevelData valid;
    valid.name = "Puro";
    valid.platforms = {
        AABB{{80,40},{160,56}},
        AABB{{360,240},{520,256}},
    };
    REQUIRE(doc.restoreFromLevelData(valid));
    CHECK(doc.platformCount() == 2);
    CHECK(doc.spawnPosition().x == doctest::Approx(320.0f));
    CHECK(doc.spawnPosition().y == doctest::Approx(16.0f));
    REQUIRE(doc.flag() != nullptr);
    CHECK(doc.flag()->min.x == doctest::Approx(360.0f));
    CHECK(doc.flag()->min.y == doctest::Approx(256.0f));
    CHECK(doc.flag()->max.y == doctest::Approx(296.0f));
}

TEST_CASE("operações válidas incrementam a geração e operações derivadas não mutam") {
    LevelEditorDocument doc(true, AABB{{0,0},{640,20}});
    CHECK(doc.generation() == 0);

    REQUIRE(doc.addPlatform(AABB{{64.25f,40.5f},{192.75f,60.25f}}));
    CHECK(doc.generation() == 1);

    CHECK_FALSE(doc.setSpawnX(120.5f));
    CHECK_FALSE(doc.setFlag(AABB{{400,300},{464,340}}));
    doc.removeFlag();
    CHECK(doc.generation() == 1);

    CHECK_FALSE(doc.movePlatform(0, {64.25f,15.0f}));
    CHECK(doc.generation() == 1);

    REQUIRE(doc.movePlatform(0, {100.5f,80.25f}));
    CHECK(doc.generation() == 2);
    REQUIRE(doc.removePlatform(0));
    CHECK(doc.generation() == 3);
}

TEST_CASE("presets mantêm tamanhos determinísticos") {
    const Vec2 small = LevelEditorDocument::presetSize(EditorSizePreset::SMALL);
    const Vec2 medium = LevelEditorDocument::presetSize(EditorSizePreset::MEDIUM);
    const Vec2 large = LevelEditorDocument::presetSize(EditorSizePreset::LARGE);

    CHECK(small.x == doctest::Approx(64.0f));
    CHECK(small.y == doctest::Approx(16.0f));
    CHECK(medium.x == doctest::Approx(128.0f));
    CHECK(medium.y == doctest::Approx(20.0f));
    CHECK(large.x == doctest::Approx(192.0f));
    CHECK(large.y == doctest::Approx(24.0f));
}

} // namespace
