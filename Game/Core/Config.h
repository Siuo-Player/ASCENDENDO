#pragma once
// =============================================================================
//  Game/Core/Config.h
//
//  @version 7.2
// =============================================================================

namespace config {
    // ─── Janela e Resolução Lógica ───────────────────────────────────────────
    constexpr float LOGICAL_WIDTH       = 640.0f; 
    constexpr float LOGICAL_HEIGHT      = 360.0f;
    constexpr float TARGET_ASPECT       = LOGICAL_WIDTH / LOGICAL_HEIGHT;

    // ─── Física Global ───────────────────────────────────────────────────────
    constexpr float FIXED_STEP          = 1.0f / 60.0f;
    constexpr float GRAVITY             = -980.0f;

    // ─── Configurações do Jogador (Escala Reduzida) ──────────────────────────
    constexpr float PLAYER_WIDTH        = 16.0f;
    constexpr float PLAYER_HEIGHT       = 16.0f;
    constexpr float PLAYER_MOVE_SPEED   = 140.0f; 
    constexpr float PLAYER_MIN_JUMP     = 250.0f; 
    constexpr float PLAYER_MAX_JUMP     = 600.0f; 
    constexpr float PLAYER_CHARGE_TIME  = 0.4f;

    // ─── Sistema de Câmara e Tracking ────────────────────────────────────────
    constexpr float CAMERA_SPEED        = 5.0f;
    constexpr float CAMERA_OFFSET_Y     = LOGICAL_HEIGHT * 0.35f; // 35% inferior

    // ─── Parâmetros Estéticos e Cores (Vulkan) ───────────────────────────────
    constexpr float COLOR_PLATFORM_R    = 0.35f;
    constexpr float COLOR_PLATFORM_G    = 0.65f;
    constexpr float COLOR_PLATFORM_B    = 0.25f;

    constexpr float COLOR_PLAYER_R      = 0.90f;
    constexpr float COLOR_PLAYER_G      = 0.45f;
    constexpr float COLOR_PLAYER_B      = 0.20f;

    // ─── Bandeira / FLAG ─────────────────────────────────────────────────────
    constexpr float COLOR_FLAG_POLE_R   = 0.85f;   // mastro: dourado escuro
    constexpr float COLOR_FLAG_POLE_G   = 0.65f;
    constexpr float COLOR_FLAG_POLE_B   = 0.10f;
    constexpr float COLOR_FLAG_R        = 1.00f;   // pano: dourado brilhante
    constexpr float COLOR_FLAG_G        = 0.90f;
    constexpr float COLOR_FLAG_B        = 0.10f;

    // ─── Ecrã de Créditos ────────────────────────────────────────────────────
    constexpr float CLEAR_CREDITS_R     = 0.02f;
    constexpr float CLEAR_CREDITS_G     = 0.02f;
    constexpr float CLEAR_CREDITS_B     = 0.12f;

    // ─── Menu Principal ──────────────────────────────────────────────────────
    constexpr float CLEAR_MENU_R        = 0.03f;
    constexpr float CLEAR_MENU_G        = 0.03f;
    constexpr float CLEAR_MENU_B        = 0.06f;
}