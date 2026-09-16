// =============================================================================
// Tests/Unit/test_editor_undo_gestures.cpp
// =============================================================================
// Regression coverage for continuous move/resize undo baselines.

#include "doctest/doctest.h"
#include "Logic/EditorSession.h"
#include "Core/KeyBindings.h"

using namespace logic;

namespace {

void press(InputManager& input, core::KeyBindings& bindings, EditorSession& session,
           double x, double y) {
    input.beginFrame();
    input.injectCursorPos(x, y);
    input.onMouseButtonEvent(MouseButton::LEFT, Action::PRESS);
    session.update(input, bindings, 640, 360);
}

void move(InputManager& input, core::KeyBindings& bindings, EditorSession& session,
          double x, double y) {
    input.beginFrame();
    input.injectCursorPos(x, y);
    session.update(input, bindings, 640, 360);
}

void release(InputManager& input, core::KeyBindings& bindings, EditorSession& session) {
    input.beginFrame();
    input.onMouseButtonEvent(MouseButton::LEFT, Action::RELEASE);
    session.update(input, bindings, 640, 360);
}

void undo(InputManager& input, core::KeyBindings& bindings, EditorSession& session) {
    input.beginFrame();
    input.onKeyEvent(Key::Z, Action::PRESS);
    session.update(input, bindings, 640, 360);
    input.beginFrame();
    input.onKeyEvent(Key::Z, Action::RELEASE);
    session.update(input, bindings, 640, 360);
}

} // namespace

TEST_SUITE("EditorSession — undo de gestos continuos apos uma acao anterior") {

TEST_CASE("[regressao] undo apos mover uma plataforma que sucede um stamp "
          "desfaz so o movimento") {
    EditorSession session(false);
    InputManager input;
    core::KeyBindings bindings;

    // Acao anterior real, nao injetada diretamente no documento.
    press(input, bindings, session, 300.0, 260.0);
    release(input, bindings, session);
    REQUIRE(session.document().platformCount() == 1);
    const AABB original = session.document().platforms()[0].bounds;

    press(input, bindings, session, 300.0, 260.0);
    for (int i = 1; i <= 5; ++i)
        move(input, bindings, session, 300.0 + i * 20.0, 260.0);
    release(input, bindings, session);

    const AABB afterDrag = session.document().platforms()[0].bounds;
    REQUIRE(afterDrag.min.x != doctest::Approx(original.min.x));

    undo(input, bindings, session);

    REQUIRE(session.document().platformCount() == 1);
    const AABB afterUndo = session.document().platforms()[0].bounds;
    CHECK(afterUndo.min.x == doctest::Approx(original.min.x));
    CHECK(afterUndo.min.y == doctest::Approx(original.min.y));
    CHECK(afterUndo.max.x == doctest::Approx(original.max.x));
    CHECK(afterUndo.max.y == doctest::Approx(original.max.y));
}

TEST_CASE("[regressao] undo apos redimensionar uma plataforma que sucede "
          "outra plataforma desfaz so o resize") {
    EditorSession session(false);
    InputManager input;
    core::KeyBindings bindings;

    press(input, bindings, session, 500.0, 260.0);
    release(input, bindings, session);
    press(input, bindings, session, 200.0, 260.0);
    release(input, bindings, session);
    REQUIRE(session.document().platformCount() == 2);
    const AABB originalA = session.document().platforms()[1].bounds;

    press(input, bindings, session, 200.0, 260.0);
    release(input, bindings, session);
    REQUIRE(session.controller().hasSelection());

    const float handleWindowY = 360.0f - originalA.max.y;
    press(input, bindings, session, originalA.max.x, handleWindowY);
    REQUIRE(session.controller().resizeHandle() != EditorResizeHandle::NONE);
    for (int i = 1; i <= 5; ++i)
        move(input, bindings, session, originalA.max.x + i * 15.0, handleWindowY);
    release(input, bindings, session);

    REQUIRE(session.document().platformCount() == 2);
    const AABB resized = session.document().platforms()[1].bounds;
    REQUIRE(resized.max.x != doctest::Approx(originalA.max.x));

    undo(input, bindings, session);

    REQUIRE(session.document().platformCount() == 2);
    const AABB afterUndo = session.document().platforms()[1].bounds;
    CHECK(afterUndo.min.x == doctest::Approx(originalA.min.x));
    CHECK(afterUndo.min.y == doctest::Approx(originalA.min.y));
    CHECK(afterUndo.max.x == doctest::Approx(originalA.max.x));
    CHECK(afterUndo.max.y == doctest::Approx(originalA.max.y));
}

TEST_CASE("dois arrastos seguidos produzem dois undos separados, "
          "nao um so e nao mais que dois") {
    EditorSession session(false);
    InputManager input;
    core::KeyBindings bindings;

    press(input, bindings, session, 300.0, 260.0);
    release(input, bindings, session);
    const AABB original = session.document().platforms()[0].bounds;

    press(input, bindings, session, 300.0, 260.0);
    for (int i = 1; i <= 3; ++i)
        move(input, bindings, session, 300.0 + i * 20.0, 260.0);
    release(input, bindings, session);
    const AABB afterFirstDrag = session.document().platforms()[0].bounds;

    press(input, bindings, session, afterFirstDrag.min.x + 5.0, 260.0);
    for (int i = 1; i <= 3; ++i)
        move(input, bindings, session,
             afterFirstDrag.min.x + 5.0 + i * 20.0, 260.0);
    release(input, bindings, session);
    const AABB afterSecondDrag = session.document().platforms()[0].bounds;
    REQUIRE(afterSecondDrag.min.x != doctest::Approx(afterFirstDrag.min.x));

    undo(input, bindings, session);
    REQUIRE(session.document().platformCount() == 1);
    CHECK(session.document().platforms()[0].bounds.min.x ==
          doctest::Approx(afterFirstDrag.min.x));

    undo(input, bindings, session);
    REQUIRE(session.document().platformCount() == 1);
    CHECK(session.document().platforms()[0].bounds.min.x ==
          doctest::Approx(original.min.x));

    CHECK(session.canUndo());
}

} // TEST_SUITE
