// =============================================================================
//  Tests/Unit/test_campaign_id.cpp
//
//  @version 8.2
//  @history
//    v8.1 — criado. Valida CampaignID: determinismo, sensibilidade a
//            alteracoes de conteudo/ordem, string vazia se campanha nao
//            existir. SEM dependencias de GPU.
//    v8.2 — fix: remove_all(dir) [THROWING] trocado por remove_all(dir,ec)
//            [nao-throwing]. No Windows, o SO pode manter um ficheiro
//            momentaneamente "em uso" mesmo apos o ofstream ser fechado
//            (release assincrono do handle); a versao throwing lancava
//            excepcao e FAZIA FALHAR O TESTE mesmo com todas as assercoes
//            correctas. Temp dirs movidos de raiz do projecto para
//            build/test_tmp/ (consolidado, ja gitignored, limpo por
//            'make clean').
// =============================================================================
#include "doctest/doctest.h"
#include "Core/CampaignID.h"
#include <fstream>
#include <cstdio>
#include <filesystem>

using namespace core;

// Todas as pastas temporarias de teste vivem AQUI (nao na raiz do projecto).
// build/ ja esta no .gitignore e e removido por 'make clean' — consolida
// automaticamente qualquer residuo de testes falhados sem sujar a raiz.
static const std::string TEST_TMP_ROOT = "build/test_tmp/campaign_id";

static void writeFile(const std::string& path, const std::string& content) {
    std::ofstream f(path);
    f << content;
}

// Remocao segura: nunca lanca excepcao mesmo se o SO ainda tiver o
// ficheiro momentaneamente bloqueado (comum no Windows logo a seguir
// a fechar um ofstream). Falhas de limpeza sao ignoradas de proposito —
// nao afectam a CORRECCAO do teste, apenas podem deixar um residuo
// inofensivo em build/test_tmp/ (removido no proximo 'make clean').
static void safeRemoveAll(const std::string& path) {
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
}

TEST_SUITE("Fase 8.1 — CampaignID") {

    TEST_CASE("fnv1a64: mesma entrada produz sempre o mesmo hash") {
        const uint8_t data[] = {1, 2, 3, 4, 5};
        uint64_t h1 = fnv1a64(data, 5);
        uint64_t h2 = fnv1a64(data, 5);
        CHECK(h1 == h2);
    }

    TEST_CASE("fnv1a64: entradas diferentes produzem hashes diferentes") {
        const uint8_t dataA[] = {1, 2, 3};
        const uint8_t dataB[] = {1, 2, 4};
        CHECK(fnv1a64(dataA, 3) != fnv1a64(dataB, 3));
    }

    TEST_CASE("toHex16: produz sempre 16 caracteres") {
        CHECK(toHex16(0).length() == 16);
        CHECK(toHex16(0xFFFFFFFFFFFFFFFFull).length() == 16);
        CHECK(toHex16(0xFFFFFFFFFFFFFFFFull) == "ffffffffffffffff");
    }

    TEST_CASE("computeCampaignID: campanha inexistente devolve string vazia") {
        std::string id = computeCampaignID("build/test_tmp/nao_existe_" + std::to_string(rand()));
        CHECK(id.empty());
    }

    TEST_CASE("computeCampaignID: mesma campanha produz sempre o mesmo ID") {
        const std::string dir = TEST_TMP_ROOT + "/a";
        safeRemoveAll(dir);
        std::filesystem::create_directories(dir);
        writeFile(dir + "/campaign.txt", "nivel1.lvl\nnivel2.lvl\n");
        writeFile(dir + "/nivel1.lvl", "PLATFORM 0 0 640 20\n");
        writeFile(dir + "/nivel2.lvl", "PLATFORM 0 0 100 20\n");

        std::string id1 = computeCampaignID(dir);
        std::string id2 = computeCampaignID(dir);

        REQUIRE(!id1.empty());
        CHECK(id1 == id2);
        CHECK(id1.length() == 16);

        safeRemoveAll(dir);
    }

    TEST_CASE("computeCampaignID: alterar um nivel muda o ID") {
        const std::string dir = TEST_TMP_ROOT + "/b";
        safeRemoveAll(dir);
        std::filesystem::create_directories(dir);
        writeFile(dir + "/campaign.txt", "nivel1.lvl\n");
        writeFile(dir + "/nivel1.lvl", "PLATFORM 0 0 640 20\n");
        std::string idBefore = computeCampaignID(dir);

        writeFile(dir + "/nivel1.lvl", "PLATFORM 0 0 641 20\n");
        std::string idAfter = computeCampaignID(dir);

        REQUIRE(!idBefore.empty());
        REQUIRE(!idAfter.empty());
        CHECK(idBefore != idAfter);

        safeRemoveAll(dir);
    }

    TEST_CASE("computeCampaignID: mudar a ORDEM dos niveis muda o ID") {
        const std::string dirA = TEST_TMP_ROOT + "/order_a";
        const std::string dirB = TEST_TMP_ROOT + "/order_b";
        safeRemoveAll(dirA);
        safeRemoveAll(dirB);
        std::filesystem::create_directories(dirA);
        std::filesystem::create_directories(dirB);

        writeFile(dirA + "/campaign.txt", "n1.lvl\nn2.lvl\n");
        writeFile(dirA + "/n1.lvl", "PLATFORM 0 0 100 20\n");
        writeFile(dirA + "/n2.lvl", "PLATFORM 0 0 200 20\n");

        writeFile(dirB + "/campaign.txt", "n2.lvl\nn1.lvl\n");
        writeFile(dirB + "/n1.lvl", "PLATFORM 0 0 100 20\n");
        writeFile(dirB + "/n2.lvl", "PLATFORM 0 0 200 20\n");

        std::string idA = computeCampaignID(dirA);
        std::string idB = computeCampaignID(dirB);

        REQUIRE(!idA.empty());
        REQUIRE(!idB.empty());
        CHECK(idA != idB);

        safeRemoveAll(dirA);
        safeRemoveAll(dirB);
    }

    TEST_CASE("computeCampaignID: nomes iguais, conteudo igual -> mesmo ID (mesma campanha)") {
        const std::string dirA = TEST_TMP_ROOT + "/same_a";
        const std::string dirB = TEST_TMP_ROOT + "/same_b";
        safeRemoveAll(dirA);
        safeRemoveAll(dirB);
        std::filesystem::create_directories(dirA);
        std::filesystem::create_directories(dirB);

        writeFile(dirA + "/campaign.txt", "n1.lvl\n");
        writeFile(dirA + "/n1.lvl", "PLATFORM 0 0 100 20\n");
        writeFile(dirB + "/campaign.txt", "n1.lvl\n");
        writeFile(dirB + "/n1.lvl", "PLATFORM 0 0 100 20\n");

        CHECK(computeCampaignID(dirA) == computeCampaignID(dirB));

        safeRemoveAll(dirA);
        safeRemoveAll(dirB);
    }
}
