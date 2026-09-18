#include "../../external/doctest/doctest.h"
#include "../../Game/Core/Config.h"
#include "../../Game/Logic/Level.h"
#include "../../Game/Logic/LevelData.h"
#include "../../Game/Logic/Player.h"
#include "../../Game/Logic/ReplayManager.h"

#include <vector>

namespace {

using namespace logic;

struct StateSnapshot {
    Vec2 position;
    Vec2 velocity;
    bool grounded;
    float jumpCharge;
    bool charging;
    float accumulator;
};

StateSnapshot snapshot(const Player& player, const PhysicsWorld& world) {
    return {
        player.position(),
        player.velocity(),
        player.body.isGrounded,
        player.jumpCharge,
        player.isCharging,
        world.accumulator()
    };
}

void checkSnapshot(const StateSnapshot& expected,
                   const Player& player,
                   const PhysicsWorld& world) {
    CHECK(player.position() == expected.position);
    CHECK(player.velocity() == expected.velocity);
    CHECK(player.body.isGrounded == expected.grounded);
    CHECK(player.jumpCharge == expected.jumpCharge);
    CHECK(player.isCharging == expected.charging);
    CHECK(world.accumulator() == expected.accumulator);
}

bool goalReached(const Player& player, const Level& level) {
    return level.hasFlag &&
           PhysicsWorld::collides(player.body.bounds(), level.flagBounds);
}

Level makeEvidenceLevel() {
    LevelData data;
    data.name = "Replay Evidence";
    data.finalCampaignLevel = true;
    data.screenCount = 1;
    data.platforms = {
        AABB{{240.0f, 104.0f}, {400.0f, 120.0f}},
        AABB{{240.0f, 200.0f}, {400.0f, 216.0f}},
    };

    Level level;
    level.appendFromData(
        data,
        static_cast<float>(config::LOGICAL_WIDTH),
        0.0f,
        true);
    return level;
}

void chargeAndReleaseJump(Player& player,
                          PhysicsWorld& world,
                          Level& level,
                          ReplayManager& replay) {
    bool pressed = true;
    bool groundedBeforeJump = player.isGrounded();

    for (int tick = 0; tick < 24; ++tick) {
        const TickInput input{
            false,
            false,
            true,
            pressed,
            false,
        };
        replay.recordTick(player, world, input);
        player.update(input, world, PhysicsWorld::FIXED_STEP);
        level.resolveCollision(player.body);
        pressed = false;
    }

    REQUIRE(groundedBeforeJump);
    REQUIRE(player.isCharging());

    const TickInput release{
        false,
        false,
        false,
        false,
        true,
    };
    replay.recordTick(player, world, release);
    player.update(release, world, PhysicsWorld::FIXED_STEP);
    level.resolveCollision(player.body);

    REQUIRE_FALSE(player.isGrounded());
}

bool advanceUntilGroundedOrGoal(Player& player,
                                PhysicsWorld& world,
                                Level& level,
                                ReplayManager& replay,
                                int maxTicks) {
    for (int tick = 0; tick < maxTicks; ++tick) {
        const TickInput input{};
        replay.recordTick(player, world, input);
        player.update(input, world, PhysicsWorld::FIXED_STEP);
        level.resolveCollision(player.body);

        if (goalReached(player, level)) return true;
        if (player.isGrounded()) return false;
    }
    return false;
}

} // namespace

TEST_SUITE("ReplayRunEvidence") {
TEST_CASE("valid run replay reproduces the derived FLAG through the same engine tick sequence") {
    Level originalLevel = makeEvidenceLevel();
    Player originalPlayer;
    PhysicsWorld originalWorld;
    ReplayManager replay;

    originalPlayer.body.position = originalLevel.spawnPosition;
    originalPlayer.body.isGrounded = true;

    // Ground -> platform 1.
    chargeAndReleaseJump(originalPlayer, originalWorld, originalLevel, replay);
    REQUIRE_FALSE(advanceUntilGroundedOrGoal(
        originalPlayer, originalWorld, originalLevel, replay, 120));
    REQUIRE(originalPlayer.isGrounded());

    // Platform 1 -> platform 2.
    chargeAndReleaseJump(originalPlayer, originalWorld, originalLevel, replay);
    REQUIRE_FALSE(advanceUntilGroundedOrGoal(
        originalPlayer, originalWorld, originalLevel, replay, 120));
    REQUIRE(originalPlayer.isGrounded());

    // The derived FLAG starts at the top of the highest platform, so the
    // campaign completes on the same fixed tick that lands the player there.
    chargeAndReleaseJump(originalPlayer, originalWorld, originalLevel, replay);

    bool originalGoal = false;
    int originalGoalTick = -1;
    StateSnapshot originalFinal{};
    for (int tick = 0; tick < 120; ++tick) {
        const TickInput input{};
        replay.recordTick(originalPlayer, originalWorld, input);
        originalPlayer.update(input, originalWorld, PhysicsWorld::FIXED_STEP);
        originalLevel.resolveCollision(originalPlayer.body);

        if (goalReached(originalPlayer, originalLevel)) {
            originalGoal = true;
            originalGoalTick = static_cast<int>(replay.getReplayLength());
            originalFinal = snapshot(originalPlayer, originalWorld);
            break;
        }
    }

    REQUIRE(originalGoal);
    REQUIRE(originalGoalTick > 0);
    const size_t recordedTicks = replay.getReplayLength();

    Level replayLevel = makeEvidenceLevel();
    Player replayPlayer;
    PhysicsWorld replayWorld;
    replayPlayer.body.position = replayLevel.spawnPosition;
    replayPlayer.body.isGrounded = true;

    replay.startPlayback();

    bool replayGoal = false;
    int replayGoalTick = -1;
    StateSnapshot replayFinal{};

    while (!replay.isPlaybackComplete()) {
        TickInput input{};
        REQUIRE(replay.preparePlaybackTick(input));
        replayPlayer.update(input, replayWorld, PhysicsWorld::FIXED_STEP);
        replayLevel.resolveCollision(replayPlayer.body);

        if (goalReached(replayPlayer, replayLevel)) {
            replayGoal = true;
            replayGoalTick = static_cast<int>(replay.getCurrentPlaybackTick());
            replayFinal = snapshot(replayPlayer, replayWorld);
            break;
        }
    }

    REQUIRE(replayGoal);
    CHECK(replayGoalTick == originalGoalTick);
    CHECK(replay.getCurrentPlaybackTick() == static_cast<size_t>(originalGoalTick));
    CHECK(recordedTicks >= static_cast<size_t>(originalGoalTick));

    checkSnapshot(originalFinal, replayPlayer, replayWorld);
    CHECK(replay.isPlaybackComplete() == (replay.getCurrentPlaybackTick() == replay.getReplayLength()));
}
}
