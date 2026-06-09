// =============================================================================
//  Tests/Unit/test_player.cpp
//
//  @version 6.1
//  @history
//    v6.1 — atualizado para validar Salto Parabólico e bloqueio aéreo de input.
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/Player.h"
#include "../../Game/Logic/Physics.h"
#include "../../Game/Logic/InputManager.h"

using namespace logic;

TEST_SUITE("Player / Movimento e Salto") {

    TEST_CASE("Controlo Horizontal no chao") {
        Player p; PhysicsWorld w; InputManager i;
        p.body.isGrounded = true;

        i.injectRawState(false, true, false, false, false); // RIGHT
        p.update(i, w, PhysicsWorld::FIXED_STEP);
        CHECK(p.velocity().x == doctest::Approx(Player::MOVE_SPEED));
        CHECK(p.facingDirection == 1.0f);

        i.injectRawState(true, false, false, false, false); // LEFT
        p.update(i, w, PhysicsWorld::FIXED_STEP);
        CHECK(p.velocity().x == doctest::Approx(-Player::MOVE_SPEED));
        CHECK(p.facingDirection == -1.0f);
    }

    TEST_CASE("Salto Parabolico e Perda de Controlo Aereo") {
        Player p; PhysicsWorld w; InputManager i;
        p.body.isGrounded = true;

        i.injectRawState(false, true, false, false, false);
        p.update(i, w, PhysicsWorld::FIXED_STEP);
        
        i.injectRawState(false, false, false, true, true);
        p.update(i, w, PhysicsWorld::FIXED_STEP);

        CHECK(p.isGrounded() == false);
        
        // Correção matemática: A gravidade atua logo no frame em que o jogador sai do chão!
        float expectedY = (Player::MIN_JUMP_FORCE * 0.866f) + (PhysicsWorld::GRAVITY * PhysicsWorld::FIXED_STEP);
        float expectedX = Player::MIN_JUMP_FORCE * 0.5f;

        CHECK(p.velocity().y == doctest::Approx(expectedY));
        CHECK(p.velocity().x == doctest::Approx(expectedX));

        // Tentar inverter direcao NO AR
        i.injectRawState(true, false, false, false, false); // Input: ESQUERDA
        p.update(i, w, PhysicsWorld::FIXED_STEP);

        // GARANTIR que a inercia da parábola não foi alterada pelo jogador!
        CHECK(p.velocity().x == doctest::Approx(expectedX));
    }
}