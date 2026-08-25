#include "doctest/doctest.h"
#include "Core/KeyBindings.h"

#include <filesystem>
#include <fstream>
#include <string>

using namespace core;

namespace {
std::string tempControlsPath() {
    return (std::filesystem::temp_directory_path() / "ascendendo_test_controls.cfg").string();
}
}

TEST_SUITE("KeyBindings: persistencia") {

    TEST_CASE("saveToFile + loadFromFile: round-trip completo") {
        const std::string path = tempControlsPath();
        std::filesystem::remove(path);

        KeyBindings original;
        original.rebind(GameAction::Pause, Key::Q);
        original.rebind(GameAction::OpenEditor, Key::W);

        REQUIRE(original.saveToFile(path) == true);

        KeyBindings loaded;
        REQUIRE(loaded.loadFromFile(path) == true);

        for (GameAction a : ALL_ACTIONS) {
            CHECK(loaded.keysFor(a) == original.keysFor(a));
        }

        std::filesystem::remove(path);
    }

    TEST_CASE("loadFromFile: ficheiro inexistente nao altera bindings") {
        KeyBindings kb;
        kb.rebind(GameAction::Pause, Key::Q);

        const bool ok = kb.loadFromFile("/tmp/este_ficheiro_nao_existe_ascendendo.cfg");

        CHECK(ok == false);
        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::Q});
    }

    TEST_CASE("loadFromFile: accoes desconhecidas sao ignoradas") {
        const std::string path = tempControlsPath();
        std::ofstream f(path, std::ios::trunc);
        f << "# comentario\n";
        f << "AccaoDoFuturo=Z\n";
        f << "Pause=Q\n";
        f.close();

        KeyBindings kb;
        const bool ok = kb.loadFromFile(path);

        CHECK(ok == true);
        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::Q});

        std::filesystem::remove(path);
    }

    TEST_CASE("loadFromFile: accao ausente mantem o default") {
        const std::string path = tempControlsPath();
        std::ofstream f(path, std::ios::trunc);
        f << "Pause=Q\n";
        f.close();

        KeyBindings kb;
        const bool ok = kb.loadFromFile(path);

        CHECK(ok == true);
        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::Q});
        CHECK(kb.keysFor(GameAction::Quit) == std::vector<int>{Key::Q});
        CHECK(kb.keysFor(GameAction::UILeft) == std::vector<int>{Key::LEFT});

        std::filesystem::remove(path);
    }

    TEST_CASE("loadFromFile: linha com tecla invalida mantem default") {
        const std::string path = tempControlsPath();
        std::ofstream f(path, std::ios::trunc);
        f << "Pause=NaoExiste\n";
        f.close();

        KeyBindings kb;
        const bool ok = kb.loadFromFile(path);

        CHECK(ok == false);
        CHECK(kb.keysFor(GameAction::Pause) == std::vector<int>{Key::ESCAPE});

        std::filesystem::remove(path);
    }

    TEST_CASE("saveToFile: grava todas as acoes") {
        const std::string path = tempControlsPath();
        std::filesystem::remove(path);

        KeyBindings kb;
        REQUIRE(kb.saveToFile(path) == true);

        std::ifstream f(path);
        REQUIRE(f.is_open());

        int nonCommentLines = 0;
        std::string line;
        while (std::getline(f, line)) {
            if (!line.empty() && line[0] != '#') {
                ++nonCommentLines;
            }
        }
        f.close();

        CHECK(nonCommentLines == static_cast<int>(ACTION_COUNT));
        std::filesystem::remove(path);
    }
}
