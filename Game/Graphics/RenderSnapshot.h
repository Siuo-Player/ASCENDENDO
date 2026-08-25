#pragma once
// =============================================================================
// Game/Graphics/RenderSnapshot.h
//
// Dados imutáveis necessários para apresentação de um frame.
// Não contém Player, Level, GameState ou outros tipos de domínio.
// =============================================================================

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
    RenderFlagSnapshot flag{};
};

} // namespace gfx
