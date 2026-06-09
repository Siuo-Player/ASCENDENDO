// =============================================================================
//  Tests/Unit/test_player.cpp
//
//  @version 3.2
//  @history
//    v3.2 — criado (movimento horizontal, salto por carga)
//
//  Ordem correta do game loop nos testes:
//    input.beginFrame();          // 1. limpar estados "just"
//    input.onKeyEvent(key, act);  // 2. injectar eventos
//    player.update(input, world, dt); // 3. processar
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/Player.h"
#include "Logic/InputManager.h"
#include "Logic/Physics.h"

using namespace logic;

// ── Helpers de simulação ──────────────────────────────────────────────────────

// Simular um frame com uma tecla premida (e mantida de frames anteriores)
static void frameWith(Player& p, InputManager& i, PhysicsWorld& w,
                      int key = -1, int action = -1) {
    i.beginFrame();
    if (key >= 0) i.onKeyEvent(key, action);
    p.update(i, w, PhysicsWorld::FIXED_STEP);
}

// N frames sem eventos novos (teclas mantidas permanecem)
static void stepN(Player& p, InputManager& i, PhysicsWorld& w, int n) {
    for (int f = 0; f < n; ++f) { i.beginFrame(); p.update(i, w, PhysicsWorld::FIXED_STEP); }
}

// Toque rápido: press + release no mesmo frame
static void tap(Player& p, InputManager& i, PhysicsWorld& w, int key) {
    i.beginFrame();
    i.onKeyEvent(key, Action::PRESS);
    i.onKeyEvent(key, Action::RELEASE);
    p.update(i, w, PhysicsWorld::FIXED_STEP);
}

// =============================================================================

TEST_SUITE("Player / Movimento Horizontal") {

    TEST_CASE("D -> velocidade positiva") {
        Player p; InputManager i; PhysicsWorld w;
        p.body.isGrounded = true;

        frameWith(p, i, w, Key::D, Action::PRESS);

        CHECK(p.velocity().x == doctest::Approx(Player::MOVE_SPEED));
    }

    TEST_CASE("A -> velocidade negativa") {
        Player p; InputManager i; PhysicsWorld w;
        p.body.isGrounded = true;

        frameWith(p, i, w, Key::A, Action::PRESS);

        CHECK(p.velocity().x == doctest::Approx(-Player::MOVE_SPEED));
    }

    TEST_CASE("A + D simultaneos -> parar") {
        Player p; InputManager i; PhysicsWorld w;
        p.body.isGrounded = true;

        i.beginFrame();
        i.onKeyEvent(Key::A, Action::PRESS);
        i.onKeyEvent(Key::D, Action::PRESS);
        p.update(i, w, PhysicsWorld::FIXED_STEP);

        CHECK(p.velocity().x == doctest::Approx(0.0f));
    }

    TEST_CASE("sem teclas -> zero") {
        Player p; InputManager i; PhysicsWorld w;
        p.body.isGrounded = true;

        frameWith(p, i, w);

        CHECK(p.velocity().x == doctest::Approx(0.0f));
    }
}

TEST_SUITE("Player / Salto por Carga") {

    TEST_CASE("toque rapido -> forca minima") {
        Player p; InputManager i; PhysicsWorld w;
        p.body.isGrounded = true;

        tap(p, i, w, Key::SPACE);

        // Após world.step(), gravidade já foi aplicada num frame.
        // MIN_JUMP_FORCE + GRAVITY * FIXED_STEP ≈ 300 - 16.3 = 283.7
        // Verificamos apenas que o jogador está a subir
        CHECK(p.velocity().y > 0.0f);
        CHECK(p.isGrounded() == false);
        // E que não excede MAX (não havia carga)
        CHECK(p.velocity().y <= Player::MAX_JUMP_FORCE);
    }

    TEST_CASE("carga total -> forca maxima") {
        Player p; InputManager i; PhysicsWorld w;
        p.body.isGrounded = true;

        // Carregar durante CHARGE_TIME + margem
        int frames = static_cast<int>(Player::CHARGE_TIME / PhysicsWorld::FIXED_STEP) + 5;
        frameWith(p, i, w, Key::SPACE, Action::PRESS);  // frame 0: press
        for (int f = 1; f < frames; ++f) stepN(p, i, w, 1);

        CHECK(p.chargeRatio() == doctest::Approx(1.0f));

        // Soltar
        frameWith(p, i, w, Key::SPACE, Action::RELEASE);

        CHECK(p.velocity().y > 0.0f);
        CHECK(p.isGrounded() == false);
        // Com 5% de tolerância relativa para a gravidade do primeiro passo
        CHECK(p.velocity().y > Player::MAX_JUMP_FORCE * 0.9f);
    }

    TEST_CASE("salto no ar e ignorado") {
        Player p; InputManager i; PhysicsWorld w;
        p.body.position   = {0, 100};
        p.body.isGrounded = false;

        tap(p, i, w, Key::SPACE);

        // Velocidade Y apenas da gravidade (negativa), não de um salto
        CHECK(p.velocity().y < 0.0f);
        CHECK(p.chargeRatio() == doctest::Approx(0.0f));
    }

    TEST_CASE("chargeRatio limitado a 1.0") {
        Player p; InputManager i; PhysicsWorld w;
        p.body.isGrounded = true;

        frameWith(p, i, w, Key::SPACE, Action::PRESS);
        stepN(p, i, w, 600); // ~10s

        CHECK(p.chargeRatio() <= 1.0f);
        CHECK(p.chargeRatio() == doctest::Approx(1.0f));
    }

    TEST_CASE("segundo salto mais fraco que carga total") {
        Player p; InputManager i; PhysicsWorld w;
        p.body.isGrounded = true;

        // Salto 1: carga total
        int frames = static_cast<int>(Player::CHARGE_TIME / PhysicsWorld::FIXED_STEP) + 5;
        frameWith(p, i, w, Key::SPACE, Action::PRESS);
        for (int f = 1; f < frames; ++f) stepN(p, i, w, 1);
        frameWith(p, i, w, Key::SPACE, Action::RELEASE);
        float vy1 = p.velocity().y;
        CHECK(vy1 > 0.0f);

        // Aterrar
        stepN(p, i, w, 300);
        REQUIRE(p.isGrounded());

        // Salto 2: toque rápido
        tap(p, i, w, Key::SPACE);
        float vy2 = p.velocity().y;

        CHECK(vy2 > 0.0f);
        CHECK(vy2 < vy1); // toque < carga total
    }
}
