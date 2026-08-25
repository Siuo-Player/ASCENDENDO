#pragma once
// =============================================================================
//  Game/Core/Config.h
// =============================================================================

namespace config {
    constexpr float LOGICAL_WIDTH       = 640.0f;
    constexpr float LOGICAL_HEIGHT      = 360.0f;
    constexpr float TARGET_ASPECT       = LOGICAL_WIDTH / LOGICAL_HEIGHT;

    constexpr float FIXED_STEP          = 1.0f / 60.0f;
    constexpr float GRAVITY             = -980.0f;

    constexpr float PLAYER_WIDTH        = 16.0f;
    constexpr float PLAYER_HEIGHT       = 16.0f;
    constexpr float PLAYER_MOVE_SPEED   = 140.0f;
    constexpr float PLAYER_MIN_JUMP     = 250.0f;
    constexpr float PLAYER_MAX_JUMP     = 600.0f;
    constexpr float PLAYER_CHARGE_TIME  = 0.4f;

    constexpr float CAMERA_SPEED        = 5.0f;
    constexpr float CAMERA_OFFSET_Y     = LOGICAL_HEIGHT * 0.35f;

    constexpr float COLOR_PLATFORM_R    = 0.35f;
    constexpr float COLOR_PLATFORM_G    = 0.65f;
    constexpr float COLOR_PLATFORM_B    = 0.25f;

    constexpr float COLOR_PLAYER_R      = 0.90f;
    constexpr float COLOR_PLAYER_G      = 0.45f;
    constexpr float COLOR_PLAYER_B      = 0.20f;

    constexpr float COLOR_FLAG_POLE_R   = 0.85f;
    constexpr float COLOR_FLAG_POLE_G   = 0.65f;
    constexpr float COLOR_FLAG_POLE_B   = 0.10f;
    constexpr float COLOR_FLAG_R        = 1.00f;
    constexpr float COLOR_FLAG_G        = 0.90f;
    constexpr float COLOR_FLAG_B        = 0.10f;

    constexpr float CLEAR_CREDITS_R     = 0.02f;
    constexpr float CLEAR_CREDITS_G     = 0.02f;
    constexpr float CLEAR_CREDITS_B     = 0.12f;

    constexpr float CLEAR_MENU_R        = 0.03f;
    constexpr float CLEAR_MENU_G        = 0.03f;
    constexpr float CLEAR_MENU_B        = 0.06f;

    // O editor faz snap aos 4 px, mas mostra uma malha de 16 px.
    // 16 px corresponde ao tamanho lógico do jogador e torna os blocos
    // MEDIUM/LARGE do editor visualmente legíveis sem centenas de linhas.
    constexpr float EDITOR_GRID_SNAP            = 4.0f;
    constexpr float EDITOR_GRID_VISUAL_SPACING  = 16.0f;
    constexpr float EDITOR_GRID_MAJOR_SPACING    = 64.0f;

    // Mantido para compatibilidade de configuração; o Level Editor de uma
    // única tela não usa pan. O Campaign Editor será o primeiro estado a
    // permitir deslocação vertical.
    constexpr float EDITOR_CAMERA_PAN_SPEED     = 400.0f;

    constexpr float CLEAR_EDITOR_R              = 0.10f;
    constexpr float CLEAR_EDITOR_G              = 0.10f;
    constexpr float CLEAR_EDITOR_B              = 0.11f;
}
