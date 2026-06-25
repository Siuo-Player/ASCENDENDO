// =============================================================================
//  Tests/Unit/test_campaign.cpp
//
//  @version 7.4
//  @history
//    v7.4 — criado. Integra ai_validator.py no ciclo de testes doctest.
//            Pre-commit falha se algum nivel da campanha for impossivel.
//
//  NOTA: NAO incluir DOCTEST_CONFIG_IMPLEMENT nem DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
//  aqui. Essa macro existe apenas em Tests/test_runner.cpp.
// =============================================================================

#include "doctest/doctest.h"
#include <cstdlib>

TEST_SUITE("Campaign / AI Validation") {

    TEST_CASE("ai_validator --campaign: todos os niveis sao fisicamente passiveis") {
        // Corre o validador Python a partir da raiz do projecto.
        // Retorno 0 = campanha valida; diferente de 0 = nivel invalido.
        int ret = std::system(
            "python3 Development/AI_Validation/ai_validator.py --campaign"
        );
        CHECK(ret == 0);
    }
}
