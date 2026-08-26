#include "doctest/doctest.h"
#include "Core/KeyBindings.h"
#include "Logic/InputManager.h"

using namespace core;
using namespace logic;

TEST_SUITE("KeyBindings: adaptadores para InputManager") {

    TEST_CASE("isActionJustPressed: SPACE aciona UIConfirm e Jump") {
        KeyBindings kb;
        InputManager input;

        input.beginFrame();
        input.onKeyEvent(Key::SPACE, Action::PRESS);

        CHECK(isActionJustPressed(kb, input, GameAction::UIConfirm) == true);
        CHECK(isActionJustPressed(kb, input, GameAction::Jump) == true);
        CHECK(isActionJustPressed(kb, input, GameAction::Pause) == false);
    }

    TEST_CASE("isActionHeld: A ou LEFT accionam MoveLeft") {
        KeyBindings kb;
        InputManager input;

        input.beginFrame();
        input.onKeyEvent(Key::LEFT, Action::PRESS);
        CHECK(isActionHeld(kb, input, GameAction::MoveLeft) == true);

        input.beginFrame();
        input.onKeyEvent(Key::LEFT, Action::RELEASE);
        input.onKeyEvent(Key::A, Action::PRESS);
        CHECK(isActionHeld(kb, input, GameAction::MoveLeft) == true);
    }

    TEST_CASE("isActionJustReleased: reflecte apenas o frame do release") {
        KeyBindings kb;
        InputManager input;

        input.beginFrame();
        input.onKeyEvent(Key::ESCAPE, Action::PRESS);
        CHECK(isActionJustReleased(kb, input, GameAction::Pause) == false);

        input.beginFrame();
        input.onKeyEvent(Key::ESCAPE, Action::RELEASE);
        CHECK(isActionJustReleased(kb, input, GameAction::Pause) == true);

        input.beginFrame();
        CHECK(isActionJustReleased(kb, input, GameAction::Pause) == false);
    }

    TEST_CASE("rebind muda qual tecla fisica aciona a accao") {
        KeyBindings kb;
        kb.rebind(GameAction::Pause, Key::Q);

        InputManager input;
        input.beginFrame();
        input.onKeyEvent(Key::ESCAPE, Action::PRESS);
        CHECK(isActionJustPressed(kb, input, GameAction::Pause) == false);

        input.beginFrame();
        input.onKeyEvent(Key::Q, Action::PRESS);
        CHECK(isActionJustPressed(kb, input, GameAction::Pause) == true);
    }
}
