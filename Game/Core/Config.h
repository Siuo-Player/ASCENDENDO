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

    // O editor faz snap aos 4 px; este valor pertence ao contrato de edição,
    // não à apresentação visual da grelha.
    constexpr float EDITOR_GRID_SNAP            = 4.0f;

    // Mantido para compatibilidade de configuração; o Level Editor de uma
    // única tela não usa pan. O Campaign Editor será o primeiro estado a
    // permitir deslocação vertical.
    constexpr float EDITOR_CAMERA_PAN_SPEED     = 400.0f;
}
