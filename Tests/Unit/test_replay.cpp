// =============================================================================
//  Tests/Unit/test_replay.cpp
//
//  @version 7.1
//  @history
//    v3.3 — criado (testes unitários para o ReplayManager e Save States)
//    v6.1 — atualizado para garantir isGrounded=true, respeitando a nova física
//    v7.0 — replay validado com TickInput e estado comparado após cada tick
//    v7.1 — explicita que grouping evidence usa a mesma sequência semântica
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/Physics.h"
#include "../../Game/Logic/Player.h"
#include "../../Game/Logic/InputManager.h"
#include "../../Game/Logic/ReplayManager.h"
#include <vector>

using namespace logic;

namespace {

struct PlayerSnapshot {
    Vec2 position;
    Vec2 velocity;
    bool grounded;
    float jumpCharge;
    bool charging;
    float accumulator;
};

PlayerSnapshot snapshot(const Player& player, const PhysicsWorld& world) {
    return {
        player.position(),
        player.velocity(),
        player.body.isGrounded,
        player.jumpCharge,
        player.isCharging,
        world.accumulator()
    };
}

void checkSnapshot(const PlayerSnapshot& expected,
                   const Player& actualPlayer,
                   const PhysicsWorld& actualWorld) {
    CHECK(actualPlayer.position().x == expected.position.x);
    CHECK(actualPlayer.position().y == expected.position.y);
    CHECK(actualPlayer.velocity().x == expected.velocity.x);
    CHECK(actualPlayer.velocity().y == expected.velocity.y);
    CHECK(actualPlayer.body.isGrounded == expected.grounded);
    CHECK(actualPlayer.jumpCharge == expected.jumpCharge);
    CHECK(actualPlayer.isCharging == expected.charging);
    CHECK(actualWorld.accumulator() == expected.accumulator);
}

} // namespace

