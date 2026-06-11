#pragma once
// =============================================================================
//  Game/Logic/Level.h
//
//  @version 6.2
//  @history
//    v6.2 — criado. Lista de plataformas (AABBs) com resolucao de colisao
//            de topo. Sem dependencias graficas — puro logic layer.
//
//  CONVENÇÃO DE COORDENADAS (Y para cima):
//    addPlatform(x, y, w, h)  →  canto inferior-esquerdo em (x, y).
//    resolveCollision()        →  pousa o corpo no max.y da plataforma.
//    Deve ser chamada SEMPRE DEPOIS de PhysicsWorld::step().
// =============================================================================

#include "Logic/Physics.h"
#include <vector>

namespace logic {

// Bloco geometrico: so os limites de colisao.
// Cor e aparencia ficam a cargo do Renderer (Fase 6.2b).
struct Platform {
    AABB bounds;
};

class Level {
public:
    // Adiciona uma plataforma com canto inferior-esquerdo em (x,y) e dimensoes (w,h).
    void addPlatform(float x, float y, float w, float h);

    // Resolve colisao do corpo com as plataformas deste nivel.
    // Regra: so pousa no TOPO de uma plataforma (velocity.y <= 0).
    // Ignora colisoes laterais e de baixo (one-way platforms).
    // Retorna true se houve colisao com pelo menos uma plataforma.
    // Deve ser chamado APOS PhysicsWorld::step() em cada passo de fisica.
    bool resolveCollision(PhysicsBody& body) const;

    const std::vector<Platform>& platforms()  const { return m_platforms; }
    int                          platformCount() const { return (int)m_platforms.size(); }
    void                         clear()              { m_platforms.clear(); }

private:
    // 1px de tolerancia para detetar corpo "em cima" sem sobreposicao real.
    // Necessario porque step() define isGrounded=false quando pos.y > GROUND_Y.
    static constexpr float SKIN_WIDTH = 1.0f;

    std::vector<Platform> m_platforms;
};

} // namespace logic
