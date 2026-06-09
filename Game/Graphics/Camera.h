// =============================================================================
//  Game/Graphics/Camera.h
//
//  @version 4.1
//  @history
//    v4.1 — criado (Viewport lógico e conversão para NDC do Vulkan)
// =============================================================================

#pragma once
#include "Logic/Physics.h" // Reaproveitamos o Vec2

namespace gfx {

class Camera {
public:
    static constexpr float LOGICAL_WIDTH  = 360.0f;
    static constexpr float LOGICAL_HEIGHT = 640.0f;

    // Posição da câmara no mundo (representa o canto inferior esquerdo do ecrã)
    logic::Vec2 position = {0.0f, 0.0f};

    // Converte uma coordenada do mundo (física) para Normalized Device Coordinates
    logic::Vec2 worldToNDC(const logic::Vec2& worldPos) const;
};

} // namespace gfx