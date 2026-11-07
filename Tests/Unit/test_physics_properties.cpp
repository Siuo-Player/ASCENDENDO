#include "doctest/doctest.h"

#include "Core/Config.h"
#include "Logic/Level.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Logic/TickInput.h"

#include <cmath>
#include <cstdint>
#include <random>

namespace {

// Simple deterministic Linear Congruential Generator for reproducible PBT
class PbtRng {
public:
    explicit PbtRng(uint64_t seed = 1337) : state_(seed) {}

    uint32_t next() {
        state_ = state_ * 6364136223846793005ULL + 1442695040888963407ULL;
        return static_cast<uint32_t>(state_ >> 32);
    }

    float nextFloat(float min, float max) {
        const float unit = static_cast<float>(next()) / 4294967295.0f;
        return min + unit * (max - min);
    }

    bool nextBool() {
        return (next() & 1) != 0;
    }

private:
    uint64_t state_;
};

} // namespace

TEST_SUITE("PhysicsPropertyBasedTesting") {

TEST_CASE("PBT Invariant 1: Player never penetrates below implicit ground (Y >= 16.0)") {
    PbtRng rng(42);
    logic::Player player;
    player.body.position = { 320.0f, 16.0f };
    player.body.isGrounded = true;

    logic::PhysicsWorld world;
    logic::Level level;
    level.addPlatform(0.0f, 0.0f, config::LOGICAL_WIDTH, 16.0f);

    constexpr int NUM_TICKS = 5000;
    for (int t = 0; t < NUM_TICKS; ++t) {
        logic::TickInput input;
        input.left = rng.nextBool();
        input.right = rng.nextBool();
        input.jumpHeld = rng.nextBool();
        input.jumpPressed = rng.nextBool();
        input.jumpReleased = rng.nextBool();

        player.update(input, world, config::FIXED_STEP);
        level.resolveCollision(player.body);

        // INVARIANT: Player feet must stay at or above Y=16.0f
        REQUIRE(player.body.position.y >= 16.0f - 0.001f);
        REQUIRE(std::isfinite(player.body.position.x));
        REQUIRE(std::isfinite(player.body.position.y));
        REQUIRE(player.body.position.x >= 0.0f);
        REQUIRE(player.body.position.x <= config::LOGICAL_WIDTH - config::PLAYER_WIDTH);
    }
}

TEST_CASE("PBT Invariant 2: MTV resolution guarantees zero interior penetration with platforms") {
    PbtRng rng(101);

    for (int trial = 0; trial < 200; ++trial) {
        logic::Level level;
        level.addPlatform(0.0f, 0.0f, config::LOGICAL_WIDTH, 16.0f);

        // Generate 2 random platforms above ground
        const float p1_x = rng.nextFloat(20.0f, 150.0f);
        const float p1_y = rng.nextFloat(32.0f, 100.0f);
        level.addPlatform(p1_x, p1_y, 120.0f, 16.0f);

        const float p2_x = rng.nextFloat(250.0f, 400.0f);
        const float p2_y = rng.nextFloat(120.0f, 200.0f);
        level.addPlatform(p2_x, p2_y, 100.0f, 16.0f);

        logic::Player player;
        // Start above platforms or on the ground
        player.body.position = { rng.nextFloat(10.0f, 500.0f), rng.nextFloat(220.0f, 320.0f) };
        player.body.velocity = { rng.nextFloat(-100.0f, 100.0f), rng.nextFloat(-200.0f, 50.0f) };

        logic::PhysicsWorld world;

        // Run 60 ticks of continuous physics
        for (int t = 0; t < 60; ++t) {
            logic::TickInput input;
            input.left = rng.nextBool();
            input.right = rng.nextBool();
            if (t % 15 == 0) input.jumpPressed = true;
            if (t % 15 == 10) input.jumpReleased = true;

            player.update(input, world, config::FIXED_STEP);
            level.resolveCollision(player.body);

            // Check MTV invariant: player bounding box does not penetrate platform interiors
            const auto pBounds = player.body.bounds();
            for (const auto& plat : level.platforms()) {
                const bool strictlyInside =
                    pBounds.min.x < plat.bounds.max.x - 0.01f &&
                    pBounds.max.x > plat.bounds.min.x + 0.01f &&
                    pBounds.min.y < plat.bounds.max.y - 0.01f &&
                    pBounds.max.y > plat.bounds.min.y + 0.01f;
                CHECK_FALSE(strictlyInside);
            }
        }
    }
}

TEST_CASE("PBT Invariant 3: Commitment Jump apex height is monotonic with charge and strictly bounded by 138px") {
    float prevApex = 0.0f;
    constexpr int SAMPLES = 50;

    for (int i = 0; i <= SAMPLES; ++i) {
        const float chargeRatio = static_cast<float>(i) / static_cast<float>(SAMPLES);
        const float chargeTime = chargeRatio * config::PLAYER_CHARGE_TIME;

        // Calculate theoretical initial vertical launch velocity
        const float speed = config::PLAYER_MIN_JUMP +
                            (config::PLAYER_MAX_JUMP - config::PLAYER_MIN_JUMP) * chargeRatio;
        constexpr float sin60 = 0.86602540378f;
        const float vy0 = speed * sin60;

        // Theoretical continuous apex height
        const float theoreticalMax = (vy0 * vy0) / (2.0f * std::abs(config::GRAVITY));

        // Simulate jump in discrete physics
        logic::Player player;
        player.body.position = { 320.0f, 16.0f };
        player.body.isGrounded = true;

        // Charge
        player.isCharging = true;
        player.jumpCharge = chargeTime;

        logic::PhysicsWorld world;
        logic::TickInput releaseInput;
        releaseInput.jumpReleased = true;

        player.update(releaseInput, world, config::FIXED_STEP);
        CHECK_FALSE(player.body.isGrounded);

        float apexY = player.body.position.y;
        logic::TickInput inFlight;

        // Fly until falling back down
        while (player.body.velocity.y > 0.0f) {
            player.update(inFlight, world, config::FIXED_STEP);
            if (player.body.position.y > apexY) {
                apexY = player.body.position.y;
            }
        }

        const float deltaApex = apexY - 16.0f;

        // INVARIANT 3.1: Apex delta must never exceed 138px
        CHECK(deltaApex <= 138.0f);

        // INVARIANT 3.2: Discrete apex matches theoretical within 1 physics tick step
        const float tickStepTolerance = (speed * sin60) * config::FIXED_STEP;
        CHECK(deltaApex <= theoreticalMax + tickStepTolerance);

        // INVARIANT 3.3: Higher charge strictly produces higher jump (monotonicity)
        if (i > 0) {
            CHECK(deltaApex >= prevApex - 0.001f);
        }
        prevApex = deltaApex;
    }
}

TEST_CASE("PBT Invariant 4: Bit-for-bit deterministic simulation under identical input sequences") {
    PbtRng rng(999);
    std::vector<logic::TickInput> sequence;
    for (int i = 0; i < 600; ++i) {
        logic::TickInput input;
        input.left = rng.nextBool();
        input.right = rng.nextBool();
        input.jumpHeld = rng.nextBool();
        input.jumpPressed = rng.nextBool();
        input.jumpReleased = rng.nextBool();
        sequence.push_back(input);
    }

    // Run A
    logic::Player playerA;
    playerA.body.position = { 320.0f, 16.0f };
    playerA.body.isGrounded = true;
    logic::PhysicsWorld worldA;

    // Run B
    logic::Player playerB;
    playerB.body.position = { 320.0f, 16.0f };
    playerB.body.isGrounded = true;
    logic::PhysicsWorld worldB;

    logic::Level level;

    for (std::size_t t = 0; t < sequence.size(); ++t) {
        playerA.update(sequence[t], worldA, config::FIXED_STEP);
        level.resolveCollision(playerA.body);

        playerB.update(sequence[t], worldB, config::FIXED_STEP);
        level.resolveCollision(playerB.body);

        // INVARIANT: Bit-exact equality across every single tick
        CHECK(playerA.body.position.x == playerB.body.position.x);
        CHECK(playerA.body.position.y == playerB.body.position.y);
        CHECK(playerA.body.velocity.x == playerB.body.velocity.x);
        CHECK(playerA.body.velocity.y == playerB.body.velocity.y);
        CHECK(playerA.body.isGrounded == playerB.body.isGrounded);
    }
}

} // TEST_SUITE