TEST_SUITE("Fase 3.3 — Save States & Replay System") {

    TEST_CASE("Save State Estático — Salvar e Carregar Slots") {
        PhysicsWorld world;
        Player player;
        ReplayManager replay;

        player.body.position = { 150.0f, 80.0f };
        player.body.velocity = { 200.0f, -50.0f };
        player.body.isGrounded = true;
        player.jumpCharge = 0.75f;
        player.isCharging = true;
        world.advance(0.025f);

        const float originalAcc = world.accumulator();
        replay.saveState(1, player, world);

        player.body.position = { 999.0f, 999.0f };
        player.body.velocity = { 0.0f, 0.0f };
        player.jumpCharge = 0.0f;
        player.isCharging = false;
        world.advance(0.1f);

        const bool success = replay.loadState(1, player, world);
        REQUIRE(success == true);

        CHECK(player.body.position.x == 150.0f);
        CHECK(player.body.position.y == 80.0f);
        CHECK(player.body.velocity.x == 200.0f);
        CHECK(player.body.velocity.y == -50.0f);
        CHECK(player.jumpCharge == 0.75f);
        CHECK(player.isCharging == true);
        CHECK(world.accumulator() == originalAcc);
    }

    TEST_CASE("Continuous Recording e Rewind por tick") {
        PhysicsWorld world;
        Player player;
        ReplayManager replay;

        player.body.isGrounded = true;
        const TickInput tickInput{false, true, false, false, false};

        replay.recordTick(player, world, tickInput);
        player.update(tickInput, world, PhysicsWorld::FIXED_STEP);
        const Vec2 posTick1 = player.position();

        replay.recordTick(player, world, tickInput);
        player.update(tickInput, world, PhysicsWorld::FIXED_STEP);
        const Vec2 posTick2 = player.position();

        replay.recordTick(player, world, tickInput);
        player.update(tickInput, world, PhysicsWorld::FIXED_STEP);

        CHECK(replay.getReplayLength() == 3);

        const bool rew1 = replay.rewind(player, world);
        CHECK(rew1 == true);
        CHECK(player.position() == posTick2);

        const bool rew2 = replay.rewind(player, world);
        CHECK(rew2 == true);
        CHECK(player.position() == posTick1);
    }

    TEST_CASE("Replay determinístico — estado reproduzido após cada tick") {
        PhysicsWorld worldRecording;
        Player playerRecording;
        ReplayManager replay;
        playerRecording.body.isGrounded = true;

        const std::vector<TickInput> sequence = {
            TickInput{true, false, true, true, false},
            TickInput{true, false, true, false, false},
            TickInput{true, false, true, false, false},
            TickInput{true, false, false, false, true},
            TickInput{false, false, false, false, false}
        };

        std::vector<PlayerSnapshot> expectedStates;
        expectedStates.reserve(sequence.size());

        for (const TickInput& input : sequence) {
            replay.recordTick(playerRecording, worldRecording, input);
            playerRecording.update(input, worldRecording, PhysicsWorld::FIXED_STEP);
            expectedStates.push_back(snapshot(playerRecording, worldRecording));
        }

        REQUIRE(replay.getReplayLength() == sequence.size());

        PhysicsWorld worldPlayback;
        Player playerPlayback;
        playerPlayback.body.isGrounded = true;

        replay.startPlayback();
        for (size_t tick = 0; tick < expectedStates.size(); ++tick) {
            TickInput playbackInput{};
            REQUIRE(replay.preparePlaybackTick(playbackInput) == true);
            CHECK(playbackInput.left == sequence[tick].left);
            CHECK(playbackInput.right == sequence[tick].right);
            CHECK(playbackInput.jumpHeld == sequence[tick].jumpHeld);
            CHECK(playbackInput.jumpPressed == sequence[tick].jumpPressed);
            CHECK(playbackInput.jumpReleased == sequence[tick].jumpReleased);

            playerPlayback.update(playbackInput, worldPlayback, PhysicsWorld::FIXED_STEP);
            checkSnapshot(expectedStates[tick], playerPlayback, worldPlayback);
        }

        CHECK(replay.isPlaybackComplete() == true);
        TickInput unused{};
        CHECK(replay.preparePlaybackTick(unused) == false);
    }

    TEST_CASE("Mesma sequência semântica de ticks é independente de agrupamento externo") {
        const std::vector<TickInput> sequence = {
            TickInput{true, false, true, true, false},
            TickInput{true, false, true, false, false},
            TickInput{true, false, false, false, true},
            TickInput{false, true, false, false, false}
        };

        PhysicsWorld worldA;
        PhysicsWorld worldB;
        Player playerA;
        Player playerB;
        playerA.body.isGrounded = true;
        playerB.body.isGrounded = true;

        // Schedule A: one semantic tick at a time.
        for (const TickInput& input : sequence) {
            playerA.update(input, worldA, PhysicsWorld::FIXED_STEP);
        }

        // Schedule B: the same semantic TickInput sequence is grouped into
        // two-tick batches. This intentionally does not claim that live
        // frame-level GLFW edge sampling is frame-rate independent; that is a
        // separate property from replaying an already semantic tick sequence.
        for (size_t i = 0; i < sequence.size(); i += 2) {
            playerB.update(sequence[i], worldB, PhysicsWorld::FIXED_STEP);
            if (i + 1 < sequence.size()) {
                playerB.update(sequence[i + 1], worldB, PhysicsWorld::FIXED_STEP);
            }
        }

        CHECK(playerB.position().x == playerA.position().x);
        CHECK(playerB.position().y == playerA.position().y);
        CHECK(playerB.velocity().x == playerA.velocity().x);
        CHECK(playerB.velocity().y == playerA.velocity().y);
        CHECK(playerB.body.isGrounded == playerA.body.isGrounded);
        CHECK(playerB.jumpCharge == playerA.jumpCharge);
        CHECK(playerB.isCharging == playerA.isCharging);
        CHECK(worldB.accumulator() == worldA.accumulator());
    }

    TEST_CASE("Replay vazio não produz ticks") {
        ReplayManager replay;
        TickInput input{};

        replay.startPlayback();

        CHECK(replay.getReplayLength() == 0);
        CHECK(replay.isPlaybackComplete() == true);
        CHECK(replay.preparePlaybackTick(input) == false);
    }
}
