// =============================================================================
//  Tests/Unit/test_keybindings.cpp
//
//  @version 9.1
//  @history
//    v9.1 — criado (Fase 9.1: Sistema de Configuracao de Controlos)
//
//  SEM dependencias de GPU/GLFW (InputManager e' testavel sem GLFW real,
//  tal como test_input.cpp/test_player.cpp ja fazem).
// =============================================================================
#include "doctest/doctest.h"
#include "Core/KeyBindings.h"
#include "Logic/InputManager.h"
#include <cstdio>
#include <fstream>
#include <filesystem>

using namespace core;
using namespace logic;

namespace {
// Caminho unico por processo para nao colidir entre execucoes paralelas.
std::string tempControlsPath() {
    return (std::filesystem::temp_directory_path() / "ascendendo_test_controls.cfg").string();
}
} // namespace anonimo

TEST_SUITE("Fase 9.1 — KeyBindings: defaults") {

    TEST_CASE("construtor preenche todas as acoes com pelo menos 1 tecla") {
        KeyBindings kb;
        for (GameAction a : ALL_ACTIONS) {
            CHECK(kb.keysFor(a).empty() == false);
        }
    }

    TEST_CASE("defaults espelham o comportamento actual do jogo") {
        KeyBindings kb;

        // MoveLeft/MoveRight == exactamente os pares de isLeft()/isRight()
        CHECK(kb.keysFor(GameAction::MoveLeft)  == std::vector<int>{Key::A, Key::LEFT});
        CHECK(kb.keysFor(GameAction::MoveRight) == std::vector<int>{Key::D, Key::RIGHT});
        CHECK(kb.keysFor(GameAction::Jump)      == std::vector<int>{Key::SPACE});

        // Pause/UIConfirm/UILeft/UIRight == as teclas ja hardcoded em main.cpp
        CHECK(kb.keysFor(GameAction::Pause)     == std::vector<int>{Key::ESCAPE});
        CHECK(kb.keysFor(GameAction::UIConfirm) == std::vector<int>{Key::SPACE});
        CHECK(kb.keysFor(GameAction::UILeft)    == std::vector<int>{Key::LEFT});
        CHECK(kb.keysFor(GameAction::UIRight)   == std::vector<int>{Key::RIGHT});

        // Novas (Fase 9)
        CHECK(kb.keysFor(GameAction::OpenEditor) == std::vector<int>{Key::E});
        CHECK(kb.keysFor(GameAction::Quit)       == std::vector<int>{Key::Q});
        CHECK(kb.keysFor(GameAction::EditorPanUp)   == std::vector<int>{Key::W});
        CHECK(kb.keysFor(GameAction::EditorPanDown) == std::vector<int>{Key::S});
    }
}

TEST_SUITE("Fase 9.1 — KeyBindings: rebind / reset") {

    TEST_CASE("rebind substitui TODAS as teclas da accao por uma unica") {
        KeyBindings kb;
        REQUIRE(kb.keysFor(GameAction::MoveLeft).size() == 2); // A + LEFT, por omissao

        kb.rebind(GameAction::MoveLeft, Key::Q);

        CHECK(kb.keysFor(GameAction::MoveLeft) == std::vector<int>{Key::Q});
    }

    TEST_CASE("rebind nao afecta outras acoes") {
        KeyBindings kb;
        kb.rebind(GameAction::OpenEditor, Key::W);

        CHECK(kb.keysFor(GameAction::Quit) == std::vector<int>{Key::Q}); // inalterado
    }

    TEST_CASE("resetToDefault restaura so' a accao pedida") {
        KeyBindings kb;
        kb.rebind(GameAction::Pause, Key::Q);
        kb.rebind(GameAction::Quit,  Key::ESCAPE);

        kb.resetToDefault(GameAction::Pause);

        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::ESCAPE});
        CHECK(kb.keysFor(GameAction::Quit)  == std::vector<int>{Key::ESCAPE}); // Quit continua rebound
    }

    TEST_CASE("resetAllToDefaults restaura tudo") {
        KeyBindings kb;
        kb.rebind(GameAction::Pause, Key::Q);
        kb.rebind(GameAction::Quit,  Key::ESCAPE);
        kb.rebind(GameAction::UILeft, Key::A);

        kb.resetAllToDefaults();

        CHECK(kb.keysFor(GameAction::Pause)  == std::vector<int>{Key::ESCAPE});
        CHECK(kb.keysFor(GameAction::Quit)   == std::vector<int>{Key::Q});
        CHECK(kb.keysFor(GameAction::UILeft) == std::vector<int>{Key::LEFT});
    }
}

