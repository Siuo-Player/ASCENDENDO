#include "doctest/doctest.h"
#include "Core/KeyBindings.h"

using namespace core;

TEST_SUITE("KeyBindings: defaults") {

    TEST_CASE("construtor preenche todas as acoes com pelo menos 1 tecla") {
        KeyBindings kb;
        for (GameAction a : ALL_ACTIONS) {
            CHECK(kb.keysFor(a).empty() == false);
        }
    }

    TEST_CASE("defaults preservam o comportamento base do jogo") {
        KeyBindings kb;

        CHECK(kb.keysFor(GameAction::MoveLeft) == std::vector<int>{Key::A, Key::LEFT});
        CHECK(kb.keysFor(GameAction::MoveRight) == std::vector<int>{Key::D, Key::RIGHT});
        CHECK(kb.keysFor(GameAction::Jump) == std::vector<int>{Key::SPACE});
        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::ESCAPE});
        CHECK(kb.keysFor(GameAction::UIConfirm) == std::vector<int>{Key::SPACE});
        CHECK(kb.keysFor(GameAction::UILeft) == std::vector<int>{Key::LEFT});
        CHECK(kb.keysFor(GameAction::UIRight) == std::vector<int>{Key::RIGHT});
        CHECK(kb.keysFor(GameAction::OpenEditor) == std::vector<int>{Key::E});
        CHECK(kb.keysFor(GameAction::Quit) == std::vector<int>{Key::Q});
        CHECK(kb.keysFor(GameAction::EditorPanUp) == std::vector<int>{Key::W});
        CHECK(kb.keysFor(GameAction::EditorPanDown) == std::vector<int>{Key::S});
    }
}

TEST_SUITE("KeyBindings: rebind / reset") {

    TEST_CASE("rebind substitui todas as teclas da accao por uma unica") {
        KeyBindings kb;
        REQUIRE(kb.keysFor(GameAction::MoveLeft).size() == 2);

        kb.rebind(GameAction::MoveLeft, Key::Q);

        CHECK(kb.keysFor(GameAction::MoveLeft) == std::vector<int>{Key::Q});
    }

    TEST_CASE("rebind nao afecta outras acoes") {
        KeyBindings kb;
        kb.rebind(GameAction::OpenEditor, Key::W);

        CHECK(kb.keysFor(GameAction::Quit) == std::vector<int>{Key::Q});
    }

    TEST_CASE("resetToDefault restaura so a accao pedida") {
        KeyBindings kb;
        kb.rebind(GameAction::Pause, Key::Q);
        kb.rebind(GameAction::Quit, Key::ESCAPE);

        kb.resetToDefault(GameAction::Pause);

        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::ESCAPE});
        CHECK(kb.keysFor(GameAction::Quit) == std::vector<int>{Key::ESCAPE});
    }

    TEST_CASE("resetAllToDefaults restaura tudo") {
        KeyBindings kb;
        kb.rebind(GameAction::Pause, Key::Q);
        kb.rebind(GameAction::Quit, Key::ESCAPE);
        kb.rebind(GameAction::UILeft, Key::A);

        kb.resetAllToDefaults();

        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::ESCAPE});
        CHECK(kb.keysFor(GameAction::Quit) == std::vector<int>{Key::Q});
        CHECK(kb.keysFor(GameAction::UILeft) == std::vector<int>{Key::LEFT});
    }
}

TEST_SUITE("KeyBindings: nome <-> tecla") {

    TEST_CASE("keyToString: teclas conhecidas devolvem nome legivel") {
        CHECK(keyToString(Key::SPACE) == "SPACE");
        CHECK(keyToString(Key::ESCAPE) == "ESCAPE");
        CHECK(keyToString(Key::LEFT) == "LEFT");
        CHECK(keyToString(Key::E) == "E");
        CHECK(keyToString(Key::Q) == "Q");
    }

    TEST_CASE("keyToString: tecla desconhecida cai para o codigo numerico") {
        CHECK(keyToString(301) == "301");
    }

    TEST_CASE("stringToKey: round-trip para todas as teclas nomeadas") {
        const int knownKeys[] = {
            Key::LEFT, Key::RIGHT, Key::UP, Key::DOWN,
            Key::A, Key::D, Key::W, Key::S, Key::E, Key::Q,
            Key::SPACE, Key::ESCAPE
        };

        for (int key : knownKeys) {
            int parsed = -1;
            REQUIRE(stringToKey(keyToString(key), parsed) == true);
            CHECK(parsed == key);
        }
    }

    TEST_CASE("stringToKey: fallback numerico para codigos sem nome") {
        int parsed = 0;
        REQUIRE(stringToKey("301", parsed) == true);
        CHECK(parsed == 301);
    }

    TEST_CASE("stringToKey: string invalida devolve false") {
        int parsed = 0;
        CHECK(stringToKey("", parsed) == false);
        CHECK(stringToKey("NaoExiste", parsed) == false);
        CHECK(stringToKey("12X", parsed) == false);
    }
}
