// =============================================================================
//  Tests/Unit/test_level.cpp
//
//  @version 6.2
//  @history
//    v6.2 — criado (10 testes: construcao, colisao de topo, queda, borda)
//
//  SEM dependencias de GPU. Testa o logic layer isolado.
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/Level.h"
#include "Logic/Physics.h"

using namespace logic;

// ── Helper ───────────────────────────────────────────────────────────────────
// Simula N passos de fisica + resolucao de nivel. Padrao de uso no game loop.
static void simulate(PhysicsBody& body, PhysicsWorld& world,
                     const Level& level, int steps)
{
    for (int i = 0; i < steps; ++i) {
        world.step(body, PhysicsWorld::FIXED_STEP);
        level.resolveCollision(body);
    }
}

// ═════════════════════════════════════════════════════════════════════════════

TEST_SUITE("Level / Construcao") {

    TEST_CASE("nivel vazio tem zero plataformas") {
        Level level;
        CHECK(level.platformCount() == 0);
        CHECK(level.platforms().empty());
    }

    TEST_CASE("addPlatform cria plataforma com bounds corretos") {
        Level level;
        level.addPlatform(50.0f, 100.0f, 200.0f, 20.0f);

        REQUIRE(level.platformCount() == 1);
        const auto& p = level.platforms()[0];
        CHECK(p.bounds.min.x == doctest::Approx(50.0f));
        CHECK(p.bounds.min.y == doctest::Approx(100.0f)); // base
        CHECK(p.bounds.max.x == doctest::Approx(250.0f));
        CHECK(p.bounds.max.y == doctest::Approx(120.0f)); // topo
        CHECK(p.bounds.width()  == doctest::Approx(200.0f));
        CHECK(p.bounds.height() == doctest::Approx(20.0f));
    }

    TEST_CASE("multiplas plataformas guardadas na ordem de insercao") {
        Level level;
        level.addPlatform(0.0f,  50.0f, 100.0f, 10.0f);
        level.addPlatform(0.0f, 150.0f, 100.0f, 10.0f);
        level.addPlatform(0.0f, 250.0f, 100.0f, 10.0f);

        REQUIRE(level.platformCount() == 3);
        CHECK(level.platforms()[0].bounds.min.y == doctest::Approx(50.0f));
        CHECK(level.platforms()[1].bounds.min.y == doctest::Approx(150.0f));
        CHECK(level.platforms()[2].bounds.min.y == doctest::Approx(250.0f));
    }

    TEST_CASE("clear() remove todas as plataformas") {
        Level level;
        level.addPlatform(0.0f, 100.0f, 100.0f, 10.0f);
        level.addPlatform(0.0f, 200.0f, 100.0f, 10.0f);
        REQUIRE(level.platformCount() == 2);

        level.clear();
        CHECK(level.platformCount() == 0);
    }
}

// ═════════════════════════════════════════════════════════════════════════════

TEST_SUITE("Level / Colisao de Topo") {

    TEST_CASE("corpo cai e pousa no topo da plataforma") {
        Level level;
        level.addPlatform(0.0f, 100.0f, 640.0f, 20.0f); // topo em Y=120

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {100.0f, 200.0f}; // acima da plataforma
        body.isGrounded = false;

        simulate(body, world, level, 200); // mais que suficiente para cair 80px

        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(120.0f));
    }

    TEST_CASE("corpo permanece na plataforma frame a frame") {
        Level level;
        level.addPlatform(0.0f, 100.0f, 640.0f, 20.0f); // topo em Y=120

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {100.0f, 200.0f};
        body.isGrounded = false;

        simulate(body, world, level, 200); // pousar
        REQUIRE(body.isGrounded == true);
        REQUIRE(body.position.y == doctest::Approx(120.0f));

        // 30 frames parado — isGrounded nao deve oscilar
        simulate(body, world, level, 30);

        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(120.0f));
    }

    TEST_CASE("corpo a subir atravessa plataforma livremente (one-way)") {
        Level level;
        level.addPlatform(0.0f, 100.0f, 640.0f, 20.0f); // topo em Y=120

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {50.0f, 115.0f}; // dentro da plataforma, a subir
        body.velocity.y = 300.0f;
        body.isGrounded = false;

        world.step(body, PhysicsWorld::FIXED_STEP);
        bool collided = level.resolveCollision(body);

        // velocity.y > 0 → nao deve pousar
        CHECK(collided == false);
        CHECK(body.isGrounded == false);
        CHECK(body.position.y > 115.0f); // subiu
    }

    TEST_CASE("corpo fora do intervalo X nao colide com a plataforma") {
        Level level;
        level.addPlatform(100.0f, 100.0f, 100.0f, 20.0f); // X=[100,200], topo=120

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {400.0f, 200.0f}; // fora do X da plataforma
        body.isGrounded = false;

        simulate(body, world, level, 300); // queda livre

        // Deve cair ao chao (Y=0), nunca pousar na plataforma
        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(0.0f));
    }

    TEST_CASE("nivel vazio nao interfere com colisao do chao") {
        Level level; // sem plataformas

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {0.0f, 50.0f};
        body.isGrounded = false;

        simulate(body, world, level, 200);

        // Cai ao chao normalmente sem nivel
        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(0.0f));
    }
}

// ═════════════════════════════════════════════════════════════════════════════

TEST_SUITE("Level / Fisica com Nivel") {

    TEST_CASE("plataforma mais alta intercepta antes do chao") {
        Level level;
        level.addPlatform(0.0f, 200.0f, 640.0f, 20.0f); // topo em Y=220

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {100.0f, 400.0f}; // cai de cima
        body.isGrounded = false;

        simulate(body, world, level, 300); // max ~0.6s para atingir Y=220

        // Deve pousar NA PLATAFORMA (Y=220), nao no chao (Y=0)
        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(220.0f));
    }

    TEST_CASE("corpo cai apos sair lateralmente da plataforma") {
        Level level;
        level.addPlatform(100.0f, 100.0f, 100.0f, 20.0f); // X=[100,200], topo=120

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {120.0f, 200.0f}; // acima da plataforma
        body.isGrounded = false;

        simulate(body, world, level, 200); // pousar
        REQUIRE(body.isGrounded == true);
        REQUIRE(body.position.y == doctest::Approx(120.0f));

        // Mover para fora dos limites X da plataforma
        body.position.x = 350.0f; // x=[350,382] nao sobrepo X=[100,200]

        world.step(body, PhysicsWorld::FIXED_STEP);
        level.resolveCollision(body);

        // Sem suporte → deve comecar a cair
        CHECK(body.isGrounded == false);
    }
}
