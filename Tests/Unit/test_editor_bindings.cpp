#include "doctest/doctest.h"
#include "Core/KeyBindings.h"
#include "Logic/InputManager.h"

using namespace core;
using namespace logic;

TEST_SUITE("Fase 9.4 — Editor KeyBindings") {

TEST_CASE("defaults do editor são G, [, ], DELETE/BACKSPACE") {
    KeyBindings kb;
    CHECK(kb.keysFor(GameAction::EditorToggleMode) == std::vector<int>{Key::G});
    CHECK(kb.keysFor(GameAction::EditorSizeDown) == std::vector<int>{Key::LBRACKET});
    CHECK(kb.keysFor(GameAction::EditorSizeUp) == std::vector<int>{Key::RBRACKET});
    CHECK(kb.keysFor(GameAction::DeleteSelection) == std::vector<int>{Key::DELETE_KEY, Key::BACKSPACE});
}

TEST_CASE("nomes das teclas do editor fazem round-trip") {
    const int keys[] = {
        Key::G, Key::LBRACKET, Key::RBRACKET, Key::DELETE_KEY, Key::BACKSPACE
    };
    for (int key : keys) {
        int parsed = -1;
        REQUIRE(stringToKey(keyToString(key), parsed));
        CHECK(parsed == key);
    }
}

TEST_CASE("duas teclas podem representar a mesma ação de apagar") {
    KeyBindings kb;
    InputManager input;

    input.beginFrame();
    input.onKeyEvent(Key::BACKSPACE, Action::PRESS);
    CHECK(isActionJustPressed(kb, input, GameAction::DeleteSelection));

    input.beginFrame();
    input.onKeyEvent(Key::BACKSPACE, Action::RELEASE);
    input.beginFrame();
    input.onKeyEvent(Key::DELETE_KEY, Action::PRESS);
    CHECK(isActionJustPressed(kb, input, GameAction::DeleteSelection));
}

}