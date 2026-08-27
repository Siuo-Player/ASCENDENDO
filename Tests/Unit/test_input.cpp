// =============================================================================
//  Tests/Unit/test_input.cpp
//
//  @version 3.1
//  @history
//    v3.1 — criado (7 testes, sem GLFW — injeta eventos diretamente)
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/InputManager.h"
#include "Core/KeyBindings.h"

using namespace logic;

TEST_SUITE("InputManager") {

    TEST_CASE("tecla premida: isKeyDown e isKeyJustPressed") {
        InputManager input;
        input.onKeyEvent(Key::RIGHT, Action::PRESS);

        CHECK(input.isKeyDown(Key::RIGHT)        == true);
        CHECK(input.isKeyJustPressed(Key::RIGHT)  == true);
        CHECK(input.isKeyJustReleased(Key::RIGHT) == false);
    }

    TEST_CASE("tecla solta: isKeyDown false, isKeyJustReleased true") {
        InputManager input;
        input.onKeyEvent(Key::RIGHT, Action::PRESS);
        input.onKeyEvent(Key::RIGHT, Action::RELEASE);

        CHECK(input.isKeyDown(Key::RIGHT)        == false);
        CHECK(input.isKeyJustReleased(Key::RIGHT) == true);
    }

    TEST_CASE("beginFrame limpa justPressed e justReleased, mantem isKeyDown") {
        InputManager input;
        input.onKeyEvent(Key::SPACE, Action::PRESS);
        CHECK(input.isKeyJustPressed(Key::SPACE) == true);

        input.beginFrame();

        CHECK(input.isKeyDown(Key::SPACE)        == true);
        CHECK(input.isKeyJustPressed(Key::SPACE)  == false);
        CHECK(input.isKeyJustReleased(Key::SPACE) == false);
    }

    TEST_CASE("tecla nunca premida: todos os estados false") {
        InputManager input;

        CHECK(input.isKeyDown(Key::LEFT)         == false);
        CHECK(input.isKeyJustPressed(Key::LEFT)   == false);
        CHECK(input.isKeyJustReleased(Key::LEFT)  == false);
    }

    TEST_CASE("isLeft() responde a A ou LEFT_ARROW") {
        InputManager input;
        CHECK(input.isLeft() == false);

        input.onKeyEvent(Key::A, Action::PRESS);
        CHECK(input.isLeft() == true);

        input.onKeyEvent(Key::A, Action::RELEASE);
        input.onKeyEvent(Key::LEFT, Action::PRESS);
        CHECK(input.isLeft() == true);
    }

    TEST_CASE("isJump() responde a SPACE") {
        InputManager input;
        CHECK(input.isJump() == false);

        input.onKeyEvent(Key::SPACE, Action::PRESS);
        CHECK(input.isJump() == true);

        input.onKeyEvent(Key::SPACE, Action::RELEASE);
        CHECK(input.isJump() == false);
    }

    TEST_CASE("REPEAT nao altera justPressed apos beginFrame") {
        InputManager input;
        input.onKeyEvent(Key::D, Action::PRESS);
        input.beginFrame();

        input.onKeyEvent(Key::D, Action::REPEAT);

        CHECK(input.isKeyDown(Key::D)        == true);
        CHECK(input.isKeyJustPressed(Key::D)  == false);
    }

    TEST_CASE("tickInput atribui edge events apenas ao primeiro tick do frame") {
        InputManager input;
        core::KeyBindings bindings;
        input.onKeyEvent(Key::SPACE, Action::PRESS);

        const TickInput first = input.tickInput(bindings, 0);
        const TickInput second = input.tickInput(bindings, 1);

        CHECK(first.jumpHeld == true);
        CHECK(first.jumpPressed == true);
        CHECK(second.jumpHeld == true);
        CHECK(second.jumpPressed == false);
    }

    TEST_CASE("tickInput preserva acoes continuas em todos os ticks") {
        InputManager input;
        core::KeyBindings bindings;
        input.onKeyEvent(Key::D, Action::PRESS);

        const TickInput first = input.tickInput(bindings, 0);
        const TickInput second = input.tickInput(bindings, 1);
        const TickInput third = input.tickInput(bindings, 2);

        CHECK(first.right == true);
        CHECK(second.right == true);
        CHECK(third.right == true);
    }

    TEST_CASE("tickInput de release tambem e consumido apenas no primeiro tick") {
        InputManager input;
        core::KeyBindings bindings;
        input.onKeyEvent(Key::SPACE, Action::PRESS);
        input.onKeyEvent(Key::SPACE, Action::RELEASE);

        const TickInput first = input.tickInput(bindings, 0);
        const TickInput second = input.tickInput(bindings, 1);

        CHECK(first.jumpHeld == false);
        CHECK(first.jumpReleased == true);
        CHECK(second.jumpReleased == false);
    }

}
