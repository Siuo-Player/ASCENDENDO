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

TEST_SUITE("Editor navigation") {

TEST_CASE("W e S navegam uma tela inteira sem ultrapassar o mundo") {
    logic::EditorSession session(false);
    logic::LevelData data;
    data.name = "Navigation";
    data.screenCount = 3;
    data.platforms.push_back({{100.0f, 400.0f}, {228.0f, 420.0f}});
    REQUIRE(session.document().restoreFromLevelData(data));

    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(0.0, 360.0);
    session.update(input, bindings, 640, 360);

    press(input, logic::Key::W);
    session.update(input, bindings, 640, 360);
    CHECK(session.renderSnapshot().viewBottomY == doctest::Approx(360.0f));

    press(input, logic::Key::W);
    session.update(input, bindings, 640, 360);
    CHECK(session.renderSnapshot().viewBottomY == doctest::Approx(720.0f));

    press(input, logic::Key::W);
    session.update(input, bindings, 640, 360);
    CHECK(session.renderSnapshot().viewBottomY == doctest::Approx(720.0f));

    press(input, logic::Key::S);
    session.update(input, bindings, 640, 360);
    CHECK(session.renderSnapshot().viewBottomY == doctest::Approx(360.0f));
}

TEST_CASE("setas continuam a usar navegação fina e seguem o cursor") {
    logic::EditorSession session(false);
    logic::LevelData data;
    data.name = "Cursor Navigation";
    data.screenCount = 3;
    REQUIRE(session.document().restoreFromLevelData(data));

    logic::InputManager input;
    core::KeyBindings bindings;

    input.beginFrame();
    input.injectCursorPos(0.0, 360.0);
    session.update(input, bindings, 640, 360);

    for (int i = 0; i < 200; ++i) {
        press(input, logic::Key::UP);
        session.update(input, bindings, 640, 360);
    }

    const auto snapshot = session.renderSnapshot();
    CHECK(snapshot.cursorWorld.y == doctest::Approx(200.0f));
    CHECK(snapshot.viewBottomY == doctest::Approx(20.0f));
}

} // TEST_SUITE
