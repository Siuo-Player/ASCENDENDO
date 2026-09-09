#include "doctest/doctest.h"
#include "Logic/EditorSession.h"
#include "Core/KeyBindings.h"

namespace {

void press(logic::InputManager& input, int key) {
    input.beginFrame();
    input.onKeyEvent(key, logic::Action::PRESS);
    input.onKeyEvent(key, logic::Action::RELEASE);
}

}

TEST_SUITE("EditorSession — keyboard first-class") {

TEST_CASE("cursor teclado move-se em passos de um pixel e persiste sem movimento do rato") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    session.update(input, bindings, 640, 360);

    press(input, logic::Key::RIGHT);
    session.update(input, bindings, 640, 360);
    CHECK(session.cursor().world.x == doctest::Approx(321.0f));

    input.beginFrame();
    session.update(input, bindings, 640, 360);
    CHECK(session.cursor().world.x == doctest::Approx(321.0f));
}

TEST_CASE("ENTER coloca plataforma no cursor e DELETE funciona sem rato") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    session.update(input, bindings, 640, 360);

    press(input, logic::Key::ENTER);
    session.update(input, bindings, 640, 360);
    REQUIRE(session.document().platformCount() == 1);
    REQUIRE(session.controller().hasSelection());

    press(input, logic::Key::DELETE_KEY);
    session.update(input, bindings, 640, 360);
    CHECK(session.document().platformCount() == 0);
}

TEST_CASE("cursor teclado percorre integralmente um nivel multi-screen") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    REQUIRE(session.document().restoreFromLevelData({
        "multi",
        {{{0.0f, 0.0f}, {640.0f, 16.0f}},
         {{100.0f, 380.0f}, {220.0f, 400.0f}}},
        logic::Vec2{0.0f, 16.0f},
        std::nullopt,
        2
    }));

    // GLFW window Y=360 corresponds to the bottom of the initial logical
    // viewport; keyboard navigation then traverses the complete 720 px level.
    input.beginFrame();
    input.injectCursorPos(0.0, 360.0);
    session.update(input, bindings, 640, 360);

    for (int i = 0; i < 720; ++i) {
        press(input, logic::Key::UP);
        session.update(input, bindings, 640, 360);
    }

    CHECK(session.cursor().world.y == doctest::Approx(720.0f));
    CHECK(session.document().levelHeight() == doctest::Approx(720.0f));
    CHECK(session.renderSnapshot().viewBottomY == doctest::Approx(360.0f));
}

TEST_CASE("undo e redo restauram edicoes do documento") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    session.update(input, bindings, 640, 360);

    press(input, logic::Key::ENTER);
    session.update(input, bindings, 640, 360);
    REQUIRE(session.document().platformCount() == 1);
    REQUIRE(session.canUndo());

    press(input, logic::Key::Z);
    session.update(input, bindings, 640, 360);
    CHECK(session.document().platformCount() == 0);
    CHECK(session.canRedo());

    press(input, logic::Key::Y);
    session.update(input, bindings, 640, 360);
    CHECK(session.document().platformCount() == 1);
}

TEST_CASE("nova edicao depois de undo elimina redo futuro") {
    logic::EditorSession session(false);
    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(320.0, 180.0);
    session.update(input, bindings, 640, 360);

    press(input, logic::Key::ENTER);
    session.update(input, bindings, 640, 360);
    press(input, logic::Key::Z);
    session.update(input, bindings, 640, 360);
    REQUIRE(session.canRedo());

    press(input, logic::Key::ENTER);
    session.update(input, bindings, 640, 360);
    CHECK_FALSE(session.canRedo());
}

} // TEST_SUITE
