// =============================================================================
//  Tests/Unit/test_player.cpp
//
//  @version 7.1
//  @history
//    v6.1  — Commitment Jump + bloqueio aereo
//    v6.2c — cair da borda: velocity.x zerada
//    v6.7  — cair da borda: preserva 100% da inércia X (física normal)
//    v7.1  — atualizado para usar constantes globais de Config.h
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/Player.h"
#include "Logic/Physics.h"
#include "Logic/InputManager.h"
#include "Core/Config.h" // Importação das constantes globais

using namespace logic;

TEST_SUITE("Player / Movimento e Salto") {

    TEST_CASE("Controlo Horizontal no chao") {
        Player p; PhysicsWorld w; InputManager i;
        p.body.isGrounded = true;

        i.injectRawState(false, true, false, false, false); // RIGHT
        p.update(i, w, PhysicsWorld::FIXED_STEP);
        CHECK(p.velocity().x == doctest::Approx(config::PLAYER_MOVE_SPEED));
        CHECK(p.facingDirection == 1.0f);

        i.injectRawState(true, false, false, false, false); // LEFT
        p.update(i, w, PhysicsWorld::FIXED_STEP);
        CHECK(p.velocity().x == doctest::Approx(-config::PLAYER_MOVE_SPEED));
        CHECK(p.facingDirection == -1.0f);
    }

    TEST_CASE("Salto Parabolico e Perda de Controlo Aereo") {
        Player p; PhysicsWorld w; InputManager i;
        p.body.isGrounded = true;

        i.injectRawState(false, true, false, false, false);
        p.update(i, w, PhysicsWorld::FIXED_STEP);

        i.injectRawState(false, false, false, true, true); // SPACE tap
        p.update(i, w, PhysicsWorld::FIXED_STEP);

        CHECK(p.isGrounded() == false);

        float expectedY = (config::PLAYER_MIN_JUMP * 0.866f) + (PhysicsWorld::GRAVITY * PhysicsWorld::FIXED_STEP);
        float expectedX = config::PLAYER_MIN_JUMP * 0.5f;

        CHECK(p.velocity().y == doctest::Approx(expectedY));
        CHECK(p.velocity().x == doctest::Approx(expectedX));

        // Tentar inverter direcao no ar — inércia dita que não é possível
        i.injectRawState(true, false, false, false, false); // LEFT
        p.update(i, w, PhysicsWorld::FIXED_STEP);

        CHECK(p.velocity().x == doctest::Approx(expectedX));
    }
}

TEST_SUITE("Player / Queda de Borda") {

    TEST_CASE("cair da borda sem saltar: mantem inercia X a 100% (fisica normal)") {
        Player p; PhysicsWorld w; InputManager i;

        // Jogador a andar para a direita no chao
        p.body.isGrounded = true;
        i.injectRawState(false, true, false, false, false); // RIGHT
        p.update(i, w, PhysicsWorld::FIXED_STEP);
        REQUIRE(p.velocity().x == doctest::Approx(config::PLAYER_MOVE_SPEED));

        // Simular saida da borda: forcar estado aereo SEM salto
        p.body.isGrounded = false;
        p.body.position.y = 100.0f; // alto suficiente para nao aterrar logo

        // Proximo frame sem input
        i.injectRawState(false, false, false, false, false);
        p.update(i, w, PhysicsWorld::FIXED_STEP);

        // Física normal: momento mantém-se intacto
        CHECK(p.velocity().x == doctest::Approx(config::PLAYER_MOVE_SPEED));
    }
}