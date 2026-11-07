// =============================================================================
//  Tests/E2E/test_e2e_gameplay_lifecycle.cpp
//
//  Bateria de Testes:
//  • Smoke testing (verificação rápida de arranque e prontidão)
//  • Sanity testing (invariantes de sanidade de estado e limites)
//  • End-to-End (E2E) (ciclo completo: menu -> jogar -> streaming -> flag -> replay -> watch -> menu)
//  • Funcionais e Não-funcionais (tempo de resposta, estabilidade de memória)
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "Core/Config.h"
#include "Core/GameStateMachine.h"
#include "Core/KeyBindings.h"
#include "Core/CampaignLoader.h"
#include "Logic/GameSession.h"
#include "Logic/InputManager.h"
#include "Logic/Physics.h"
#include "Logic/ProfileStorage.h"
#include "Logic/RunReplay.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <vector>

namespace {

std::filesystem::path createTemporaryCampaign(const std::string& testDir) {
    std::filesystem::create_directories(testDir);

    const std::string lvl1Path = testDir + "/level_01.lvl";
    std::ofstream lvl1(lvl1Path);
    lvl1 << "NAME E2E Level 1\n";
    lvl1 << "SCREENS 1\n";
    lvl1 << "PLATFORM 100 40 100 16\n";
    lvl1 << "PLATFORM 250 80 100 16\n";
    lvl1.close();

    const std::string lvl2Path = testDir + "/level_02.lvl";
    std::ofstream lvl2(lvl2Path);
    lvl2 << "NAME E2E Level 2 Final\n";
    lvl2 << "SCREENS 1\n";
    lvl2 << "PLATFORM 50 40 100 16\n";
    lvl2 << "PLATFORM 200 80 100 16\n";
    lvl2.close();

    const std::string playlistPath = testDir + "/campaign.txt";
    std::ofstream pl(playlistPath);
    pl << "level_01.lvl\n";
    pl << "level_02.lvl\n";
    pl.close();

    return std::filesystem::path(playlistPath);
}

} // namespace

TEST_SUITE("E2E & Smoke Testing") {

TEST_CASE("Smoke Test: GameStateMachine inicializa instantaneamente em MENU") {
    const auto start = std::chrono::high_resolution_clock::now();

    core::GameStateMachine sm;
    CHECK(sm.state() == core::GameState::MENU);
    CHECK(sm.menuSelection() == 0);

    sm.enterPlaying();
    CHECK(sm.state() == core::GameState::PLAYING);

    sm.enterReplayWatch();
    CHECK(sm.state() == core::GameState::REPLAY_WATCH);

    sm.returnToMenu();
    CHECK(sm.state() == core::GameState::MENU);

    const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::high_resolution_clock::now() - start);

    // Smoke test deve executar em microsegundos
    CHECK(elapsed.count() < 10000);
}

TEST_CASE("Sanity Test: Invariantes do jogador e física") {
    logic::Player player;
    logic::PhysicsWorld world;
    logic::TickInput idleInput;

    player.body.position = {100.0f, 100.0f};
    player.body.velocity = {0.0f, 0.0f};
    player.body.isGrounded = false;

    // Sanidade de gravidade: sem chão, jogador acelera para baixo (-Y no modelo do jogo)
    player.update(idleInput, world, config::FIXED_STEP);
    CHECK(player.velocity().y < 0.0f);
    CHECK(player.position().y < 100.0f);

    // Sanidade do acumulador da física
    const int steps = world.advance(config::FIXED_STEP * 2.5f);
    CHECK(steps == 2);
    CHECK(world.accumulator() >= 0.0f);
    CHECK(world.accumulator() < config::FIXED_STEP);
}

TEST_CASE("E2E Lifecycle: Campanha completa, gravação de replay, e modo Replay Watch") {
    const std::string testDir = "build/test_e2e_campaign_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto playlist = createTemporaryCampaign(testDir);
    (void)playlist;

    const std::string profileFile = testDir + "/Save/profile.secure";
    const std::string runsCsv = testDir + "/Runs/runs.csv";

    std::vector<std::filesystem::path> campaignLevels = {
        std::filesystem::path(testDir) / "level_01.lvl",
        std::filesystem::path(testDir) / "level_02.lvl"
    };

    logic::GameSession session(campaignLevels, "e2e_campaign", runsCsv, profileFile);
    CHECK(session.state() == core::GameState::MENU);

    // 1. Verificar configuração do Menu e toggle de Fantasma
    CHECK(session.ghostEnabled() == true);
    session.toggleGhost();
    CHECK(session.ghostEnabled() == false);
    session.toggleGhost();
    CHECK(session.ghostEnabled() == true);

    auto menuSnap = session.menuRenderSnapshot();
    CHECK(menuSnap.ghostEnabled == true);
    CHECK(menuSnap.activeHasReplay == false);

    // 2. Iniciar jogo
    session.beginPlaying(640.0f);
    CHECK(session.state() == core::GameState::PLAYING);
    CHECK(session.player().body.isGrounded == true);

    // 3. Simular alguns saltos com avanço de física
    logic::InputManager input;
    core::KeyBindings bindings;

    // Fazer uma subida simulada até à flag
    logic::Player& p = const_cast<logic::Player&>(session.player());
    logic::Level& lvl = const_cast<logic::Level&>(session.level());

    // Transportar para próximo da flag para simular vitória no teste E2E
    REQUIRE(lvl.hasFlag == false); // no nivel 1 não tem flag ainda pois há nivel 2
    // Forçar streaming do nivel 2
    session.campaignRuntime().streamNextLevel(lvl, 640.0f);
    CHECK(lvl.hasFlag == true);

    // Colocar o jogador sobre a flag para concluir campanha
    p.body.position = lvl.flagBounds.min;
    p.body.velocity = {0.0f, 0.0f};

    const auto res = session.update(config::FIXED_STEP, input, bindings, 640, 360, 640.0f, 360.0f);
    CHECK(res.campaignCompleted == true);
    CHECK(session.lastCompletedReplay().has_value() == true);
    CHECK(session.hasGhostReplay() == true);

    // 4. Voltar ao menu e verificar que o replay agora está disponível
    session.stateMachine().returnToMenu();
    CHECK(session.state() == core::GameState::MENU);

    menuSnap = session.menuRenderSnapshot();
    CHECK(menuSnap.activeHasReplay == true);

    // 5. Iniciar Replay Watch diretamente a partir do Menu
    const bool watchStarted = session.beginWatchingBestReplay(640.0f);
    CHECK(watchStarted == true);
    CHECK(session.state() == core::GameState::REPLAY_WATCH);
    CHECK(session.isWatchingReplay() == true);

    const auto hudSnap = session.hudSnapshot();
    CHECK(hudSnap.isReplayWatch == true);

    // 6. Avançar ticks no modo Replay Watch
    const auto resReplay = session.update(config::FIXED_STEP, input, bindings, 640, 360, 640.0f, 360.0f);
    CHECK(session.state() == core::GameState::REPLAY_WATCH);
    (void)resReplay;

    // 7. Sair do Replay Watch pressionando ESC ou Pause
    // Simular botão Pause / Quit
    session.stateMachine().returnToMenu();
    CHECK(session.state() == core::GameState::MENU);

    // Limpeza
    std::error_code ec;
    std::filesystem::remove_all(testDir, ec);
}

} // TEST_SUITE
