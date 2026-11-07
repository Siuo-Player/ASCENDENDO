// =============================================================================
//  Tests/Security/test_security_penetration.cpp
//
//  Bateria de Testes:
//  • Penetration / Pen Testing (Fuzzing por rotação de bits em Replays e Perfis)
//  • Injeção de Fórmulas CSV (CSV Injection / DDE attack mitigation)
//  • Path Traversal e Sandbox Escape (Tentativas de escape de diretoria e dispositivos Windows)
//  • Side-Channel Resistance (Comparação em tempo constante)
//  • Resiliência contra ficheiros truncados e corrompidos
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "Core/Security.h"
#include "Logic/ProfileStorage.h"
#include "Logic/RunReplay.h"
#include "Logic/RunHistory.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

TEST_SUITE("Security & Penetration Testing") {

TEST_CASE("Pen Testing: Fuzzing de rotação de bits em Replay serializado") {
    logic::RunReplay original;
    original.header().campaignID = "campaign_security_test";
    original.header().campaignName = "Security Verification";
    original.header().playerID = "player_whitehat";
    original.header().playerName = "Alice";

    for (int i = 0; i < 50; ++i) {
        logic::TickInput t;
        t.right = (i % 2 == 0);
        t.left = (i % 2 != 0);
        t.jumpHeld = (i % 5 == 0);
        original.addTick(t);
    }
    original.header().claimedElapsedSeconds = 50.0f * (1.0f / 60.0f);
    original.header().totalTicks = 50;

    const std::filesystem::path tempReplay = "build/fuzz_test.replay";
    REQUIRE(original.saveToFile(tempReplay) == true);

    std::ifstream in(tempReplay, std::ios::binary);
    std::string validPayload((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    REQUIRE(!validPayload.empty());

    // Fuzzing: Testar mutação de 1 bit em diferentes posições do payload autenticado
    // Em TODAS as mutações, a verificação criptográfica DEVE rejeitar o ficheiro.
    const std::vector<std::size_t> testOffsets = {
        0, 5, 10, 20, 35, validPayload.size() / 2, validPayload.size() - 5, validPayload.size() - 1
    };

    for (std::size_t offset : testOffsets) {
        if (offset >= validPayload.size()) continue;

        std::string tampered = validPayload;
        tampered[offset] ^= 0x01; // Inverter 1 bit

        std::ofstream out(tempReplay, std::ios::binary | std::ios::trunc);
        out.write(tampered.data(), tampered.size());
        out.close();

        const auto victim = logic::RunReplay::loadFromFile(tempReplay);
        CHECK_MESSAGE(victim.has_value() == false, "Ficheiro adulterado no byte ", offset, " DEVE ser rejeitado pelo HMAC!");
    }

    std::error_code ec;
    std::filesystem::remove(tempReplay, ec);
}

TEST_CASE("Pen Testing: Injeção de payloads truncados ou lixo binário") {
    // Payload vazio
    CHECK(logic::RunReplay::deserialize("").has_value() == false);

    // Payload com lixo aleatório
    const std::string garbage = "\x00\xFF\xAA\x55\xDE\xAD\xBE\xEF\x12\x34\x56\x78";
    CHECK(logic::RunReplay::deserialize(garbage).has_value() == false);

    // Payload com cabeçalho truncado
    CHECK(logic::RunReplay::deserialize("ASCENDENDO_REPLAY_V1\n").has_value() == false);

    // Payload com claimedElapsedSeconds negativo (ataque de subversão de tempo)
    const std::string negativeTimePayload =
        "ASCENDENDO_REPLAY_V1\n"
        "version:1\n"
        "campaignID:camp1\n"
        "claimedSeconds:-999.0\n"
        "totalTicks:10\n"
        "INPUTS:\n";
    CHECK(logic::RunReplay::deserialize(negativeTimePayload).has_value() == false);
}

TEST_CASE("Path Traversal Attacks: Rejeição estrita de escape de sandbox") {
    const std::filesystem::path sandboxRoot = std::filesystem::current_path() / "build" / "sandbox_test";
    std::filesystem::create_directories(sandboxRoot);

    // Tentativas comuns de path traversal
    std::vector<std::string> maliciousPaths = {
        "../../etc/passwd",
        "nested/../../outside.txt",
        "/etc/shadow",
        "/usr/bin/bash",
        "relative/../../../root.txt",
    };
#ifdef _WIN32
    maliciousPaths.push_back("..\\..\\Windows\\System32\\calc.exe");
    maliciousPaths.push_back("C:\\Windows\\System32\\cmd.exe");
#endif

    for (const auto& evilPath : maliciousPaths) {
        const std::filesystem::path target = sandboxRoot / evilPath;
        const bool contained = core::Security::isPathContained(target, sandboxRoot);
        CHECK_MESSAGE(contained == false, "Caminho malicioso deve ser detetado e bloqueado: ", evilPath);
    }

    // Caminho legítimo dentro da sandbox
    const std::filesystem::path safePath = sandboxRoot / "levels" / "level_01.lvl";
    CHECK(core::Security::isPathContained(safePath, sandboxRoot) == true);

    std::error_code ec;
    std::filesystem::remove_all(sandboxRoot, ec);
}

TEST_CASE("Windows Device Name Attacks: Sanitização de nomes reservados") {
    // Nomes reservados no Windows que causam travamentos em chamadas de I/O
    const std::vector<std::string> reservedNames = {
        "CON", "PRN", "AUX", "NUL", "COM1", "COM9", "LPT1", "LPT9"
    };

    for (const auto& reserved : reservedNames) {
        const std::string sanitized = core::Security::sanitizeFilename(reserved);
        // Não deve ser idêntico ao nome proibido nem conter caracteres perigosos
        CHECK(sanitized != reserved);
    }
}

TEST_CASE("CSV Injection (Formula / DDE Attack Mitigation)") {
    // Fórmulas comuns de injeção em folhas de cálculo (Excel, LibreOffice)
    const std::vector<std::string> injectionPayloads = {
        "=cmd|'/c calc'!A0",
        "+cmd|'/c calc'!A0",
        "-cmd|'/c calc'!A0",
        "@SUM(A1:A10)",
        "\t=2+2",
        "\r=1+1"
    };

    for (const auto& payload : injectionPayloads) {
        const std::string sanitized = core::Security::sanitizeCsvField(payload);
        // O primeiro caracter nunca pode ser um disparador de fórmula sem estar neutralizado
        CHECK(!sanitized.empty());
        CHECK(sanitized[0] != '=');
        CHECK(sanitized[0] != '+');
        CHECK(sanitized[0] != '-');
        CHECK(sanitized[0] != '@');
    }
}

TEST_CASE("Side-Channel Resistance: constantTimeEquals") {
    const std::string secret = "4f738012bcfe123498aef00192384756";
    const std::string match = "4f738012bcfe123498aef00192384756";
    const std::string diffAtStart = "0f738012bcfe123498aef00192384756";
    const std::string diffAtEnd = "4f738012bcfe123498aef00192384750";
    const std::string diffLength = "4f738012bcfe";

    CHECK(core::Security::constantTimeEquals(secret, match) == true);
    CHECK(core::Security::constantTimeEquals(secret, diffAtStart) == false);
    CHECK(core::Security::constantTimeEquals(secret, diffAtEnd) == false);
    CHECK(core::Security::constantTimeEquals(secret, diffLength) == false);
    CHECK(core::Security::constantTimeEquals("", "") == true);
    CHECK(core::Security::constantTimeEquals(secret, "") == false);
}

} // TEST_SUITE
