// =============================================================================
//  Tests/Unit/test_input_mouse.cpp
//
//  @version 9.2
//  @history
//    v9.2 — criado (Fase 9.2: suporte a rato). Ficheiro SEPARADO de
//           test_input.cpp (que ja existe e cobre teclado) -- deliberado,
//           para nao arriscar sobrepor um ficheiro real que nao foi
//           partilhado nesta sessao.
//
//  SEM dependencias de GPU/GLFW (mesmo padrao de injeccao de eventos ja
//  usado para teclado).
// =============================================================================
#include "doctest/doctest.h"
#include "Logic/InputManager.h"

using namespace logic;

TEST_SUITE("Fase 9.2 — InputManager: botoes de rato") {

    TEST_CASE("isMouseButtonDown reflecte PRESS/RELEASE") {
        InputManager input;
        input.beginFrame();
        CHECK(input.isMouseButtonDown(MouseButton::LEFT) == false);

        input.onMouseButtonEvent(MouseButton::LEFT, Action::PRESS);
        CHECK(input.isMouseButtonDown(MouseButton::LEFT) == true);

        input.beginFrame();
        input.onMouseButtonEvent(MouseButton::LEFT, Action::RELEASE);
        CHECK(input.isMouseButtonDown(MouseButton::LEFT) == false);
    }

    TEST_CASE("isMouseButtonJustPressed so' e' verdade no frame do PRESS") {
        InputManager input;
        input.beginFrame();
        input.onMouseButtonEvent(MouseButton::LEFT, Action::PRESS);
        CHECK(input.isMouseButtonJustPressed(MouseButton::LEFT) == true);

        input.beginFrame(); // novo frame -- "just" limpo, botao continua down logicamente,
                            // mas sem novo evento onMouseButtonEvent nao ha PRESS neste frame
        CHECK(input.isMouseButtonJustPressed(MouseButton::LEFT) == false);
        CHECK(input.isMouseButtonDown(MouseButton::LEFT) == true); // continua premido
    }

    TEST_CASE("isMouseButtonJustReleased so' e' verdade no frame do RELEASE") {
        InputManager input;
        input.beginFrame();
        input.onMouseButtonEvent(MouseButton::LEFT, Action::PRESS);
        CHECK(input.isMouseButtonJustReleased(MouseButton::LEFT) == false);

        input.beginFrame();
        input.onMouseButtonEvent(MouseButton::LEFT, Action::RELEASE);
        CHECK(input.isMouseButtonJustReleased(MouseButton::LEFT) == true);

        input.beginFrame();
        CHECK(input.isMouseButtonJustReleased(MouseButton::LEFT) == false);
    }

    TEST_CASE("botoes LEFT/RIGHT/MIDDLE sao independentes entre si") {
        InputManager input;
        input.beginFrame();
        input.onMouseButtonEvent(MouseButton::LEFT, Action::PRESS);

        CHECK(input.isMouseButtonDown(MouseButton::LEFT)   == true);
        CHECK(input.isMouseButtonDown(MouseButton::RIGHT)  == false);
        CHECK(input.isMouseButtonDown(MouseButton::MIDDLE) == false);
    }

    TEST_CASE("codigos de botao de rato nao colidem com codigos de tecla") {
        // MouseButton::LEFT=0 e Key::A=65 (etc.) vivem em mapas SEPARADOS --
        // premir um botao de rato nao deve mexer no estado de nenhuma tecla.
        InputManager input;
        input.beginFrame();
        input.onMouseButtonEvent(MouseButton::LEFT, Action::PRESS);

        CHECK(input.isKeyDown(Key::A) == false);
        CHECK(input.isLeft()          == false);
    }
}

TEST_SUITE("Fase 9.2 — InputManager: posicao do cursor") {

    TEST_CASE("valor inicial e' (0,0)") {
        InputManager input;
        CHECK(input.cursorX() == doctest::Approx(0.0));
        CHECK(input.cursorY() == doctest::Approx(0.0));
    }

    TEST_CASE("onCursorPosEvent actualiza a posicao") {
        InputManager input;
        input.onCursorPosEvent(123.5, 456.75);
        CHECK(input.cursorX() == doctest::Approx(123.5));
        CHECK(input.cursorY() == doctest::Approx(456.75));
    }

    TEST_CASE("posicao do cursor NAO e' limpa por beginFrame() -- e' continua, nao um evento") {
        InputManager input;
        input.onCursorPosEvent(200.0, 300.0);
        input.beginFrame();
        input.beginFrame();
        CHECK(input.cursorX() == doctest::Approx(200.0));
        CHECK(input.cursorY() == doctest::Approx(300.0));
    }

    TEST_CASE("injectCursorPos funciona tal como onCursorPosEvent (mocks/testes)") {
        InputManager input;
        input.injectCursorPos(50.0, 75.0);
        CHECK(input.cursorX() == doctest::Approx(50.0));
        CHECK(input.cursorY() == doctest::Approx(75.0));
    }
}
