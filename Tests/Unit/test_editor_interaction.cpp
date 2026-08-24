#include "doctest/doctest.h"
#include "Logic/EditorInteraction.h"

using namespace logic;

TEST_SUITE("Fase 9.4 — EditorInteractionController") {

TEST_CASE("cursor logical -> world usa exatamente o offset da camera") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    gfx::Camera camera;
    camera.position = {32.0f, 64.0f};

    const EditorCursor c = controller.cursorFromLogical({100.0f, 20.0f}, camera);

    CHECK(c.logical.x == doctest::Approx(100.0f));
    CHECK(c.logical.y == doctest::Approx(20.0f));
    CHECK(c.world.x == doctest::Approx(132.0f));
    CHECK(c.world.y == doctest::Approx(84.0f));
}

TEST_CASE("STAMP cria tamanho médio por defeito e seleciona a plataforma") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);

    REQUIRE(controller.stampAt({128.0f, 80.0f}));
    REQUIRE(doc.platformCount() == 1);
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

TEST_CASE("DRAG normaliza os cantos e aplica grid no documento") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);

    REQUIRE(controller.dragFromTo({200.0f, 120.0f}, {80.0f, 40.0f}));
    REQUIRE(doc.platformCount() == 1);
    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(80.0f));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(40.0f));
    CHECK(doc.platforms()[0].bounds.max.x == doctest::Approx(200.0f));
    CHECK(doc.platforms()[0].bounds.max.y == doctest::Approx(120.0f));
}

TEST_CASE("hit-test seleciona a entidade mais recente quando há sobreposição") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);

    REQUIRE(doc.addPlatform(AABB{{40,40},{160,80}}));
    REQUIRE(doc.addPlatform(AABB{{80,60},{200,100}}));

    CHECK(controller.hitPlatform({100,70}) == 1);
    CHECK(controller.hitPlatform({10,10}) == 2); // platformCount() = npos lógico
}

TEST_CASE("mover preserva o offset do cursor e permanece no grid") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    REQUIRE(doc.addPlatform(AABB{{100,80},{228,100}}));

    REQUIRE(controller.beginMove({112,90}));
    REQUIRE(controller.updateMove({215,157}));
    REQUIRE(controller.endMove());

    CHECK(doc.platforms()[0].bounds.min.x == doctest::Approx(204.0f));
    CHECK(doc.platforms()[0].bounds.min.y == doctest::Approx(148.0f));
    CHECK(controller.selectedIndex() == 0);
}

TEST_CASE("deleteAt apaga entidade e corrige indice da seleção") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    REQUIRE(doc.addPlatform(AABB{{0,40},{64,56}}));
    REQUIRE(doc.addPlatform(AABB{{80,40},{144,56}}));
    REQUIRE(doc.addPlatform(AABB{{160,40},{224,56}}));

    REQUIRE(controller.beginMove({176,48}));
    CHECK(controller.selectedIndex() == 2);
    REQUIRE(controller.deleteAt({20,48}));
    CHECK(doc.platformCount() == 2);
    CHECK(controller.selectedIndex() == 1);
    CHECK(controller.hasSelection());
}

TEST_CASE("deleteAt apaga a seleção clicada e a limpa") {
    LevelEditorDocument doc(false, AABB{{0,0},{640,20}});
    EditorInteractionController controller(doc);
    REQUIRE(doc.addPlatform(AABB{{0,40},{64,56}}));

    REQUIRE(controller.beginMove({20,48}));
    REQUIRE(controller.deleteAt({20,48}));
    CHECK_FALSE(controller.hasSelection());
    CHECK(doc.platformCount() == 0);
}
}
