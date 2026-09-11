#include "doctest/doctest.h"
#include "Logic/EditorInteraction.h"

using namespace logic;

TEST_SUITE("Fase 9.4 — EditorInteractionController") {

TEST_CASE("cursor logical -> world usa exatamente a posição da camera") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    const EditorCursor c = controller.cursorFromLogical({100.0f, 20.0f}, {32.0f, 64.0f});
    CHECK(c.logical.x == doctest::Approx(100.0f));
    CHECK(c.logical.y == doctest::Approx(20.0f));
    CHECK(c.world.x == doctest::Approx(132.0f));
    CHECK(c.world.y == doctest::Approx(84.0f));
}

TEST_CASE("STAMP cria tamanho médio por defeito e seleciona a plataforma") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    REQUIRE(controller.stampAt({128.0f, 80.0f}));
    CHECK(doc.platformCount() == 1);
    CHECK(controller.hasSelection());
    CHECK(controller.selectedIndex() == 0);
    CHECK(doc.platforms()[0].bounds.width() == doctest::Approx(128.0f));
    CHECK(doc.platforms()[0].bounds.height() == doctest::Approx(20.0f));
}

TEST_CASE("STAMP usa o preset selecionado") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    controller.setSizePreset(EditorSizePreset::LARGE);
    REQUIRE(controller.stampAt({200.0f, 100.0f}));
    CHECK(doc.platforms()[0].bounds.width() == doctest::Approx(192.0f));
    CHECK(doc.platforms()[0].bounds.height() == doctest::Approx(24.0f));
}

TEST_CASE("DRAG normaliza os cantos sem aplicar grid") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    REQUIRE(controller.dragFromTo({200.25f, 120.5f}, {80.75f, 40.25f}));
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(80.75f));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(40.25f));
    CHECK(doc.platforms()[0].bounds.max.x == doctest::Approx(200.25f));
    CHECK(doc.platforms()[0].bounds.max.y == doctest::Approx(120.5f));
}

TEST_CASE("hit-test seleciona a entidade mais recente quando há sobreposição") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    REQUIRE(doc.addPlatform(AABB{{40,40},{160,80}}));
    REQUIRE(doc.addPlatform(AABB{{80,60},{200,100}}));
    CHECK(controller.hitPlatform({100,70}) == 1);
    CHECK(controller.hitPlatform({10,10}) == doc.platformCount());
}

TEST_CASE("mover preserva o offset do cursor e as coordenadas exatas") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    REQUIRE(doc.addPlatform(AABB{{100,80},{228,100}}));
    REQUIRE(controller.beginMove({112,90}));
    REQUIRE(controller.updateMove({215,157}));
    REQUIRE(controller.endMove());
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(203.0f));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(147.0f));
}

TEST_CASE("cancelMove restaura exatamente a posição anterior") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    REQUIRE(doc.addPlatform(AABB{{100,80},{228,100}}));
    REQUIRE(controller.beginMove({112,90}));
    REQUIRE(controller.updateMove({215,157}));
    REQUIRE(controller.cancelMove());
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(100.0f));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(80.0f));
    CHECK_FALSE(controller.hasSelection());
}

TEST_CASE("entidades derivadas não podem ser colocadas ou removidas") {
    LevelEditorDocument finalLevel(true, AABB{{32,12},{320,28}});
    EditorInteractionController controller(finalLevel);
    REQUIRE(finalLevel.addPlatform(AABB{{300,80},{500,96}}));

    const Vec2 spawnBefore = finalLevel.spawnPosition();
    REQUIRE(finalLevel.hasFlag());
    const AABB flagBefore = *finalLevel.flag();

    CHECK_FALSE(controller.placeSpawnAt({96.25f, 500.0f}));
    CHECK(finalLevel.spawnPosition().x == doctest::Approx(spawnBefore.x));
    CHECK(finalLevel.spawnPosition().y == doctest::Approx(spawnBefore.y));

    CHECK_FALSE(controller.placeFlagAt({320.25f, 80.5f}));
    REQUIRE(finalLevel.flag() != nullptr);
    CHECK(finalLevel.flag()->min.x == doctest::Approx(flagBefore.min.x));
    CHECK(finalLevel.flag()->min.y == doctest::Approx(flagBefore.min.y));

    CHECK_FALSE(controller.removeFlag());
    CHECK(finalLevel.hasFlag());
}

TEST_CASE("seleção de ferramentas legadas nunca sai de PLATFORM") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);

    controller.setEntityTool(EditorEntityTool::SPAWN);
    CHECK(controller.entityTool() == EditorEntityTool::PLATFORM);
    controller.setEntityTool(EditorEntityTool::FLAG);
    CHECK(controller.entityTool() == EditorEntityTool::PLATFORM);
}

TEST_CASE("deleteAt apaga entidade e corrige indice da seleção") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    REQUIRE(doc.addPlatform(AABB{{0,40},{64,56}}));
    REQUIRE(doc.addPlatform(AABB{{80,40},{144,56}}));
    REQUIRE(doc.addPlatform(AABB{{160,40},{224,56}}));
    REQUIRE(controller.beginMove({176,48}));
    REQUIRE(controller.deleteAt({20,48}));
    CHECK(doc.platformCount() == 2);
    CHECK(controller.selectedIndex() == 1);
}

} // namespace
