// =============================================================================
//  Tests/Unit/test_physics.cpp
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/Physics.h"

#include <limits>

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
        CHECK(a.overlaps(b) == false);
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
        body.position = {0, 100};
        body.isGrounded = false;

        world.step(body, PhysicsWorld::FIXED_STEP);

        CHECK(body.velocity.y < 0.0f);
        CHECK(body.position.y < 100.0f);
    }

    TEST_CASE("corpo no chao nao cai") {
        PhysicsWorld world;
        PhysicsBody body;
        body.position = {0, 0};
        body.isGrounded = true;

        world.step(body, PhysicsWorld::FIXED_STEP);

        CHECK(body.position.y == doctest::Approx(0.0f));
        CHECK(body.isGrounded == true);
    }

    TEST_CASE("salto aplica velocidade para cima") {
        PhysicsWorld world;
        PhysicsBody body;
        body.position = {0, 0};
        body.isGrounded = true;

        world.jump(body, 500.0f);

        CHECK(body.velocity.y == doctest::Approx(500.0f));
        CHECK(body.isGrounded == false);
    }

    TEST_CASE("salto no ar e ignorado") {
        PhysicsWorld world;
        PhysicsBody body;
        body.isGrounded = false;
        body.velocity = {};

        world.jump(body, 500.0f);

        CHECK(body.velocity.y == doctest::Approx(0.0f));
    }

    TEST_CASE("corpo cai e aterra") {
        PhysicsWorld world;
        PhysicsBody body;
        body.position = {0, 100};
        body.isGrounded = false;

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

    TEST_CASE("Fixed Timestep: frame lento pequeno nao perde passos") {
        PhysicsWorld world;
        const int steps = world.advance(0.1f);
        CHECK(steps == 6);
    }

    TEST_CASE("Fixed Timestep: frame muito longo e limitado") {
        PhysicsWorld world;
        const int steps = world.advance(10.0f);
        CHECK(steps == PhysicsWorld::MAX_STEPS_PER_ADVANCE);
        CHECK(world.accumulator() < PhysicsWorld::FIXED_STEP);
    }

    TEST_CASE("Fixed Timestep: tempo invalido nao altera simulacao") {
        PhysicsWorld world;
        const float before = world.accumulator();

        CHECK(world.advance(std::numeric_limits<float>::quiet_NaN()) == 0);
        CHECK(world.advance(std::numeric_limits<float>::infinity()) == 0);
        CHECK(world.advance(-1.0f) == 0);
        CHECK(world.accumulator() == doctest::Approx(before));
    }
}
