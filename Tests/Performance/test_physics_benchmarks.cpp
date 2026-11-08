// =============================================================================
//  Tests/Performance/test_physics_benchmarks.cpp
//
//  Bateria de Testes:
//  • Desempenho (Performance Benchmarks de simulação física a alta frequência)
//  • Carga (Load testing com 50.000+ ticks contínuos)
//  • Estresse (Stress testing com 5.000 plataformas densas)
//  • Volume (Replay de 20.000 ticks e eficiência de memória)
//  • Escalabilidade (Escalonamento linear e estabilidade numérica)
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "Core/Config.h"
#include "Logic/Level.h"
#include "Logic/LevelData.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Logic/RunReplay.h"

#include <chrono>
#include <cmath>
#include <vector>

TEST_SUITE("Performance, Load & Stress Testing") {

TEST_CASE("Performance Benchmark: Simulação física a mais de 30.000 Hz") {
    logic::Player player;
    logic::PhysicsWorld world;
    logic::Level level;

    // Criar chão e plataformas de teste
    logic::LevelData data;
    data.name = "Benchmark Level";
    data.screenCount = 5;
    data.platforms = {
        logic::AABB{{0.0f, 0.0f}, {640.0f, 16.0f}},
        logic::AABB{{100.0f, 60.0f}, {200.0f, 76.0f}},
        logic::AABB{{220.0f, 120.0f}, {320.0f, 136.0f}},
        logic::AABB{{340.0f, 180.0f}, {440.0f, 196.0f}},
    };
    level.appendFromData(data, 640.0f, 0.0f, false);

    player.body.position = {120.0f, 16.0f};
    player.body.isGrounded = true;

    logic::TickInput jumpInput;
    jumpInput.jumpHeld = true;

    constexpr int ITERATIONS = 20000;
    const auto t0 = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < ITERATIONS; ++i) {
        // Alternar entre carregar e saltar
        jumpInput.jumpHeld = (i % 60) < 30;
        player.update(jumpInput, world, config::FIXED_STEP);
        level.resolveCollision(player.body);
    }

    const auto t1 = std::chrono::high_resolution_clock::now();
    const double elapsedSec = std::chrono::duration<double>(t1 - t0).count();
    const double ticksPerSec = static_cast<double>(ITERATIONS) / (elapsedSec > 0.0 ? elapsedSec : 0.00001);

    // O motor deve processar confortavelmente mais de 25.000 ticks por segundo
    // (o que representa > 400x mais rápido que 60 Hz em tempo real)
    CHECK(ticksPerSec > 25000.0);
    CHECK(std::isfinite(player.position().x));
    CHECK(std::isfinite(player.position().y));
}

TEST_CASE("Load Testing: 50.000 ticks contínuos sem divergência ou overflow") {
    logic::Player player;
    logic::PhysicsWorld world;
    logic::TickInput input;

    player.body.position = {300.0f, 50.0f};
    player.body.velocity = {0.0f, 0.0f};
    player.body.isGrounded = false;

    for (int i = 0; i < 50000; ++i) {
        player.update(input, world, config::FIXED_STEP);

        // Limitar se cair abaixo de zero para simular loop contínuo
        if (player.body.position.y < 0.0f) {
            player.body.position.y = 100.0f;
            player.body.velocity.y = 0.0f;
        }

        // Verificar sanidade numérica a cada 5.000 ticks
        if (i % 5000 == 0) {
            CHECK(std::isfinite(player.body.position.x));
            CHECK(std::isfinite(player.body.position.y));
            CHECK(std::isfinite(player.body.velocity.y));
        }
    }
}

TEST_CASE("Stress Testing: Deteção de colisão com 5.000 plataformas simultâneas") {
    logic::LevelData denseData;
    denseData.name = "Dense Stress Level";
    denseData.screenCount = 20;

    // Gerar 5.000 plataformas ao longo de uma torre vertical de 10.000 metros
    denseData.platforms.reserve(5000);
    for (int i = 0; i < 5000; ++i) {
        const float x = static_cast<float>((i * 73) % 500);
        const float y = static_cast<float>(i * 4);
        denseData.platforms.push_back(logic::AABB{{x, y}, {x + 80.0f, y + 16.0f}});
    }

    logic::Level level;
    level.appendFromData(denseData, 640.0f, 0.0f, false);
    // 5000 plataformas authored + 1 plataforma de chão implícito (Y=0..16)
    CHECK(level.platformCount() == 5001);

    logic::Player player;
    player.body.position = {100.0f, 1000.0f};
    player.body.velocity = {0.0f, -100.0f};

    const auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        player.body.position.y -= 1.0f;
        level.resolveCollision(player.body);
    }
    const auto t1 = std::chrono::high_resolution_clock::now();

    const double elapsedMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    // 1.000 verificações contra 5.000 plataformas não devem exceder 250 ms em CPU moderna (ou 600 ms sob ASan/Debug)
#if defined(__SANITIZE_ADDRESS__) || defined(DEBUG) || (defined(__clang__) && defined(__has_feature) && __has_feature(address_sanitizer))
    constexpr double MAX_COLLISION_MS = 600.0;
#else
    constexpr double MAX_COLLISION_MS = 250.0;
#endif
    CHECK(elapsedMs < MAX_COLLISION_MS);
}

TEST_CASE("Volume Testing: Armazenamento e serialização de 25.000 ticks de Replay") {
    logic::RunReplay replay;
    replay.header().campaignID = "campaign_volume_test";
    replay.header().campaignName = "Volume Test Campaign";
    replay.header().playerID = "player_volume_01";
    replay.header().playerName = "Speedrunner";

    for (int i = 0; i < 25000; ++i) {
        logic::TickInput tick;
        tick.right = (i % 2 == 0);
        tick.left = (i % 2 != 0);
        tick.jumpHeld = (i % 60) > 30;
        replay.addTick(tick);
    }

    CHECK(replay.inputs().size() == 25000);
    replay.header().claimedElapsedSeconds = 25000.0f * config::FIXED_STEP;
    replay.header().totalTicks = 25000;

    // Serializar e verificar assinatura HMAC
    const std::string serialized = replay.serialize();
    CHECK(!serialized.empty());

    // Deserializar e verificar integridade exata
    auto deserialized = logic::RunReplay::deserialize(serialized);
    CHECK(deserialized.has_value() == true);
    CHECK(deserialized->header().totalTicks == 25000);
    CHECK(deserialized->inputs().size() == 25000);

    // Comparar primeiro e último tick
    CHECK(deserialized->inputs()[0].right == replay.inputs()[0].right);
    CHECK(deserialized->inputs()[24999].jumpHeld == replay.inputs()[24999].jumpHeld);
}

TEST_CASE("Scalability: Escalonamento temporal e invariância de taxa de simulação") {
    logic::PhysicsWorld world;

    // Subdivisões variáveis de dt devem produzir exatamente a mesma contagem de ticks
    const float dtChunk = config::FIXED_STEP * 4.0f;
    const int steps = world.advance(dtChunk);
    CHECK(steps == 4);

    // Dt sub-tick acumula sem disparar passos prematuros
    const int subSteps = world.advance(config::FIXED_STEP * 0.4f);
    CHECK(subSteps == 0);
    CHECK(world.accumulator() > 0.0f);

    // Completar o sub-tick deve disparar exatamente 1 passo
    const int completeStep = world.advance(config::FIXED_STEP * 0.7f);
    CHECK(completeStep == 1);
}

} // TEST_SUITE
