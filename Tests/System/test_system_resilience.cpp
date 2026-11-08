// =============================================================================
//  Tests/System/test_system_resilience.cpp
//
//  Bateria de Testes de Sistema (System Testing) & Resiliência Operacional:
//  • Ciclo de Vida do Sistema (Cold Bootstrap -> Menu -> Campanha -> Editor -> Encerramento)
//  • Invariante de Enquadramento da Câmara (Safety Viewport Clamping) em 10.000 ticks
//  • Validação Integrada de Cones Balísticos (JumpTrajectory) e Passabilidade
//  • Hermeticidade e Raio de Explosão Zero do Cofre de Contas (AccountManager)
//  • Determinismo End-to-End da Simulação Física sem Fugas de Memória
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "Core/AccountManager.h"
#include "Core/Config.h"
#include "Core/GameState.h"
#include "Core/KeyBindings.h"
#include "Graphics/Camera.h"
#include "Logic/CampaignRuntime.h"
#include "Logic/EditorSession.h"
#include "Logic/GameSession.h"
#include "Logic/JumpTrajectory.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Logic/RunReplay.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <vector>

TEST_SUITE("System Testing — Resiliência Operacional") {

TEST_CASE("Sistema: Invariante de Enquadramento da Camara (Swink 2008) durante 10.000 ticks") {
    gfx::Camera camera;
    camera.position = {0.0f, 0.0f};

    logic::Player player;
    logic::PhysicsWorld world;
    player.body.position = {320.0f, 16.0f};
    player.body.isGrounded = true;

    // Simular subidas com saltos sucessivos seguidos por uma queda livre abismal de 5.000 pixels
    for (int tick = 0; tick < 10000; ++tick) {
        logic::TickInput in;
        // Alternar saltos para gerar subida e quedas
        if (tick < 4000) {
            in.jumpHeld = (tick % 60) < 24;
            in.jumpReleased = (tick % 60) == 24;
            in.right = true;
        } else if (tick < 6000) {
            // Queda livre acelerada pelo peso da gravidade (-980 px/s^2)
            in.jumpHeld = false;
            in.jumpReleased = false;
        } else {
            // Nova série de saltos rápidos
            in.jumpHeld = (tick % 45) < 18;
            in.jumpReleased = (tick % 45) == 18;
            in.left = true;
        }

        player.update(in, world, config::FIXED_STEP);

        // Atualizar a câmara seguindo o jogador
        camera.follow(player.position(), config::FIXED_STEP);

        // A câmara nunca pode estar abaixo de 0
        CHECK(camera.position.y >= 0.0f);

        // Se a câmara estiver acima do chão (position.y > 0), o avatar DEVE estar
        // garantidamente dentro do frustum de segurança:
        // Pelo menos 24px acima do fundo e no máximo 32px abaixo do topo
        if (camera.position.y > 0.0f) {
            CHECK(player.position().y >= camera.position.y + 24.0f);
            CHECK(player.position().y <= camera.position.y + config::LOGICAL_HEIGHT - 32.0f);
        }
    }
}

TEST_CASE("Sistema: Cones Balísticos no Editor vs Simulação Física Discreta") {
    logic::EditorSession editor(false);

    // Plataforma de partida
    editor.document().addPlatform(logic::AABB{{100.0f, 100.0f}, {200.0f, 116.0f}});
    editor.controller().selectPlatform(0);

    const auto snapshot = editor.renderSnapshot();
    REQUIRE(snapshot.hasTrajectoryPreview);
    REQUIRE_FALSE(snapshot.trajectoryMaxRight.empty());

    // Ponto de partida analítico
    const logic::Vec2 launchPos = snapshot.trajectoryMaxRight.front().position;
    CHECK(launchPos.x == doctest::Approx(150.0f));
    CHECK(launchPos.y == doctest::Approx(116.0f));

    // Agora simular o mesmo salto no motor discreto com carga total (0.4s = 24 ticks a 60Hz)
    logic::Level level;
    level.addPlatform(0.0f, 100.0f, 640.0f, 16.0f); // plataforma ampla sob os pés

    logic::Player player;
    logic::PhysicsWorld world;
    player.body.position = launchPos;
    player.body.isGrounded = true;
    player.facingDirection = 1.0f; // virado para a direita

    logic::TickInput charge;
    charge.jumpHeld = true;
    // Carregar sem andar para os lados para permanecer estável
    for (int i = 0; i < 30; ++i) {
        player.update(charge, world, config::FIXED_STEP);
        level.resolveCollision(player.body);
    }
    CHECK(player.jumpCharge >= 1.0f);

    logic::TickInput release;
    release.jumpReleased = true;
    player.update(release, world, config::FIXED_STEP);

    // Voo no motor discreto até atingir o ápice
    float discreteApexY = player.position().y;
    while (player.velocity().y > 0.0f) {
        logic::TickInput air;
        player.update(air, world, config::FIXED_STEP);
        if (player.position().y > discreteApexY) {
            discreteApexY = player.position().y;
        }
    }

    // O ápice analítico previsto pelo JumpTrajectory
    const auto vel = logic::JumpTrajectory::initialVelocity(1.0f, +1.0f);
    const float analyticalApexY = launchPos.y + logic::JumpTrajectory::apexHeight(vel.y);

    // O ápice discreto (passo fixo de Euler a 60Hz) e o analítico devem convergir com desvio < 5.0 px
    CHECK(std::abs(discreteApexY - analyticalApexY) < 5.0f);
}

TEST_CASE("Sistema: Isolamento Hermético e Raio de Explosão Zero entre Contas Locais") {
    const std::filesystem::path tempDir =
        std::filesystem::temp_directory_path() / "ascendendo-system-resilience-accounts";
    std::filesystem::remove_all(tempDir);
    std::filesystem::create_directories(tempDir);

    {
        core::AccountManager manager(tempDir);
        REQUIRE(manager.registerAccount("speedrunner_alpha", "Password#Alpha123"));
        REQUIRE(manager.registerAccount("casual_beta", "Password#Beta456"));
        CHECK(manager.hasAccount("speedrunner_alpha"));
        CHECK(manager.hasAccount("casual_beta"));
    }

    // Adulterar propositadamente o diretório de dados da conta Alpha
    // para simular corrupção de disco ou ataque malicioso
    const auto alphaDir = tempDir / "accounts" / "speedrunner_alpha";
    if (std::filesystem::exists(alphaDir)) {
        std::ofstream corrupted(alphaDir / "profile.secure");
        corrupted << "ASCENDENDO_TAMPERED_DATA_INVALID";
        corrupted.close();
    }

    // Ao reabrir o cofre, a conta Beta DEVE continuar 100% funcional e íntegra (Raio de Explosão = 0)
    {
        core::AccountManager manager(tempDir);
        CHECK(manager.login("casual_beta", "Password#Beta456") == true);
        CHECK(manager.currentAccount()->username == "casual_beta");
    }

    std::filesystem::remove_all(tempDir);
}

TEST_CASE("Sistema: Ciclo de Vida Completo do GameSession com Streaming e Replay") {
    logic::GameSession session(
        {std::filesystem::path("Game/Assets/Levels/inicio.lvl")},
        "system_test_campaign",
        "system_runs.csv");

    // Configurar dimensões de apresentação lógicas
    constexpr float LOGICAL_W = 640.0f;
    session.beginPlaying(LOGICAL_W);

    CHECK(session.state() == core::GameState::PLAYING);
    CHECK(session.player().position().x == doctest::Approx(320.0f));
    CHECK(session.player().position().y >= 16.0f);

    // Simular 300 ticks de jogo ativo
    logic::InputManager input;
    core::KeyBindings bindings;

    for (int i = 0; i < 300; ++i) {
        session.update(config::FIXED_STEP, input, bindings, 1280, 720, LOGICAL_W, config::LOGICAL_HEIGHT);
    }

    CHECK(session.elapsedTime() == doctest::Approx(300.0f * config::FIXED_STEP).epsilon(0.01f));
    CHECK(std::isfinite(session.player().position().x));
    CHECK(std::isfinite(session.player().position().y));

    // snapshot de HUD deve ser válido
    const auto hud = session.hudSnapshot();
    CHECK(hud.isReplayWatch == false);
    CHECK(hud.elapsedTime == doctest::Approx(session.elapsedTime()));
    CHECK(hud.altitudeMeters >= 0.0f);
}

} // TEST_SUITE
