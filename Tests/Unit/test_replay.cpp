// =============================================================================
//  Tests/Unit/test_replay.cpp
//
//  @version 3.3
//  @history
//    v3.3 — criado (testes unitários para o ReplayManager e Save States)
// =============================================================================

#include "doctest/doctest.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Logic/InputManager.h"
#include "Logic/ReplayManager.h"

using namespace logic;

TEST_SUITE("Fase 3.3 — Save States & Replay System") {

    TEST_CASE("Save State Estático — Salvar e Carregar Slots") {
        PhysicsWorld world;
        Player player;
        ReplayManager replay;

        // Modificar o estado inicial do jogador e do mundo
        player.body.position = { 150.0f, 80.0f };
        player.body.velocity = { 200.0f, -50.0f };
        player.jumpCharge    = 0.75f;
        player.isCharging    = true;
        world.advance(0.025f);

        float originalAcc = world.accumulator();

        // Guardar no slot 1
        replay.saveState(1, player, world);

        // Corromper o estado para simular perda de foco ou morte do jogador
        player.body.position = { 999.0f, 999.0f };
        player.body.velocity = { 0.0f, 0.0f };
        player.jumpCharge    = 0.0f;
        player.isCharging    = false;
        world.advance(0.1f);

        // Restaurar o slot 1
        bool success = replay.loadState(1, player, world);
        REQUIRE(success == true);

        // Verificar integridade absoluta da simulação restaurada
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

        // Executar e gravar 3 frames de movimento para a direita
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

        // Executar Rewind de 1 frame (deve voltar exatamente ao estado após o frame 2)
        bool rew1 = replay.rewind(player, world);
        CHECK(rew1 == true);
        CHECK(player.position() == posFrame2);

        // Executar Rewind de mais 1 frame (deve voltar para o estado após o frame 1)
        bool rew2 = replay.rewind(player, world);
        CHECK(rew2 == true);
        CHECK(player.position() == posFrame1);
    }

    TEST_CASE("Determinismo de Replay — Reprodução Exata de Sequência") {
        PhysicsWorld worldRecording;
        Player playerRecording;
        InputManager inputRecording;
        ReplayManager replay;

        // --- FASE DE GRAVAÇÃO ---
        // Frame 1: Mover para a esquerda e começar a carregar o salto
        inputRecording.injectRawState(true, false, true, true, false);
        replay.recordFrame(playerRecording, worldRecording, inputRecording);
        playerRecording.update(inputRecording, worldRecording, PhysicsWorld::FIXED_STEP);

        // Frame 2: Continuar a carregar o salto
        inputRecording.injectRawState(true, false, true, false, false);
        replay.recordFrame(playerRecording, worldRecording, inputRecording);
        playerRecording.update(inputRecording, worldRecording, PhysicsWorld::FIXED_STEP);

        // Frame 3: Soltar o botão SPACE para despoletar o salto interpolado
        inputRecording.injectRawState(true, false, false, false, true);
        replay.recordFrame(playerRecording, worldRecording, inputRecording);
        playerRecording.update(inputRecording, worldRecording, PhysicsWorld::FIXED_STEP);

        Vec2 finalRecordingPos = playerRecording.position();
        Vec2 finalRecordingVel = playerRecording.velocity();

        // --- FASE DE REPRODUÇÃO (PLAYBACK) ---
        PhysicsWorld worldPlayback;
        Player playerPlayback;
        InputManager inputPlayback;

        replay.startPlayback();
        REQUIRE(replay.isPlaybackComplete() == false);

        while (!replay.isPlaybackComplete()) {
            replay.preparePlaybackFrame(inputPlayback);
            playerPlayback.update(inputPlayback, worldPlayback, PhysicsWorld::FIXED_STEP);
        }

        // Validar determinismo absoluto: os estados finais têm de coincidir de forma idêntica
        CHECK(playerPlayback.position().x == finalRecordingPos.x);
        CHECK(playerPlayback.position().y == finalRecordingPos.y);
        CHECK(playerPlayback.velocity().x == finalRecordingVel.x);
        CHECK(playerPlayback.velocity().y == finalRecordingVel.y);
    }
}