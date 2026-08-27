#include "doctest/doctest.h"
#include "Logic/Level.h"
#include "Logic/Physics.h"

using namespace logic;

namespace {
void simulate(PhysicsBody& body, PhysicsWorld& world, const Level& level, int steps) {
    for (int i = 0; i < steps; ++i) {
        world.step(body, PhysicsWorld::FIXED_STEP);
        level.resolveCollision(body);
    }
}
}

TEST_SUITE("Level / Colisao Vertical (Topo e Teto)") {
    TEST_CASE("corpo cai e pousa no topo da plataforma") {
        Level level; level.addPlatform(0.0f, 100.0f, 640.0f, 20.0f);
        PhysicsWorld world; PhysicsBody body;
        body.position = {100.0f, 200.0f}; body.isGrounded = false;
        simulate(body, world, level, 200);
        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(120.0f));
    }
    TEST_CASE("corpo a subir bate no teto") {
        Level level; level.addPlatform(0.0f, 100.0f, 640.0f, 20.0f);
        PhysicsWorld world; PhysicsBody body;
        body.position = {50.0f, 80.0f}; body.velocity.y = 300.0f; body.velocity.x = 100.0f; body.isGrounded = false;
        world.step(body, PhysicsWorld::FIXED_STEP);
        bool collided = level.resolveCollision(body);
        CHECK(collided == true);
        CHECK(body.isGrounded == false);
        CHECK(body.position.y == doctest::Approx(100.0f - body.height));
        float expectedVy = (300.0f + PhysicsWorld::GRAVITY * PhysicsWorld::FIXED_STEP) * -0.3f;
        CHECK(body.velocity.y == doctest::Approx(expectedVy));
        CHECK(body.velocity.x == doctest::Approx(90.0f));
    }
    TEST_CASE("nivel vazio nao interfere com colisao do chao") {
        Level level; PhysicsWorld world; PhysicsBody body;
        body.position = {0.0f, 50.0f}; body.isGrounded = false;
        simulate(body, world, level, 200);
        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(0.0f));
    }
}

TEST_SUITE("Level / Fisica com Nivel") {
    TEST_CASE("plataforma mais alta intercepta antes do chao") {
        Level level; level.addPlatform(0.0f, 200.0f, 640.0f, 20.0f);
        PhysicsWorld world; PhysicsBody body;
        body.position = {100.0f, 400.0f}; body.isGrounded = false;
        simulate(body, world, level, 300);
        CHECK(body.isGrounded == true);
        CHECK(body.position.y == doctest::Approx(220.0f));
    }
    TEST_CASE("corpo cai apos sair lateralmente da plataforma") {
        Level level; level.addPlatform(100.0f, 100.0f, 100.0f, 20.0f);
        PhysicsWorld world; PhysicsBody body;
        body.position = {120.0f, 200.0f}; body.isGrounded = false;
        simulate(body, world, level, 200);
        REQUIRE(body.isGrounded == true);
        body.position.x = 350.0f;
        world.step(body, PhysicsWorld::FIXED_STEP);
        level.resolveCollision(body);
        CHECK(body.isGrounded == false);
    }
}

TEST_SUITE("Level / Colisao Lateral") {
    TEST_CASE("corpo bloqueado ao entrar pela esquerda da plataforma") {
        Level level; level.addPlatform(100.0f, 50.0f, 100.0f, 100.0f);
        PhysicsBody body;
        body.position = {89.0f, 70.0f}; body.velocity.x = 200.0f; body.velocity.y = 0.0f; body.isGrounded = true;
        bool collided = level.resolveCollision(body);
        CHECK(collided == true);
        CHECK(body.velocity.x == doctest::Approx(-60.0f));
        CHECK(body.position.x + body.width <= doctest::Approx(100.0f));
    }
    TEST_CASE("corpo bloqueado ao entrar pela direita da plataforma") {
        Level level; level.addPlatform(100.0f, 50.0f, 100.0f, 100.0f);
        PhysicsBody body;
        body.position = {195.0f, 70.0f}; body.velocity.x = -200.0f; body.velocity.y = 0.0f; body.isGrounded = true;
        bool collided = level.resolveCollision(body);
        CHECK(collided == true);
        CHECK(body.velocity.x == doctest::Approx(60.0f));
        CHECK(body.position.x == doctest::Approx(200.0f));
    }
}

TEST_SUITE("Level / Collision Order Determinism") {
    TEST_CASE("same overlapping contact set is invariant to platform order") {
        Level orderAB;
        orderAB.addPlatform(100.0f, 50.0f, 100.0f, 100.0f);
        orderAB.addPlatform(150.0f, 50.0f, 100.0f, 100.0f);

        Level orderBA;
        orderBA.addPlatform(150.0f, 50.0f, 100.0f, 100.0f);
        orderBA.addPlatform(100.0f, 50.0f, 100.0f, 100.0f);

        PhysicsBody bodyAB;
        bodyAB.position = {140.0f, 70.0f};
        bodyAB.velocity = {200.0f, 0.0f};
        bodyAB.isGrounded = true;

        PhysicsBody bodyBA = bodyAB;

        const bool collidedAB = orderAB.resolveCollision(bodyAB);
        const bool collidedBA = orderBA.resolveCollision(bodyBA);

        REQUIRE(collidedAB == true);
        REQUIRE(collidedBA == true);
        CHECK(bodyAB.position.x == doctest::Approx(bodyBA.position.x));
        CHECK(bodyAB.position.y == doctest::Approx(bodyBA.position.y));
        CHECK(bodyAB.velocity.x == doctest::Approx(bodyBA.velocity.x));
        CHECK(bodyAB.velocity.y == doctest::Approx(bodyBA.velocity.y));
        CHECK(bodyAB.isGrounded == bodyBA.isGrounded);
    }
}
