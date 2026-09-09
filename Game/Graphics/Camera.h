#pragma once
// =============================================================================
//  Game/Graphics/Camera.h
//
//  @version 6.5
//  @history
//    v4.1  — worldToNDC (world space → NDC Vulkan, Y invertido)
//    v6.4  — follow(): tracking vertical suave do jogador (Lerp)
//    v6.5  — follow(): inputs nao-finitos sao ignorados (fail-closed)
// =============================================================================
#include "Logic/Physics.h"

namespace gfx {

class Camera {
public:
    logic::Vec2 position = {0.0f, 0.0f};

    // Converte posicao world para NDC [-1,1] do Vulkan.
    logic::Vec2 worldToNDC(const logic::Vec2& worldPos) const;

    // Segue suavemente o alvo em Y (tracking vertical).
    // O jogador fica a ~35% da base do ecra para mostrar mais plataformas acima.
    // speed: fator de interpolacao (5.0 = rapido/responsivo).
    // Camera Y nunca fica abaixo de 0 (chao absoluto do mundo).
    // Inputs nao-finitos (target.y, dt ou speed) sao ignorados para evitar
    // contaminar o estado com NaN/Inf.
    void follow(const logic::Vec2& target, float dt, float speed = 5.0f);
};

} // namespace gfx
