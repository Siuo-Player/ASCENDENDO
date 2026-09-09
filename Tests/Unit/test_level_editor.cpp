#include "doctest/doctest.h"
#include "Logic/LevelEditor.h"
#include "Core/Config.h"
#include <cmath>
#include <limits>

using namespace logic;

TEST_SUITE("Fase 9.4 — LevelEditorDocument") {

TEST_CASE("plataforma preserva coordenadas não alinhadas e fica dentro dos bounds") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    std::size_t index = 999;

    const AABB authored{{3.25f,21.5f},{131.75f,41.25f}};
    REQUIRE(doc.addPlatform(authored, &index));
    REQUIRE(index == 0);
    REQUIRE(doc.platformCount() == 1);

    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(3.25f));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(21.5f));
    CHECK(doc.platforms()[0].bounds.max.x == doctest::Approx(131.75f));
    CHECK(doc.platforms()[0].bounds.max.y == doctest::Approx(41.25f));
}

TEST_CASE("pedido de plataforma fora do canvas e rejeitado sem quantizacao") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});

    CHECK_FALSE(doc.addPlatform(AABB{{-1,40},{63,56}}));
    CHECK_FALSE(doc.addPlatform(AABB{{600,40},{641,56}}));
    CHECK(doc.platformCount() == 0);
}

TEST_CASE("plataforma pequena demais e rejeitada") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    CHECK_FALSE(doc.addPlatform(AABB{{0,0},{3,2}}));
    CHECK(doc.platformCount() == 0);
}

TEST_CASE("mover plataforma preserva dimensoes e coordenadas exatas") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    REQUIRE(doc.addPlatform(AABB{{100.25f,80.5f},{200.75f,100.75f}}));

    CHECK(doc.movePlatform(0, {300.75f,120.5f}));
    CHECK(doc.platforms()[0].bounds.width() == doctest::Approx(100.5f));
    CHECK(doc.platforms()[0].bounds.height() == doctest::Approx(20.25f));
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(300.75f));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(120.5f));

    CHECK_FALSE(doc.movePlatform(0, {600,340}));
    CHECK_FALSE(doc.movePlatform(0, {-1,120}));
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(300.75f));
}

TEST_CASE("remover plataforma valida indice") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    REQUIRE(doc.addPlatform(AABB{{0,40},{64,56}}));
    REQUIRE(doc.addPlatform(AABB{{80,80},{144,96}}));

    CHECK(doc.removePlatform(0));
    CHECK(doc.platformCount() == 1);
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(80.0f));
    CHECK_FALSE(doc.removePlatform(5));
}

TEST_CASE("spawn preserva coordenada X exata e Y do solo") {
    LevelEditorDocument doc(false, AABB{{16.25f,0},{623.75f,20.5f}});

    CHECK(doc.spawnPosition().y == doctest::Approx(20.5f));
    CHECK(doc.spawnMinX() == doctest::Approx(16.25f));
    CHECK(doc.spawnMaxX() == doctest::Approx(607.75f));

    CHECK(doc.setSpawnX(123.25f));
    CHECK(doc.spawnPosition().x == doctest::Approx(123.25f));
    CHECK(doc.spawnPosition().y == doctest::Approx(20.5f));

    CHECK_FALSE(doc.setSpawnX(608.0f));
    CHECK_FALSE(doc.setSpawnX(800));
    CHECK(doc.spawnPosition().x == doctest::Approx(123.25f));
}

TEST_CASE("FLAG preserva coordenadas não alinhadas no ultimo nivel") {
    const AABB flag{{400.25f,300.5f},{432.75f,332.25f}};

    LevelEditorDocument middle(false, AABB{{0,0},{640,20}});
    CHECK_FALSE(middle.setFlag(flag));
    CHECK_FALSE(middle.hasFlag());

    LevelEditorDocument finalLevel(true, AABB{{0,0},{640,20}});
    CHECK(finalLevel.setFlag(flag));
    REQUIRE(finalLevel.hasFlag());
    REQUIRE(finalLevel.flag() != nullptr);
    CHECK(finalLevel.flag()->min.x == doctest::Approx(400.25f));
    CHECK(finalLevel.flag()->min.y == doctest::Approx(300.5f));
    CHECK(finalLevel.flag()->max.x == doctest::Approx(432.75f));
    CHECK(finalLevel.flag()->max.y == doctest::Approx(332.25f));

    CHECK_FALSE(finalLevel.setFlag(AABB{{630,300},{650,332}}));
    finalLevel.removeFlag();
    CHECK_FALSE(finalLevel.hasFlag());
}

TEST_CASE("presets mantêm tamanhos determinísticos independentemente de snap") {
    const Vec2 small = LevelEditorDocument::presetSize(EditorSizePreset::SMALL);
    const Vec2 medium = LevelEditorDocument::presetSize(EditorSizePreset::MEDIUM);
    const Vec2 large = LevelEditorDocument::presetSize(EditorSizePreset::LARGE);

    CHECK(small.x < medium.x);
    CHECK(medium.x < large.x);
    CHECK(small.x == doctest::Approx(64.0f));
    CHECK(small.y == doctest::Approx(16.0f));
    CHECK(medium.x == doctest::Approx(128.0f));
    CHECK(medium.y == doctest::Approx(20.0f));
    CHECK(large.x == doctest::Approx(192.0f));
    CHECK(large.y == doctest::Approx(24.0f));
}

