// =============================================================================
//  Tests/Unit/test_run_history.cpp
//
//  @version 8.2
//  @history
//    v8.1 — criado. Valida RunHistory: formatacao de tempo, criacao de
//            cabecalho, append correcto, criacao de pasta em falta.
//    v8.2 — fix: mesmo problema e mesma solucao de test_campaign_id.cpp —
//            remove_all(dir) throwing trocado por remove_all(dir,ec).
//            Pastas de teste em build/test_tmp/ (nao na pasta REAL de
//            Development/Runs/, para nunca misturar dados de teste com
//            runs a serio).
// =============================================================================
#include "doctest/doctest.h"
#include "Logic/RunHistory.h"
#include <fstream>
#include <sstream>
#include <filesystem>

using namespace logic;

static const std::string TEST_TMP_ROOT = "build/test_tmp/run_history";

static void safeRemoveAll(const std::string& path) {
    std::error_code ec;
    std::filesystem::remove_all(path, ec);
}

TEST_SUITE("Fase 8.1 — RunHistory") {

    TEST_CASE("formatElapsed: formata MM:SS.mmm correctamente") {
        CHECK(formatElapsed(0.0f)      == "00:00.000");
        CHECK(formatElapsed(5.5f)      == "00:05.500");
        CHECK(formatElapsed(65.25f)    == "01:05.250");
        CHECK(formatElapsed(125.487f)  == "02:05.487");
        CHECK(formatElapsed(-3.0f)     == "00:00.000");
    }

    TEST_CASE("recordRun: cria pasta e ficheiro com cabecalho na primeira escrita") {
        const std::string dir  = TEST_TMP_ROOT + "/a";
        const std::string path = dir + "/runs.csv";
        safeRemoveAll(dir);

        bool ok = recordRun(path, "TesteCampanha", "abc123", 42.0f);
        REQUIRE(ok == true);
        REQUIRE(std::filesystem::exists(path));

        std::ifstream f(path);
        std::string header;
        std::getline(f, header);
        CHECK(header == "timestamp,campaign_name,campaign_id,elapsed_seconds,elapsed_formatted");

        std::string dataLine;
        std::getline(f, dataLine);
        CHECK(dataLine.find("TesteCampanha") != std::string::npos);
        CHECK(dataLine.find("abc123") != std::string::npos);

        f.close();          // fechar explicitamente ANTES de limpar (Windows)
        safeRemoveAll(dir);
    }

    TEST_CASE("recordRun: segunda chamada acrescenta sem duplicar cabecalho") {
        const std::string dir  = TEST_TMP_ROOT + "/b";
        const std::string path = dir + "/runs.csv";
        safeRemoveAll(dir);

        recordRun(path, "Camp1", "id1", 10.0f);
        recordRun(path, "Camp2", "id2", 20.0f);

        std::ifstream f(path);
        int lineCount = 0;
        int headerCount = 0;
        std::string line;
        while (std::getline(f, line)) {
            lineCount++;
            if (line.find("timestamp") != std::string::npos) headerCount++;
        }
        f.close();

        CHECK(lineCount == 3);
        CHECK(headerCount == 1);

        safeRemoveAll(dir);
    }
}
