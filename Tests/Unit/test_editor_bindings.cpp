#include "doctest/doctest.h"
#include "Core/KeyBindings.h"
#include "Logic/EditorSession.h"
#include "Logic/InputManager.h"

using namespace core;
using namespace logic;

TEST_SUITE("Fase 9.4 — Editor KeyBindings") {

TEST_CASE("defaults do editor são configuráveis por ação") {
    KeyBindings kb;
    CHECK(kb.keysFor(GameAction::EditorToggleMode) == std::vector<int>{Key::G});
    CHECK(kb.keysFor(GameAction::EditorSizeDown) == std::vector<int>{Key::LBRACKET});
    CHECK(kb.keysFor(GameAction::EditorSizeUp) == std::vector<int>{Key::RBRACKET});
    CHECK(kb.keysFor(GameAction::EditorSelectPlatform) == std::vector<int>{Key::P});
    CHECK(kb.keysFor(GameAction::EditorSelectSpawn) == std::vector<int>{Key::S});
    CHECK(kb.keysFor(GameAction::EditorSelectFlag) == std::vector<int>{Key::F});
    CHECK(kb.keysFor(GameAction::DeleteSelection) == std::vector<int>{Key::DELETE_KEY, Key::BACKSPACE});
}

TEST_CASE("nomes das teclas do editor fazem round-trip") {
    const int keys[] = {
        Key::G, Key::LBRACKET, Key::RBRACKET, Key::P, Key::S, Key::F,
        Key::DELETE_KEY, Key::BACKSPACE
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

TEST_CASE("editor session usa a ação reconfigurada para selecionar ferramenta") {
    KeyBindings kb;
    InputManager input;
    EditorSession session(false);

    session.controller().setEntityTool(EditorEntityTool::SPAWN);
    kb.rebind(GameAction::EditorSelectPlatform, Key::C);

    input.beginFrame();
    input.onKeyEvent(Key::C, Action::PRESS);
    session.update(input, kb, 640, 360);

    CHECK(session.controller().entityTool() == EditorEntityTool::PLATFORM);
}

TEST_CASE("ações de seleção permanecem independentes após rebind") {
    KeyBindings kb;
    InputManager input;

    kb.rebind(GameAction::EditorSelectPlatform, Key::C);
    kb.rebind(GameAction::EditorSelectSpawn, Key::E);
    kb.rebind(GameAction::EditorSelectFlag, Key::Q);

    input.beginFrame();
    input.onKeyEvent(Key::C, Action::PRESS);
    CHECK(isActionJustPressed(kb, input, GameAction::EditorSelectPlatform));
    CHECK_FALSE(isActionJustPressed(kb, input, GameAction::EditorSelectSpawn));
    CHECK_FALSE(isActionJustPressed(kb, input, GameAction::EditorSelectFlag));

    input.beginFrame();
    input.onKeyEvent(Key::E, Action::PRESS);
    CHECK(isActionJustPressed(kb, input, GameAction::EditorSelectSpawn));

    input.beginFrame();
    input.onKeyEvent(Key::Q, Action::PRESS);
    CHECK(isActionJustPressed(kb, input, GameAction::EditorSelectFlag));
}

}