TEST_CASE("entradas não-finitas são rejeitadas sem mutar o documento") {
    LevelEditorDocument doc(true, AABB{{0,0},{640,20}});
    REQUIRE(doc.addPlatform(AABB{{64.25f,40.5f},{192.75f,60.25f}}));
    REQUIRE(doc.setSpawnX(128.5f));
    REQUIRE(doc.setFlag(AABB{{400.25f,300.5f},{464.75f,316.25f}}));

    const std::uint64_t generation = doc.generation();
    const AABB platformBefore = doc.platforms()[0].bounds;
    const Vec2 spawnBefore = doc.spawnPosition();
    const AABB flagBefore = *doc.flag();

    CHECK_FALSE(doc.addPlatform(AABB{{std::nanf(""), 80.0f}, {128.0f, 100.0f}}));
    CHECK_FALSE(doc.movePlatform(0, {std::numeric_limits<float>::infinity(), 80.0f}));
    CHECK_FALSE(doc.setSpawnX(-std::numeric_limits<float>::infinity()));
    CHECK_FALSE(doc.setFlag(AABB{{400.0f, 300.0f}, {std::nanf(""), 316.0f}}));

    CHECK(doc.generation() == generation);
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(platformBefore.min.x));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(platformBefore.min.y));
    CHECK(doc.spawnPosition().x == doctest::Approx(spawnBefore.x));
    CHECK(doc.spawnPosition().y == doctest::Approx(spawnBefore.y));
    CHECK(doc.flag()->min.x == doctest::Approx(flagBefore.min.x));
    CHECK(doc.flag()->max.x == doctest::Approx(flagBefore.max.x));
}

TEST_CASE("restore preserva coordenadas não alinhadas sem quantizacao") {
    LevelEditorDocument doc(true, AABB{{0,0},{640,20}});

    LevelData data;
    data.name = "Precisao";
    data.screenCount = 2;
    data.platforms = {
        AABB{{0,0},{640,20.5f}},
        AABB{{33.25f,391.5f},{161.75f,412.25f}},
    };
    data.spawnPosition = Vec2{17.5f,20.5f};
    data.flag = AABB{{401.25f,661.5f},{433.75f,692.25f}};

    REQUIRE(doc.restoreFromLevelData(data));
    REQUIRE(doc.platformCount() == 1);
    CHECK(doc.screenCount() == 2);
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(33.25f));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(391.5f));
    CHECK(doc.platforms()[0].bounds.max.x == doctest::Approx(161.75f));
    CHECK(doc.platforms()[0].bounds.max.y == doctest::Approx(412.25f));
    CHECK(doc.spawnPosition().x == doctest::Approx(17.5f));
    CHECK(doc.spawnPosition().y == doctest::Approx(20.5f));
    REQUIRE(doc.flag() != nullptr);
    CHECK(doc.flag()->min.x == doctest::Approx(401.25f));
    CHECK(doc.flag()->min.y == doctest::Approx(661.5f));
    CHECK(doc.flag()->max.x == doctest::Approx(433.75f));
    CHECK(doc.flag()->max.y == doctest::Approx(692.25f));

    const LevelData roundTrip = doc.toLevelData(data.name);
    REQUIRE(roundTrip.platforms.size() == 2);
    CHECK(roundTrip.platforms[0].max.y == doctest::Approx(20.5f));
    CHECK(roundTrip.platforms[1].min.x == doctest::Approx(33.25f));
    CHECK(roundTrip.platforms[1].min.y == doctest::Approx(391.5f));
    CHECK(roundTrip.spawnPosition->x == doctest::Approx(17.5f));
    CHECK(roundTrip.flag->max.y == doctest::Approx(692.25f));
}

TEST_CASE("operações válidas incrementam a geração e operações sem mudança não o fazem") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    CHECK(doc.generation() == 0);

    REQUIRE(doc.addPlatform(AABB{{64.25f,40.5f},{192.75f,60.25f}}));
    CHECK(doc.generation() == 1);

    CHECK(doc.movePlatform(0, {64.25f,40.5f}));
    CHECK(doc.generation() == 1);

    REQUIRE(doc.movePlatform(0, {100.5f,80.25f}));
    CHECK(doc.generation() == 2);

    CHECK_FALSE(doc.setSpawnX(700.0f));
    CHECK(doc.generation() == 2);

    REQUIRE(doc.setSpawnX(120.5f));
    CHECK(doc.generation() == 3);

    REQUIRE(doc.removePlatform(0));
    CHECK(doc.generation() == 4);
    CHECK_FALSE(doc.removePlatform(0));
    CHECK(doc.generation() == 4);
}

}
