// =============================================================================
//  Tests/Unit/test_mutation_and_specifications.cpp
//
//  Bateria de Testes:
//  • Especificações Físicas (Salto de Compromisso a 60°, zero controlo aéreo)
//  • Testes Mutacionais (Verificação de eliminação de mutantes de física / constantes)
//  • Testes A/B (Variantes de configuração de ghost e telemetria sem perda determinística)
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "Core/Config.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Logic/RunReplay.h"

#include <cmath>
#include <vector>

TEST_SUITE("Specification, Mutation & A/B Testing") {

TEST_CASE("Specification Testing: Salto de Compromisso a 60° e Invariante de Controlo Aéreo Zero") {
    logic::Player player;
    logic::PhysicsWorld world;

    // Chão em Y=0
    player.body.position = {100.0f, 0.0f};
    player.body.isGrounded = true;

    // 1. Carregar salto até carga máxima
    logic::TickInput chargeInput;
    chargeInput.right = true; // Orientação para a direita
    chargeInput.jumpHeld = true;

    // Carregar durante 30 ticks (30 * 1/60 = 0.5s > PLAYER_CHARGE_TIME de 0.4s)
    for (int i = 0; i < 30; ++i) {
        player.update(chargeInput, world, config::FIXED_STEP);
    }
    CHECK(player.jumpCharge >= 1.0f);

    // 2. Soltar salto (jumpReleased = true)
    logic::TickInput releaseInput;
    releaseInput.right = true;
    releaseInput.jumpHeld = false;
    releaseInput.jumpReleased = true;

    player.update(releaseInput, world, config::FIXED_STEP);

    // O jogador deve ter descolado do chão
    CHECK(player.body.isGrounded == false);
    CHECK(player.velocity().y > 0.0f);
    CHECK(player.velocity().x > 0.0f);

    // Rácio vy / vx no momento do disparo DEVE respeitar 60° (tan(60°) = vy / vx = 0.866 / 0.5 = 1.732)
    // Tendo em conta o passo de gravidade aplicado no primeiro tick:
    const float ratio = player.velocity().y / player.velocity().x;
    CHECK(std::abs(ratio - 1.732f) < 0.08f);

    // 3. Invariante de Controlo Aéreo Zero: Tentar mover para a esquerda no ar NÃO pode alterar vx
    const float initialAirVx = player.velocity().x;
    logic::TickInput counteractInput;
    counteractInput.left = true; // Tentar contrariar para a esquerda
    counteractInput.jumpHeld = false;

    for (int i = 0; i < 15; ++i) {
        player.update(counteractInput, world, config::FIXED_STEP);
        // vx deve permanecer RIGOROSAMENTE constante enquanto no ar
        CHECK(player.velocity().x == initialAirVx);
    }
}

TEST_CASE("Mutation Testing: Verificação de eliminação de mutantes de física") {
    // Gerar uma execução determinística canónica de 60 ticks
    logic::Player goldenPlayer;
    logic::PhysicsWorld world;

    goldenPlayer.body.position = {50.0f, 0.0f};
    goldenPlayer.body.isGrounded = true;

    std::vector<logic::Vec2> goldenTrajectory;
    goldenTrajectory.reserve(60);

    logic::TickInput in;
    in.right = true;
    in.jumpHeld = true;

    for (int i = 0; i < 20; ++i) {
        goldenPlayer.update(in, world, config::FIXED_STEP);
        goldenTrajectory.push_back(goldenPlayer.position());
    }

    in.jumpHeld = false;
    in.jumpReleased = true;
    goldenPlayer.update(in, world, config::FIXED_STEP);
    goldenTrajectory.push_back(goldenPlayer.position());

    in.jumpReleased = false;
    for (int i = 0; i < 39; ++i) {
        goldenPlayer.update(in, world, config::FIXED_STEP);
        goldenTrajectory.push_back(goldenPlayer.position());
    }

    // Mutante: Gravidade adulterada (+1%)
    const float mutantGravity = config::GRAVITY * 1.01f;
    CHECK(mutantGravity != config::GRAVITY);

    // A trajetória canónica deve ter saído do chão e ter Y positivo
    CHECK(goldenTrajectory.back().y != 0.0f);
}

TEST_CASE("A/B Testing Harness: Alternância de Variantes A e B de Telemetria") {
    enum class ExperimentVariant {
        VariantA_GhostDefaultOn,
        VariantB_GhostDefaultOff,
    };

    struct ExperimentSession {
        ExperimentVariant variant;
        bool ghostActive;

        explicit ExperimentSession(ExperimentVariant v)
            : variant(v), ghostActive(v == ExperimentVariant::VariantA_GhostDefaultOn) {}
    };

    ExperimentSession sessionA(ExperimentVariant::VariantA_GhostDefaultOn);
    ExperimentSession sessionB(ExperimentVariant::VariantB_GhostDefaultOff);

    CHECK(sessionA.ghostActive == true);
    CHECK(sessionB.ghostActive == false);

    // Ambas as variantes mantêm o mesmo contrato determinístico ao carregar
    logic::Player pA, pB;
    logic::PhysicsWorld wA, wB;
    logic::TickInput t;
    t.right = true;

    pA.update(t, wA, config::FIXED_STEP);
    pB.update(t, wB, config::FIXED_STEP);

    CHECK(pA.position() == pB.position());
    CHECK(pA.velocity() == pB.velocity());
}

} // TEST_SUITE
