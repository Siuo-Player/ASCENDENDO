#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/Level.h"

using namespace logic;

TEST_SUITE("Level") {
    TEST_CASE("clear limpa geometria e metadata") {
        Level level;
        level.name = "Chunk antigo";
        level.hasFlag = true;
        level.flagBounds = {{1.0f, 2.0f}, {3.0f, 4.0f}};
        level.addPlatform(0.0f, 0.0f, 10.0f, 1.0f);

        REQUIRE(level.platformCount() == 1);

        level.clear();

        CHECK(level.platformCount() == 0);
        CHECK(level.name == "Sem Nome");
        CHECK_FALSE(level.hasFlag);
        CHECK(level.flagBounds.min == Vec2{});
        CHECK(level.flagBounds.max == Vec2{});
    }

    TEST_CASE("resolveCollision aterra um corpo em queda sobre uma plataforma") {
        Level level;
        level.addPlatform(100.0f, 50.0f, 200.0f, 10.0f);

        PhysicsBody body;
        body.position = {150.0f, 60.0f - body.height - 0.5f};
        body.velocity = {0.0f, -100.0f};
        body.isGrounded = false;

        CHECK(level.resolveCollision(body));
        CHECK(body.position.y == doctest::Approx(60.0f));
        CHECK(body.velocity.y == doctest::Approx(0.0f));
        CHECK(body.isGrounded);
    }

    TEST_CASE("resolveCollision impede atravessar o lado inferior da plataforma") {
        Level level;
        level.addPlatform(100.0f, 100.0f, 200.0f, 10.0f);

        PhysicsBody body;
        body.position = {150.0f, 99.0f};
        body.velocity = {0.0f, 100.0f};
        body.isGrounded = false;

        CHECK(level.resolveCollision(body));
        CHECK(body.position.y == doctest::Approx(100.0f - body.height));
        CHECK(body.velocity.y < 0.0f);
        CHECK_FALSE(body.isGrounded);
    }

    TEST_CASE("resolveCollision resolve impacto lateral sem transformar em estado grounded") {
        Level level;
        level.addPlatform(100.0f, 50.0f, 100.0f, 100.0f);

        PhysicsBody body;
        body.position = {90.0f, 80.0f};
        body.velocity = {100.0f, 0.0f};
        body.isGrounded = false;

        CHECK(level.resolveCollision(body));
        CHECK(body.position.x == doctest::Approx(100.0f - body.width));
        CHECK(body.velocity.x < 0.0f);
        CHECK_FALSE(body.isGrounded);
    }

    TEST_CASE("resolveCollision nao interfere num corpo separado") {
        Level level;
        level.addPlatform(100.0f, 50.0f, 100.0f, 10.0f);

        PhysicsBody body;
        body.position = {0.0f, 200.0f};
        body.velocity = {0.0f, -100.0f};
        body.isGrounded = false;

        CHECK_FALSE(level.resolveCollision(body));
        CHECK(body.position.x == doctest::Approx(0.0f));
        CHECK(body.position.y == doctest::Approx(200.0f));
        CHECK(body.velocity.y == doctest::Approx(-100.0f));
        CHECK_FALSE(body.isGrounded);
    }
}
