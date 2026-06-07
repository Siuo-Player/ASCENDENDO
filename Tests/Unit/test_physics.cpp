// =============================================================================
//  Tests/Unit/test_physics.cpp
//
//  @version 3.1
//  @history
//    v3.1 — criado (Vec2, AABB, step, jump, Fixed Timestep)
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/Physics.h"

using namespace logic;

TEST_SUITE("Physics / Vec2") {

    TEST_CASE("soma") {
        Vec2 r = Vec2{3, 4} + Vec2{1, 2};
        CHECK(r.x == doctest::Approx(4.0f));
        CHECK(r.y == doctest::Approx(6.0f));
    }

    TEST_CASE("subtracao") {
        Vec2 r = Vec2{5, 7} - Vec2{2, 3};
        CHECK(r.x == doctest::Approx(3.0f));
        CHECK(r.y == doctest::Approx(4.0f));
    }

    TEST_CASE("escalar") {
        Vec2 r = Vec2{2, 3} * 2.0f;
        CHECK(r.x == doctest::Approx(4.0f));
        CHECK(r.y == doctest::Approx(6.0f));
    }

    TEST_CASE("operador +=") {
        Vec2 v{1, 1};
        v += Vec2{2, 3};
        CHECK(v.x == doctest::Approx(3.0f));
        CHECK(v.y == doctest::Approx(4.0f));
    }
}

TEST_SUITE("Physics / AABB") {

    TEST_CASE("sobreposicao") {
        AABB a{{0,0},{10,10}}, b{{5,5},{15,15}};
        CHECK(a.overlaps(b) == true);
        CHECK(PhysicsWorld::collides(a, b) == true);
    }

    TEST_CASE("separados") {
        AABB a{{0,0},{10,10}}, b{{11,11},{20,20}};
        CHECK(a.overlaps(b) == false);
    }

    TEST_CASE("adjacentes nao colidem") {
        AABB a{{0,0},{10,10}}, b{{10,0},{20,10}};
        CHECK(a.overlaps(b) == false); // toque de borda nao e colisao
    }

    TEST_CASE("simetria: A colide B == B colide A") {
        AABB a{{0,0},{10,10}}, b{{5,5},{15,15}};
        CHECK(a.overlaps(b) == b.overlaps(a));
    }
}

TEST_SUITE("Physics / PhysicsWorld") {

    TEST_CASE("gravidade aplica-se quando no ar") {
        PhysicsWorld world;
        PhysicsBody body;
        body.position   = {0, 100};
        body.isGrounded = false;

        world.step(body, PhysicsWorld::FIXED_STEP);

        CHECK(body.velocity.y < 0.0f);
        CHECK(body.position.y < 100.0f);
    }

    TEST_CASE("corpo no chao nao cai") {
        PhysicsWorld world;
        PhysicsBody body;
        body.position   = {0, 0};
        body.isGrounded = true;

        world.step(body, PhysicsWorld::FIXED_STEP);

        CHECK(body.position.y == doctest::Approx(0.0f));
        CHECK(body.isGrounded == true);
    }

    TEST_CASE("salto aplica velocidade para cima") {
        PhysicsWorld world;
        PhysicsBody body;
        body.position   = {0, 0};
        body.isGrounded = true;

        world.jump(body, 500.0f);

        CHECK(body.velocity.y == doctest::Approx(500.0f));
        CHECK(body.isGrounded == false);
    }

    TEST_CASE("salto no ar e ignorado") {
        PhysicsWorld world;
        PhysicsBody body;
        body.isGrounded = false;
        body.velocity   = {};

        world.jump(body, 500.0f);

        CHECK(body.velocity.y == doctest::Approx(0.0f));
    }

    TEST_CASE("corpo cai e aterra") {
        PhysicsWorld world;
        PhysicsBody body;
        body.position   = {0, 100};
        body.isGrounded = false;

        // Simular ate ao chao (maximo 600 frames ~= 10s)
        for (int i = 0; i < 600 && !body.isGrounded; ++i) {
            world.step(body, PhysicsWorld::FIXED_STEP);
        }

        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(0.0f));
    }

    TEST_CASE("Fixed Timestep: 1s a 60fps = 60 passos") {
        PhysicsWorld world;
        int total = 0;
        for (int i = 0; i < 60; ++i) {
            total += world.advance(1.0f / 60.0f);
        }
        CHECK(total == 60);
    }

    TEST_CASE("Fixed Timestep: frame lento nao perde passos") {
        PhysicsWorld world;
        // Frame de 100ms deve gerar 6 passos (6 * 16.6ms ~= 100ms)
        int steps = world.advance(0.1f);
        CHECK(steps == 6);
    }
}
