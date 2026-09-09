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

    // A resolução lógica da simulação é independente da quantização usada pela UI.
    // Não apresentar uma escala inteira fictícia como precisão física ao jogador.
    constexpr float EDITOR_GRID_SNAP    = 4.0f;

    // Câmara de autoria: permite navegar verticalmente num Level que tenha N telas.
    constexpr float EDITOR_CAMERA_PAN_SPEED = 400.0f;
}
