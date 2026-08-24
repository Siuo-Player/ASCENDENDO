#include "doctest/doctest.h"
#include "Logic/LevelEditor.h"
#include "Core/Config.h"
#include <cmath>

using namespace logic;

TEST_SUITE("Fase 9.4 — LevelEditorDocument") {

TEST_CASE("snap usa sempre o grid central") {
    CHECK(LevelEditorDocument::snap(0.0f) == doctest::Approx(0.0f));
    CHECK(LevelEditorDocument::snap(5.0f) == doctest::Approx(4.0f));
    CHECK(LevelEditorDocument::snap(6.0f) == doctest::Approx(4.0f));
    CHECK(LevelEditorDocument::snap(7.0f) == doctest::Approx(8.0f));

    AABB r{{3.0f, 5.0f}, {131.0f, 27.0f}};
    AABB s = LevelEditorDocument::snap(r);
    CHECK(s.min.x == doctest::Approx(4.0f));
    CHECK(s.min.y == doctest::Approx(4.0f));
    CHECK(s.max.x == doctest::Approx(132.0f));
    CHECK(s.max.y == doctest::Approx(28.0f));
}

TEST_CASE("plataforma e criada quantizada e dentro dos bounds") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    std::size_t index = 999;

    REQUIRE(doc.addPlatform(AABB{{3,21},{131,41}}, &index));
    REQUIRE(index == 0);
    REQUIRE(doc.platformCount() == 1);

    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(4.0f));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(20.0f));
    CHECK(doc.platforms()[0].bounds.max.x == doctest::Approx(132.0f));
    CHECK(doc.platforms()[0].bounds.max.y == doctest::Approx(40.0f));
}

TEST_CASE("plataforma fora dos limites e rejeitada") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});

    CHECK_FALSE(doc.addPlatform(AABB{{600,40},{700,60}}));
    CHECK_FALSE(doc.addPlatform(AABB{{20,-4},{100,20}}));
    CHECK_FALSE(doc.addPlatform(AABB{{0,0},{4,2}}));
    CHECK(doc.platformCount() == 0);
}

TEST_CASE("mover plataforma preserva dimensoes e rejeita movimento invalido") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    REQUIRE(doc.addPlatform(AABB{{100,80},{200,100}}));

    CHECK(doc.movePlatform(0, {300,120}));
    CHECK(doc.platforms()[0].bounds.width() == doctest::Approx(100.0f));
    CHECK(doc.platforms()[0].bounds.height() == doctest::Approx(20.0f));
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(300.0f));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(120.0f));

    CHECK_FALSE(doc.movePlatform(0, {600,340}));
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(300.0f));
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

TEST_CASE("spawn tem Y fixo e X limitado a plataforma inicial") {
    LevelEditorDocument doc(false, AABB{{16,0},{624,20}});

    CHECK(doc.spawnPosition().y == doctest::Approx(20.0f));
    CHECK(doc.spawnMinX() == doctest::Approx(16.0f));
    CHECK(doc.spawnMaxX() == doctest::Approx(608.0f));

    CHECK(doc.setSpawnX(123));
    CHECK(doc.spawnPosition().x == doctest::Approx(124.0f));
    CHECK(doc.spawnPosition().y == doctest::Approx(20.0f));

    CHECK_FALSE(doc.setSpawnX(800));
    CHECK(doc.spawnPosition().x == doctest::Approx(124.0f));
}

TEST_CASE("FLAG só pode existir no último nível da campanha") {
    AABB flag{{400,300},{432,332}};

    LevelEditorDocument middle(false, AABB{{0,0},{640,20}});
    CHECK_FALSE(middle.setFlag(flag));
    CHECK_FALSE(middle.hasFlag());

    LevelEditorDocument finalLevel(true, AABB{{0,0},{640,20}});
    CHECK(finalLevel.setFlag(flag));
    REQUIRE(finalLevel.hasFlag());
    REQUIRE(finalLevel.flag() != nullptr);
    CHECK(finalLevel.flag()->min.x == doctest::Approx(400.0f));

    finalLevel.removeFlag();
    CHECK_FALSE(finalLevel.hasFlag());
}

TEST_CASE("presets têm tamanhos determinísticos e múltiplos do grid") {
    const Vec2 small = LevelEditorDocument::presetSize(EditorSizePreset::SMALL);
    const Vec2 medium = LevelEditorDocument::presetSize(EditorSizePreset::MEDIUM);
    const Vec2 large = LevelEditorDocument::presetSize(EditorSizePreset::LARGE);

    CHECK(small.x < medium.x);
    CHECK(medium.x < large.x);
    CHECK(std::fmod(small.x, config::EDITOR_GRID_SNAP) == doctest::Approx(0.0f));
    CHECK(std::fmod(medium.x, config::EDITOR_GRID_SNAP) == doctest::Approx(0.0f));
    CHECK(std::fmod(large.x, config::EDITOR_GRID_SNAP) == doctest::Approx(0.0f));
}

}