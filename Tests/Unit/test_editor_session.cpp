#include "doctest/doctest.h"
#include "Logic/EditorSession.h"
#include "Core/KeyBindings.h"
#include "Core/Config.h"

TEST_SUITE("Fase 9.4 — EditorSession") {

TEST_CASE("default medium preset e cursor window->world") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    session.update(input, bindings, 640, 360);

    CHECK(session.sizePreset() == logic::EditorSizePreset::MEDIUM);
    CHECK(session.cursor().world.x == doctest::Approx(config::LOGICAL_WIDTH / 2.0f));
    CHECK(session.cursor().world.y == doctest::Approx(config::LOGICAL_HEIGHT / 2.0f));
}

TEST_CASE("STAMP cria plataforma media no clique vazio") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);

    REQUIRE(session.document().platformCount() == 1);
    CHECK(session.document().platforms()[0].bounds.width() == doctest::Approx(128.0f));
    CHECK(session.document().platforms()[0].bounds.height() == doctest::Approx(20.0f));
    CHECK(session.controller().hasSelection());
}

TEST_CASE("STAMP preview usa o preset medio e respeita o canvas") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    session.update(input, bindings, 640, 360);

    const auto preview = session.preview();
    REQUIRE(preview.visible);
    CHECK(preview.tool == logic::EditorToolMode::STAMP);
    CHECK(preview.bounds.width() == doctest::Approx(128.0f));
    CHECK(preview.bounds.height() == doctest::Approx(20.0f));
}

TEST_CASE("level editor ignora camera e mantem preview dentro da tela") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    session.update(input, bindings, 640, 360);

    const auto preview = session.preview();
    REQUIRE(preview.visible);
    CHECK(preview.bounds.min.x == doctest::Approx(256.0f));
    CHECK(preview.bounds.max.x == doctest::Approx(384.0f));
    CHECK(preview.bounds.min.y == doctest::Approx(170.0f));
    CHECK(preview.bounds.max.y == doctest::Approx(190.0f));
}

TEST_CASE("G alterna para DRAG e o arrasto cria dimensao quantizada") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    input.onKeyEvent(logic::Key::G, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);
    CHECK(session.controller().toolMode() == logic::EditorToolMode::DRAG);

    input.beginFrame();
    input.injectCursorPos(100.0, 200.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);

    CHECK(session.preview().visible);

    input.beginFrame();
    input.injectCursorPos(196.0, 136.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::RELEASE);
    session.update(input, bindings, 640, 360);

    REQUIRE(session.document().platformCount() == 1);
    CHECK(session.document().platforms()[0].bounds.min.x == doctest::Approx(100.0f));
    CHECK(session.document().platforms()[0].bounds.min.y == doctest::Approx(160.0f));
    CHECK(session.document().platforms()[0].bounds.max.x == doctest::Approx(196.0f));
    CHECK(session.document().platforms()[0].bounds.max.y == doctest::Approx(224.0f));
}

TEST_CASE("render snapshot expõe spawn fixo e objetivo derivado") {
    logic::EditorSession session(true);
    logic::InputManager input;
    core::KeyBindings bindings;

    REQUIRE(session.document().addPlatform({{256.0f, 80.0f}, {384.0f, 100.0f}}));
    const auto snapshot = session.renderSnapshot();

    CHECK(snapshot.spawnPosition.x == doctest::Approx(320.0f));
    CHECK(snapshot.spawnPosition.y == doctest::Approx(16.0f));
    CHECK(snapshot.hasFlag);
    CHECK(snapshot.flagBounds.min.x == doctest::Approx(256.0f));
    CHECK(snapshot.flagBounds.min.y == doctest::Approx(100.0f));
    CHECK(snapshot.flagBounds.width() == doctest::Approx(128.0f));
    CHECK(snapshot.flagBounds.height() == doctest::Approx(40.0f));
}

TEST_CASE("objetivo nao existe enquanto o documento nao e final") {
    logic::EditorSession session(false);
    REQUIRE(session.document().addPlatform({{256.0f, 80.0f}, {384.0f, 100.0f}}));
    CHECK_FALSE(session.document().hasFlag());
}

TEST_CASE("render snapshot expoe apenas dados graficos authored e derivados") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);

    const auto snapshot = session.renderSnapshot();
    REQUIRE(snapshot.platforms.size() == 1);
    CHECK(snapshot.hasSelection == true);
    CHECK(snapshot.selectedIndex == 0);
    CHECK(snapshot.tool == logic::EditorToolMode::STAMP);
    CHECK(snapshot.sizePreset == logic::EditorSizePreset::MEDIUM);
    CHECK(snapshot.platforms[0].width() == doctest::Approx(128.0f));
    CHECK(snapshot.spawnPosition.y == doctest::Approx(16.0f));
    CHECK_FALSE(snapshot.hasFlag);
}

TEST_CASE("cancelInteraction restaura movimento em curso") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;
    REQUIRE(session.document().addPlatform({{100.0f, 80.0f}, {228.0f, 100.0f}}));

    input.beginFrame();
    input.injectCursorPos(112.0, 270.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);

    REQUIRE(session.controller().mode() == logic::EditorMouseMode::MOVING);
    REQUIRE(session.controller().updateMove({215.0f, 157.0f}));
    CHECK(session.document().platforms()[0].bounds.min.x == doctest::Approx(203.0f));
    CHECK(session.document().platforms()[0].bounds.min.y == doctest::Approx(147.0f));

    session.cancelInteraction();
    CHECK(session.document().platforms()[0].bounds.min.x == doctest::Approx(100.0f));
    CHECK(session.document().platforms()[0].bounds.min.y == doctest::Approx(80.0f));
    CHECK_FALSE(session.controller().hasSelection());
}

TEST_CASE("DELETE apaga a selecao atual") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);
    REQUIRE(session.document().platformCount() == 1);

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    input.onKeyEvent(logic::Key::DELETE_KEY, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);

    CHECK(session.document().platformCount() == 0);
    CHECK_FALSE(session.controller().hasSelection());
}

TEST_CASE("UNDO de um movimento continuo desfaz o gesto inteiro") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;
    REQUIRE(session.document().addPlatform({{100.0f, 80.0f}, {228.0f, 100.0f}}));

    input.beginFrame();
    input.injectCursorPos(112.0, 270.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);

    input.beginFrame();
    input.injectCursorPos(150.0, 240.0);
    session.update(input, bindings, 640, 360);
    input.beginFrame();
    input.injectCursorPos(180.0, 210.0);
    session.update(input, bindings, 640, 360);
    input.beginFrame();
    input.injectCursorPos(215.0, 180.0);
    session.update(input, bindings, 640, 360);
    CHECK(session.document().platforms()[0].bounds.min.x != doctest::Approx(100.0f));

    input.beginFrame();
    input.injectCursorPos(215.0, 180.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::RELEASE);
    session.update(input, bindings, 640, 360);

    input.beginFrame();
    input.onKeyEvent(logic::Key::Z, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);

    REQUIRE(session.document().platformCount() == 1);
    CHECK(session.document().platforms()[0].bounds.min.x == doctest::Approx(100.0f));
    CHECK(session.document().platforms()[0].bounds.min.y == doctest::Approx(80.0f));
    CHECK(session.document().platforms()[0].bounds.max.x == doctest::Approx(228.0f));
    CHECK(session.document().platforms()[0].bounds.max.y == doctest::Approx(100.0f));
}

} // TEST_SUITE
