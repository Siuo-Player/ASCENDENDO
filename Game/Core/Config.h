#pragma once
// =============================================================================
//  Game/Core/Config.h
//
//  @version 5.2
//  Todas as variaveis globais e magic numbers do motor concentrados aqui.
// =============================================================================

namespace config {

    // ── Resolução Lógica (Pixel Art) ──────────────────────────────────────────
    // Formato Paisagem (Landscape) 16:9
    constexpr float LOGICAL_WIDTH  = 640.0f;
    constexpr float LOGICAL_HEIGHT = 360.0f;
    constexpr float TARGET_ASPECT  = LOGICAL_WIDTH / LOGICAL_HEIGHT;

    // ── Física e Simulação ────────────────────────────────────────────────────
    constexpr float FIXED_STEP     = 1.0f / 60.0f;
    constexpr float GRAVITY        = -980.0f;
    constexpr float GROUND_Y       = 0.0f;

} // namespace config