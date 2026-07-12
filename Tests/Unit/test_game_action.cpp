// =============================================================================
//  Tests/Unit/test_game_action.cpp
//
//  @version 9.1
//  @history
//    v9.1 — criado (Fase 9.1: Sistema de Configuracao de Controlos)
//
//  SEM dependencias de GPU/GLFW. Testa a tabela de acoes isolada.
// =============================================================================
#include "doctest/doctest.h"
#include "Core/GameAction.h"
#include <cstring>
#include <set>
#include <string>

using namespace core;

TEST_SUITE("Fase 9.1 — GameAction") {

    TEST_CASE("ALL_ACTIONS tem exactamente ACTION_COUNT entradas") {
        CHECK(sizeof(ALL_ACTIONS) / sizeof(ALL_ACTIONS[0]) == ACTION_COUNT);
        CHECK(ACTION_COUNT > 0);
    }

    TEST_CASE("todas as acoes em ALL_ACTIONS sao unicas") {
        std::set<int> seen;
        for (GameAction a : ALL_ACTIONS) {
            int v = static_cast<int>(a);
            CHECK(seen.count(v) == 0); // ainda nao vista
            seen.insert(v);
        }
        CHECK(seen.size() == ACTION_COUNT);
    }

    TEST_CASE("actionDisplayName: nenhuma accao devolve o fallback '?'") {
        for (GameAction a : ALL_ACTIONS) {
            const char* name = actionDisplayName(a);
            REQUIRE(name != nullptr);
            CHECK(std::strcmp(name, "?") != 0);
            CHECK(std::strlen(name) > 0);
        }
    }

    TEST_CASE("actionSerializedName: nenhuma accao devolve o fallback 'Unknown'") {
        for (GameAction a : ALL_ACTIONS) {
            const char* name = actionSerializedName(a);
            REQUIRE(name != nullptr);
            CHECK(std::strcmp(name, "Unknown") != 0);
            CHECK(std::strlen(name) > 0);
        }
    }

    TEST_CASE("actionSerializedName: nomes sao unicos e sem espacos (formato controls.cfg)") {
        std::set<std::string> seen;
        for (GameAction a : ALL_ACTIONS) {
            std::string name = actionSerializedName(a);
            CHECK(name.find(' ') == std::string::npos);
            CHECK(name.find('=') == std::string::npos);
            CHECK(seen.count(name) == 0);
            seen.insert(name);
        }
    }

    TEST_CASE("actionFromSerializedName: round-trip para todas as acoes") {
        for (GameAction a : ALL_ACTIONS) {
            const char* serialized = actionSerializedName(a);
            GameAction  parsed{};
            REQUIRE(actionFromSerializedName(serialized, parsed) == true);
            CHECK(parsed == a);
        }
    }

    TEST_CASE("actionFromSerializedName: nome desconhecido devolve false") {
        GameAction parsed{};
        CHECK(actionFromSerializedName("NaoExiste", parsed) == false);
        CHECK(actionFromSerializedName("", parsed) == false);
        CHECK(actionFromSerializedName(nullptr, parsed) == false);
    }

    TEST_CASE("actionFromSerializedName: case-sensitive (por desenho)") {
        // Formato do ficheiro e' ASCII estavel; nao normalizamos maiusculas/
        // minusculas para evitar ambiguidade silenciosa em edicoes manuais.
        GameAction parsed{};
        CHECK(actionFromSerializedName("moveleft", parsed) == false);
        CHECK(actionFromSerializedName("MOVELEFT", parsed) == false);
    }
}
