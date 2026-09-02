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
    input.injectCursorPos(100.0, 200.0); // world (100,200)
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);

    CHECK(session.preview().visible);

    input.beginFrame();
    input.injectCursorPos(196.0, 136.0); // world (196,136)
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::RELEASE);
    session.update(input, bindings, 640, 360);

    REQUIRE(session.document().platformCount() == 1);
    CHECK(session.document().platforms()[0].bounds.min.x == doctest::Approx(100.0f));
    CHECK(session.document().platforms()[0].bounds.min.y == doctest::Approx(136.0f));
    CHECK(session.document().platforms()[0].bounds.max.x == doctest::Approx(196.0f));
    CHECK(session.document().platforms()[0].bounds.max.y == doctest::Approx(200.0f));
}

TEST_CASE("render snapshot expoe apenas dados graficos") {
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

TEST_CASE("RIGHT cancel numa movimentacao restaura a posicao original") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);
    REQUIRE(session.document().platformCount() == 1);
    const AABB original = session.document().platforms()[0].bounds;

    input.beginFrame();
    input.injectCursorPos(360.0, 180.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);
    REQUIRE(session.controller().mode() == logic::EditorMouseMode::MOVING);

    input.beginFrame();
    input.injectCursorPos(480.0, 260.0);
    session.update(input, bindings, 640, 360);
    CHECK(session.document().platforms()[0].bounds.min.x != doctest::Approx(original.min.x));

    input.beginFrame();
    input.injectCursorPos(480.0, 260.0);
    input.onMouseButtonEvent(logic::MouseButton::RIGHT, logic::Action::PRESS);
    session.update(input, bindings, 640, 360);

    CHECK(session.controller().mode() == logic::EditorMouseMode::NONE);
    CHECK_FALSE(session.controller().hasSelection());
    CHECK(session.document().platforms()[0].bounds.min.x == doctest::Approx(original.min.x));
    CHECK(session.document().platforms()[0].bounds.min.y == doctest::Approx(original.min.y));
    CHECK(session.document().platforms()[0].bounds.max.x == doctest::Approx(original.max.x));
    CHECK(session.document().platforms()[0].bounds.max.y == doctest::Approx(original.max.y));
}

} // TEST_SUITE
