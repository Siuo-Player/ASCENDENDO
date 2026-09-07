#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/Level.h"
#include "../../Game/Logic/Player.h"
#include "../../Game/Logic/TickInput.h"
#include "../../Game/Core/Config.h"

using namespace logic;

TEST_SUITE("RuntimePlayability") {
    TEST_CASE("player can jump and land on an elevated platform using runtime physics") {
        Level level;
        level.addPlatform(0.0f, 0.0f, 220.0f, 10.0f);
        level.addPlatform(50.0f, 10.0f, 120.0f, 10.0f);

        Player player;
        player.body.position = {10.0f, 10.0f};
        player.body.isGrounded = true;
        player.facingDirection = 1.0f;

        PhysicsWorld world;
        TickInput jump;
        jump.jumpPressed = true;
        jump.jumpReleased = true;

        bool landedElevated = false;
        for (int tick = 0; tick < 120; ++tick) {
            player.update(tick == 0 ? jump : TickInput{}, world, config::FIXED_STEP);
            level.resolveCollision(player.body);

            if (player.isGrounded() && player.position().y == 20.0f) {
                landedElevated = true;
                break;
            }
        }

        REQUIRE(landedElevated);
        CHECK(player.position().x >= 50.0f - player.body.width);
        CHECK(player.position().x <= 170.0f);
        CHECK(player.velocity().y == 0.0f);
    }
}