TEST_SUITE("Fase 9.1 — KeyBindings: nome <-> tecla") {

    TEST_CASE("keyToString: teclas conhecidas devolvem nome legivel") {
        CHECK(keyToString(Key::SPACE)  == "SPACE");
        CHECK(keyToString(Key::ESCAPE) == "ESCAPE");
        CHECK(keyToString(Key::LEFT)   == "LEFT");
        CHECK(keyToString(Key::E)      == "E");
        CHECK(keyToString(Key::Q)      == "Q");
    }

    TEST_CASE("keyToString: tecla desconhecida cai para o codigo numerico") {
        CHECK(keyToString(301) == "301"); // GLFW_KEY_KP_ENTER, nao esta na tabela
    }

    TEST_CASE("stringToKey: round-trip para todas as teclas nomeadas") {
        const int knownKeys[] = { Key::LEFT, Key::RIGHT, Key::UP, Key::DOWN,
                                   Key::A, Key::D, Key::W, Key::S, Key::E, Key::Q,
                                   Key::SPACE, Key::ESCAPE };
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
        CHECK(stringToKey("", parsed)        == false);
        CHECK(stringToKey("NaoExiste", parsed) == false);
        CHECK(stringToKey("12X", parsed)      == false); // mistura digitos/letras
    }
}

TEST_SUITE("Fase 9.1 — KeyBindings: persistência (ficheiro real)") {

    TEST_CASE("saveToFile + loadFromFile: round-trip completo") {
        std::string path = tempControlsPath();
        std::filesystem::remove(path);

        KeyBindings original;
        original.rebind(GameAction::Pause,      Key::Q);
        original.rebind(GameAction::OpenEditor, Key::W);

        REQUIRE(original.saveToFile(path) == true);

        KeyBindings loaded; // comeca com defaults, diferentes do `original` rebound
        REQUIRE(loaded.loadFromFile(path) == true);

        for (GameAction a : ALL_ACTIONS) {
            CHECK(loaded.keysFor(a) == original.keysFor(a));
        }

        std::filesystem::remove(path);
    }

    TEST_CASE("loadFromFile: ficheiro inexistente devolve false e nao mexe nos bindings") {
        KeyBindings kb;
        kb.rebind(GameAction::Pause, Key::Q); // estado conhecido, nao-default

        bool ok = kb.loadFromFile("/tmp/este_ficheiro_nao_existe_ascendendo.cfg");

        CHECK(ok == false);
        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::Q}); // inalterado
    }

    TEST_CASE("loadFromFile: accoes desconhecidas no ficheiro sao ignoradas (forward-compat)") {
        std::string path = tempControlsPath();
        std::ofstream f(path, std::ios::trunc);
        f << "# comentario\n";
        f << "AccaoDoFuturo=Z\n";       // accao que nao existe (versao futura)
        f << "Pause=Q\n";               // accao valida
        f.close();

        KeyBindings kb;
        bool ok = kb.loadFromFile(path);

        CHECK(ok == true);
        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::Q});

        std::filesystem::remove(path);
    }

    TEST_CASE("loadFromFile: accao ausente do ficheiro mantem o default") {
        std::string path = tempControlsPath();
        std::ofstream f(path, std::ios::trunc);
        f << "Pause=Q\n"; // so' esta accao gravada
        f.close();

        KeyBindings kb;
        bool ok = kb.loadFromFile(path);

        CHECK(ok == true);
        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::Q});
        CHECK(kb.keysFor(GameAction::Quit)  == std::vector<int>{Key::Q}); // default, nao tocado -- ATENCAO: Key::Q e' o default de Quit tambem, propositado para testar que nao ha confusao entre acoes
        CHECK(kb.keysFor(GameAction::UILeft) == std::vector<int>{Key::LEFT}); // default, nao tocado

        std::filesystem::remove(path);
    }

    TEST_CASE("loadFromFile: linha com tecla invalida e' ignorada, mantem default") {
        std::string path = tempControlsPath();
        std::ofstream f(path, std::ios::trunc);
        f << "Pause=NaoExiste\n"; // token invalido -- linha inteira sem tecla valida
        f.close();

        KeyBindings kb;
        bool ok = kb.loadFromFile(path);

        // Ficheiro abriu mas nao tinha NENHUMA linha valida -> false
        CHECK(ok == false);
        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::ESCAPE}); // default preservado

        std::filesystem::remove(path);
    }

    TEST_CASE("saveToFile: grava todas as ALL_ACTIONS, uma linha cada") {
        std::string path = tempControlsPath();
        std::filesystem::remove(path);

        KeyBindings kb;
        REQUIRE(kb.saveToFile(path) == true);

        std::ifstream f(path);
        REQUIRE(f.is_open());
        int nonCommentLines = 0;
        std::string line;
        while (std::getline(f, line)) {
            if (!line.empty() && line[0] != '#') nonCommentLines++;
        }
        f.close(); // fechar ANTES do remove() -- no Windows, apagar um ficheiro
                   // com um handle ainda aberto lanca uma excecao ("The process
                   // cannot access the file because it is being used by another
                   // process"); no Linux isto passa despercebido (POSIX permite
                   // unlink com handles abertos), por isso so' apareceu aqui.
        CHECK(nonCommentLines == (int)ACTION_COUNT);

        std::filesystem::remove(path);
    }
}

