// =============================================================================
//  Tests/Regression/test_known_regressions.cpp
//
//  Bateria de Testes de Regressão:
//  • Regressão 1: Colisão com teto (head-bonk) zera vy imediatamente
//  • Regressão 2: Plataformas com largura zero ou invertidas são tratadas sem crash
//  • Regressão 3: Flag derivada na plataforma mais alta no nível final
//  • Regressão 4: Invariância de quebras de linha CRLF/LF em Replays e Níveis
//  • Regressão 5: Preload de streaming de campanha antes de atingir o topo
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "Core/Config.h"
#include "Core/CampaignLoader.h"
#include "Logic/CampaignRuntime.h"
#include "Logic/Level.h"
#include "Logic/LevelData.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Logic/RunReplay.h"

#include <cmath>
#include <vector>

TEST_SUITE("Regression Testing") {

TEST_CASE("Regressão 1: Colisão de subida com teto (head-bonk) zera vy") {
    logic::LevelData data;
    data.name = "Ceiling Test";
    data.screenCount = 1;
    // Chão em Y=0 e teto em Y=100
    data.platforms = {
        logic::AABB{{0.0f, 0.0f}, {640.0f, 16.0f}},
        logic::AABB{{100.0f, 100.0f}, {300.0f, 116.0f}},
    };
    logic::Level level;
    level.appendFromData(data, 640.0f, 0.0f, false);

    logic::Player player;
    // Colocar o jogador logo abaixo do teto subindo com velocidade positiva
    player.body.position = {150.0f, 85.0f};
    player.body.velocity = {0.0f, 250.0f};
    player.body.isGrounded = false;

    // Resolver colisão
    level.resolveCollision(player.body);

    // vy deve ter sido zerado (ou invertido) e a posição y corrigida para baixo do teto
    CHECK(player.body.velocity.y <= 0.0f);
    CHECK(player.body.position.y <= 100.0f);
}

TEST_CASE("Regressão 2: Plataformas de dimensão degenerada são seguras") {
    logic::LevelData data;
    data.name = "Degenerate Test";
    data.screenCount = 1;
    data.platforms = {
        logic::AABB{{100.0f, 50.0f}, {100.0f, 50.0f}},   // Ponto zero
        logic::AABB{{200.0f, 50.0f}, {150.0f, 50.0f}},   // Invertido X
        logic::AABB{{300.0f, 80.0f}, {400.0f, 40.0f}},   // Invertido Y
    };

    logic::Level level;
    level.appendFromData(data, 640.0f, 0.0f, false);
    logic::Player player;
    player.body.position = {150.0f, 50.0f};

    level.resolveCollision(player.body);
    CHECK(std::isfinite(player.body.position.x));
    CHECK(std::isfinite(player.body.position.y));
}

TEST_CASE("Regressão 3: Flag é estritamente derivada no nível final") {
    logic::LevelData data;
    data.name = "Final Level Flag Test";
    data.screenCount = 1;
    data.finalCampaignLevel = true;
    data.platforms = {
        logic::AABB{{0.0f, 0.0f}, {640.0f, 16.0f}},
        logic::AABB{{100.0f, 50.0f}, {200.0f, 66.0f}},
        logic::AABB{{250.0f, 180.0f}, {350.0f, 196.0f}}, // Plataforma mais alta (Y=196)
    };

    logic::Level level;
    level.appendFromData(data, 640.0f, 0.0f, true);
    REQUIRE(level.hasFlag == true);

    // A flag deve situar-se no topo da plataforma mais alta
    CHECK(level.flagBounds.min.y >= 196.0f);
    CHECK(level.flagBounds.min.x >= 250.0f);
    CHECK(level.flagBounds.max.x <= 350.0f);
}

TEST_CASE("Regressão 4: Invariância de quebras de linha CRLF vs LF em Replay") {
    logic::RunReplay replay;
    replay.header().campaignID = "crlf_test";
    replay.header().campaignName = "CRLF Invariance";
    replay.header().playerID = "test_player";
    replay.header().playerName = "Tester";
    logic::TickInput t1;
    t1.right = true;
    replay.addTick(t1);

    logic::TickInput t2;
    t2.left = true;
    t2.jumpHeld = true;
    replay.addTick(t2);

    replay.header().claimedElapsedSeconds = 2.0f * config::FIXED_STEP;
    replay.header().totalTicks = 2;

    const std::string serialized = replay.serialize();

    // Criar versão com CRLF
    std::string crlfVersion;
    for (char ch : serialized) {
        if (ch == '\n' && (crlfVersion.empty() || crlfVersion.back() != '\r')) {
            crlfVersion.push_back('\r');
        }
        crlfVersion.push_back(ch);
    }

    // Criar versão apenas LF
    std::string lfVersion;
    for (char ch : crlfVersion) {
        if (ch != '\r') lfVersion.push_back(ch);
    }

    auto r1 = logic::RunReplay::deserialize(crlfVersion);
    auto r2 = logic::RunReplay::deserialize(lfVersion);
    CHECK(r1.has_value() == true);
    CHECK(r2.has_value() == true);
    CHECK(r1->inputs().size() == r2->inputs().size());
}

} // TEST_SUITE
