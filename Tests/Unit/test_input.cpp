// =============================================================================
//  Tests/Unit/test_input.cpp
//
//  @version 3.1
//  @history
//    v3.1 — criado (7 testes, sem GLFW — injeta eventos diretamente)
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/InputManager.h"

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

        CHECK(input.isKeyDown(Key::SPACE)        == true);  // ainda premido
        CHECK(input.isKeyJustPressed(Key::SPACE)  == false); // ja consumido
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
        input.beginFrame(); // consome o justPressed

        input.onKeyEvent(Key::D, Action::REPEAT); // manter premida

        CHECK(input.isKeyDown(Key::D)        == true);
        CHECK(input.isKeyJustPressed(Key::D)  == false); // nao e novo press
    }

}