TEST_SUITE("Fase 9.1 — KeyBindings: adaptadores para InputManager (integracao real)") {

    TEST_CASE("isActionJustPressed: SPACE aciona UIConfirm e Jump (ambos default=SPACE)") {
        KeyBindings kb;
        InputManager input;

        input.beginFrame();
        input.onKeyEvent(Key::SPACE, Action::PRESS);

        CHECK(isActionJustPressed(kb, input, GameAction::UIConfirm) == true);
        CHECK(isActionJustPressed(kb, input, GameAction::Jump)      == true);
        CHECK(isActionJustPressed(kb, input, GameAction::Pause)     == false);
    }

    TEST_CASE("isActionHeld: A ou LEFT accionam MoveLeft (OR entre teclas)") {
        KeyBindings kb;
        InputManager input;

        input.beginFrame();
        input.onKeyEvent(Key::LEFT, Action::PRESS);
        CHECK(isActionHeld(kb, input, GameAction::MoveLeft) == true);

        input.beginFrame();
        input.onKeyEvent(Key::LEFT, Action::RELEASE);
        input.onKeyEvent(Key::A,    Action::PRESS);
        CHECK(isActionHeld(kb, input, GameAction::MoveLeft) == true);
    }

    TEST_CASE("isActionJustReleased: reflecte apenas o frame do release") {
        KeyBindings kb;
        InputManager input;

        input.beginFrame();
        input.onKeyEvent(Key::ESCAPE, Action::PRESS);
        CHECK(isActionJustReleased(kb, input, GameAction::Pause) == false);

        input.beginFrame(); // novo frame -- limpa justDown/justUp
        input.onKeyEvent(Key::ESCAPE, Action::RELEASE);
        CHECK(isActionJustReleased(kb, input, GameAction::Pause) == true);

        input.beginFrame(); // outro frame -- o "just" ja nao se aplica
        CHECK(isActionJustReleased(kb, input, GameAction::Pause) == false);
    }

    TEST_CASE("rebind muda qual tecla fisica aciona a accao") {
        KeyBindings kb;
        kb.rebind(GameAction::Pause, Key::Q);

        InputManager input;
        input.beginFrame();
        input.onKeyEvent(Key::ESCAPE, Action::PRESS); // tecla antiga (default)
        CHECK(isActionJustPressed(kb, input, GameAction::Pause) == false);

        input.beginFrame();
        input.onKeyEvent(Key::Q, Action::PRESS); // tecla nova (rebound)
        CHECK(isActionJustPressed(kb, input, GameAction::Pause) == true);
    }
}
