// =============================================================================
//  Tests/Unit/test_replay.cpp
//
//  @version 6.1
//  @history
//    v3.3 — criado (testes unitários para o ReplayManager e Save States)
//    v6.1 — atualizado para garantir isGrounded=true, respeitando a nova física
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/Physics.h"
#include "../../Game/Logic/Player.h"
#include "../../Game/Logic/InputManager.h"
#include "../../Game/Logic/ReplayManager.h"

using namespace logic;

TEST_SUITE("Fase 3.3 — Save States & Replay System") {

    TEST_CASE("Save State Estático — Salvar e Carregar Slots") {
        PhysicsWorld world;
        Player player;
        ReplayManager replay;

        player.body.position = { 150.0f, 80.0f };
        player.body.velocity = { 200.0f, -50.0f };
        player.body.isGrounded = true; // NOVA FÍSICA: garantir contacto com o chão
        player.jumpCharge    = 0.75f;
        player.isCharging    = true;
        world.advance(0.025f);

        float originalAcc = world.accumulator();
        replay.saveState(1, player, world);

        player.body.position = { 999.0f, 999.0f };
        player.body.velocity = { 0.0f, 0.0f };
        player.jumpCharge    = 0.0f;
        player.isCharging    = false;
        world.advance(0.1f);

        bool success = replay.loadState(1, player, world);
        REQUIRE(success == true);

        CHECK(player.body.position.x == 150.0f);
        CHECK(player.body.position.y == 80.0f);
        CHECK(player.body.velocity.x == 200.0f);
        CHECK(player.body.velocity.y == -50.0f);
        CHECK(player.jumpCharge      == 0.75f);
        CHECK(player.isCharging      == true);
        CHECK(world.accumulator()    == originalAcc);
    }

    TEST_CASE("Continuous Recording e Rewind Frame-by-Frame") {
        PhysicsWorld world;
        Player player;
        InputManager input;
        ReplayManager replay;

        player.body.isGrounded = true; // NOVA FÍSICA: Se não estiver no chão, não se pode mover no X!

        input.injectRawState(false, true, false, false, false); // D / RIGHT ativo
        
        replay.recordFrame(player, world, input);
        player.update(input, world, PhysicsWorld::FIXED_STEP);
        Vec2 posFrame1 = player.position();

        replay.recordFrame(player, world, input);
        player.update(input, world, PhysicsWorld::FIXED_STEP);
        Vec2 posFrame2 = player.position();

        replay.recordFrame(player, world, input);
        player.update(input, world, PhysicsWorld::FIXED_STEP);

        CHECK(replay.getReplayLength() == 3);

        bool rew1 = replay.rewind(player, world);
        CHECK(rew1 == true);
        CHECK(player.position() == posFrame2);

        bool rew2 = replay.rewind(player, world);
        CHECK(rew2 == true);
        CHECK(player.position() == posFrame1);
    }

    TEST_CASE("Determinismo de Replay — Reprodução Exata de Sequência") {
        PhysicsWorld worldRecording;
        Player playerRecording;
        InputManager inputRecording;
        ReplayManager replay;

        playerRecording.body.isGrounded = true; // NOVA FÍSICA: Chão!

        inputRecording.injectRawState(true, false, true, true, false);
        replay.recordFrame(playerRecording, worldRecording, inputRecording);
        playerRecording.update(inputRecording, worldRecording, PhysicsWorld::FIXED_STEP);

        inputRecording.injectRawState(true, false, true, false, false);
        replay.recordFrame(playerRecording, worldRecording, inputRecording);
        playerRecording.update(inputRecording, worldRecording, PhysicsWorld::FIXED_STEP);

        inputRecording.injectRawState(true, false, false, false, true);
        replay.recordFrame(playerRecording, worldRecording, inputRecording);
        playerRecording.update(inputRecording, worldRecording, PhysicsWorld::FIXED_STEP);

        Vec2 finalRecordingPos = playerRecording.position();
        Vec2 finalRecordingVel = playerRecording.velocity();

        PhysicsWorld worldPlayback;
        Player playerPlayback;
        InputManager inputPlayback;
        
        playerPlayback.body.isGrounded = true; // NOVA FÍSICA: Chão!

        replay.startPlayback();
        REQUIRE(replay.isPlaybackComplete() == false);

        while (!replay.isPlaybackComplete()) {
            replay.preparePlaybackFrame(inputPlayback);
            playerPlayback.update(inputPlayback, worldPlayback, PhysicsWorld::FIXED_STEP);
        }

        CHECK(playerPlayback.position().x == finalRecordingPos.x);
        CHECK(playerPlayback.position().y == finalRecordingPos.y);
        CHECK(playerPlayback.velocity().x == finalRecordingVel.x);
        CHECK(playerPlayback.velocity().y == finalRecordingVel.y);
    }
}