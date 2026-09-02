#pragma once
// =============================================================================
// Game/Graphics/RenderSnapshot.h
//
// Dados imutáveis necessários para apresentação de um frame.
// Não contém Player, Level, GameState ou outros tipos de domínio.
// =============================================================================

#include "Graphics/PlatformCompositor.h"

#include <cstdint>
#include <vector>

namespace gfx {

struct RenderRect {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct RenderPlayerSnapshot {
    RenderRect bounds{};
    float facingDirection = 1.0f;
};

struct RenderFlagSnapshot {
    bool visible = false;
    RenderRect bounds{};
};

struct RenderSnapshot {
    RenderPlayerSnapshot player{};
    std::vector<RenderRect> platforms;

    // Presentation-only semantic cells generated from modular platform regions.
    // The legacy platform rectangles remain available so a non-modular level is
    // never silently rewritten merely to satisfy the compositor boundary.
    bool semanticPlatformsValid = false;
    std::vector<compositor::RegionCell> semanticPlatformCells;

    RenderFlagSnapshot flag{};
};

} // namespace gfx
