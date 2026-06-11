// =============================================================================
//  Tests/Unit/test_level.cpp
//
//  @version 6.2c
//  @history
//    v6.2  — criado (10 testes: construcao, topo, queda, borda)
//    v6.2c — +3 testes colisao lateral (esquerda, direita, subida ok)
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/Level.h"
#include "Logic/Physics.h"

using namespace logic;

// ── Helper ───────────────────────────────────────────────────────────────────
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
        CHECK(p.bounds.min.y == doctest::Approx(100.0f));
        CHECK(p.bounds.max.x == doctest::Approx(250.0f));
        CHECK(p.bounds.max.y == doctest::Approx(120.0f));
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
        body.position   = {100.0f, 200.0f};
        body.isGrounded = false;

        simulate(body, world, level, 200);

        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(120.0f));
    }

    TEST_CASE("corpo permanece na plataforma frame a frame") {
        Level level;
        level.addPlatform(0.0f, 100.0f, 640.0f, 20.0f);

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {100.0f, 200.0f};
        body.isGrounded = false;

        simulate(body, world, level, 200);
        REQUIRE(body.isGrounded == true);
        REQUIRE(body.position.y == doctest::Approx(120.0f));

        simulate(body, world, level, 30);

        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(120.0f));
    }

    TEST_CASE("corpo a subir atravessa plataforma por baixo (one-way)") {
        Level level;
        level.addPlatform(0.0f, 100.0f, 640.0f, 20.0f); // topo em Y=120

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {50.0f, 115.0f}; // dentro da plataforma
        body.velocity.y = 300.0f;           // a subir
        body.isGrounded = false;

        world.step(body, PhysicsWorld::FIXED_STEP);
        bool collided = level.resolveCollision(body);

        CHECK(collided == false);
        CHECK(body.isGrounded == false);
        CHECK(body.position.y > 115.0f);
    }

    TEST_CASE("corpo fora do intervalo X nao colide com a plataforma") {
        Level level;
        level.addPlatform(100.0f, 100.0f, 100.0f, 20.0f); // X=[100,200]

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {400.0f, 200.0f};
        body.isGrounded = false;

        simulate(body, world, level, 300);

        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(0.0f));
    }

    TEST_CASE("nivel vazio nao interfere com colisao do chao") {
        Level level;

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {0.0f, 50.0f};
        body.isGrounded = false;

        simulate(body, world, level, 200);

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
        body.position   = {100.0f, 400.0f};
        body.isGrounded = false;

        simulate(body, world, level, 300);

        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(220.0f));
    }

    TEST_CASE("corpo cai apos sair lateralmente da plataforma") {
        Level level;
        level.addPlatform(100.0f, 100.0f, 100.0f, 20.0f); // X=[100,200], topo=120

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {120.0f, 200.0f};
        body.isGrounded = false;

        simulate(body, world, level, 200);
        REQUIRE(body.isGrounded == true);
        REQUIRE(body.position.y == doctest::Approx(120.0f));

        body.position.x = 350.0f; // fora do X da plataforma

        world.step(body, PhysicsWorld::FIXED_STEP);
        level.resolveCollision(body);

        CHECK(body.isGrounded == false);
    }
}

// ═════════════════════════════════════════════════════════════════════════════

TEST_SUITE("Level / Colisao Lateral") {

    TEST_CASE("corpo bloqueado ao entrar pela esquerda da plataforma") {
        Level level;
        // Plataforma alta: X=[100,200], Y=[50,150] — corpo entra lateralmente
        level.addPlatform(100.0f, 50.0f, 100.0f, 100.0f);

        PhysicsBody body;
        // Corpo sobrepoe 5px pela esquerda da plataforma, ao nivel do meio
        // body.max.x = 73+32=105 (5px dentro), body.min.y=70 (abaixo do topo 150)
        body.position   = {73.0f, 70.0f};
        body.velocity.x = 200.0f; // a mover para a direita
        body.velocity.y = 0.0f;
        body.isGrounded = true;

        bool collided = level.resolveCollision(body);

        CHECK(collided == true);
        CHECK(body.velocity.x == doctest::Approx(0.0f));
        // Corpo empurrado para fora da borda esquerda
        CHECK(body.position.x + body.width <= doctest::Approx(100.0f));
    }

    TEST_CASE("corpo bloqueado ao entrar pela direita da plataforma") {
        Level level;
        level.addPlatform(100.0f, 50.0f, 100.0f, 100.0f); // X=[100,200], Y=[50,150]

        PhysicsBody body;
        // Corpo sobrepoe 5px pela direita: body.min.x=195, body.max.x=227
        body.position   = {195.0f, 70.0f};
        body.velocity.x = -200.0f; // a mover para a esquerda
        body.velocity.y = 0.0f;
        body.isGrounded = true;

        bool collided = level.resolveCollision(body);

        CHECK(collided == true);
        CHECK(body.velocity.x == doctest::Approx(0.0f));
        // Corpo empurrado para fora da borda direita
        CHECK(body.position.x == doctest::Approx(200.0f));
    }

    TEST_CASE("corpo a cair nao e empurrado lateralmente (colisao de topo)") {
        Level level;
        level.addPlatform(100.0f, 50.0f, 100.0f, 100.0f); // topo em Y=150

        PhysicsWorld world;
        PhysicsBody  body;
        body.position   = {130.0f, 300.0f}; // acima da plataforma, no centro em X
        body.isGrounded = false;
        // Sem velocidade horizontal: deve pousar no topo, nao ser empurrado

        simulate(body, world, level, 300);

        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(150.0f));
        // X nao deve ter mudado (nao houve colisao lateral)
        CHECK(body.position.x == doctest::Approx(130.0f));
    }
}
