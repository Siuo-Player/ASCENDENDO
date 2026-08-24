#include "doctest.h"
#include "Logic/EditorSession.h"
#include "Core/KeyBindings.h"
#include "Core/Config.h"

namespace {
void frameInput(logic::InputManager& input,
                int keyOrMouse = -1,
                int action = logic::Action::RELEASE,
                double x = 320.0,
                double y = 180.0) {
    input.beginFrame();
    input.injectCursorPos(x, y);
    if (keyOrMouse >= 0) input.onMouseButtonEvent(keyOrMouse, action);
}
}

TEST_SUITE("Fase 9.4 — EditorSession") {

TEST_CASE("default medium preset e cursor window->world") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;
    gfx::Camera camera;

    frameInput(input, -1, logic::Action::RELEASE, 640.0 / 2.0, 360.0 / 2.0);
    session.update(input, bindings, camera, 640, 360);

    CHECK(session.sizePreset() == logic::EditorSizePreset::MEDIUM);
    CHECK(session.cursor().world.x == doctest::Approx(config::LOGICAL_WIDTH / 2.0f));
    CHECK(session.cursor().world.y == doctest::Approx(config::LOGICAL_HEIGHT / 2.0f));
}

TEST_CASE("STAMP cria plataforma media no clique vazio") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;
    gfx::Camera camera;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, camera, 640, 360);

    REQUIRE(session.document().platformCount() == 1);
    CHECK(session.document().platforms()[0].bounds.width() == doctest::Approx(128.0f));
    CHECK(session.document().platforms()[0].bounds.height() == doctest::Approx(20.0f));
    CHECK(session.controller().hasSelection());
}

TEST_CASE("G alterna para DRAG e o arrasto cria dimensao quantizada") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;
    gfx::Camera camera;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    input.onKeyEvent(logic::Key::G, logic::Action::PRESS);
    session.update(input, bindings, camera, 640, 360);
    CHECK(session.controller().toolMode() == logic::EditorToolMode::DRAG);

    input.beginFrame();
    input.injectCursorPos(100.0, 200.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, camera, 640, 360);

    input.beginFrame();
    input.injectCursorPos(196.0, 200.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::RELEASE);
    session.update(input, bindings, camera, 640, 360);

    REQUIRE(session.document().platformCount() == 1);
    CHECK(session.document().platforms()[0].bounds.min.x == doctest::Approx(100.0f));
    CHECK(session.document().platforms()[0].bounds.min.y == doctest::Approx(160.0f));
    CHECK(session.document().platforms()[0].bounds.max.x == doctest::Approx(196.0f));
    CHECK(session.document().platforms()[0].bounds.max.y == doctest::Approx(160.0f));
    CHECK(session.document().platforms()[0].bounds.height() == doctest::Approx(0.0f));
}

TEST_CASE("DELETE apaga a selecao atual") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;
    gfx::Camera camera;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    input.onMouseButtonEvent(logic::MouseButton::LEFT, logic::Action::PRESS);
    session.update(input, bindings, camera, 640, 360);
    REQUIRE(session.document().platformCount() == 1);

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    input.onKeyEvent(logic::Key::DELETE_KEY, logic::Action::PRESS);
    session.update(input, bindings, camera, 640, 360);

    CHECK(session.document().platformCount() == 0);
    CHECK_FALSE(session.controller().hasSelection());
}

} // TEST_SUITE
