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

    // Preload the next campaign screen before the player reaches the boundary.
    // Half a logical screen keeps the next screen ready while preserving the
    // intended upward progression and avoiding work on the initial spawn frame.
    constexpr float CAMPAIGN_STREAM_PRELOAD_DISTANCE = LOGICAL_HEIGHT * 0.5f;

    // Câmara de autoria: permite navegar verticalmente num Level que tenha N telas.
    constexpr float EDITOR_CAMERA_PAN_SPEED = 400.0f;
}