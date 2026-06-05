// =============================================================================
//  Tests/Unit/test_placeholder.cpp
//
//  Teste de sanidade: verifica que a framework de testes está operacional.
//  Apagar (ou substituir) quando o primeiro módulo real for implementado.
// =============================================================================

#include "doctest/doctest.h"
#include <cstdint>

TEST_SUITE("Infra") {

    TEST_CASE("framework de testes está operacional") {
        // Aritmética básica — se isto falhar, há algo muito errado.
        CHECK(1 + 1 == 2);
        CHECK(true);
        MESSAGE("doctest inicializado com sucesso.");
    }

    TEST_CASE("tipos de dados fundamentais têm o tamanho esperado") {
        // Garantir que compilamos com o modelo de dados correto.
        // Crítico para física determinística e save states binários.
        CHECK(sizeof(int)      == 4);
        CHECK(sizeof(float)    == 4);
        CHECK(sizeof(double)   == 8);
        CHECK(sizeof(uint8_t)  == 1);
        CHECK(sizeof(uint16_t) == 2);
        CHECK(sizeof(uint32_t) == 4);
        CHECK(sizeof(uint64_t) == 8);
    }

}
