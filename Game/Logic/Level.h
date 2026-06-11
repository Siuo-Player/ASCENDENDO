#pragma once
// =============================================================================
//  Game/Logic/Level.h
//
//  @version 6.2c
//  @history
//    v6.2  — criado (colisao de topo one-way com SKIN_WIDTH)
//    v6.2c — colisao lateral adicionada; resolucao full-AABB via MTV
//
//  CONVENÇÃO DE COORDENADAS (Y para cima):
//    addPlatform(x, y, w, h)  →  canto inferior-esquerdo em (x, y).
//    resolveCollision()        →  top landing + side blocking + one-way up.
//    Deve ser chamada SEMPRE DEPOIS de PhysicsWorld::step().
// =============================================================================

#include "Logic/Physics.h"
#include <vector>

namespace logic {

struct Platform {
    AABB bounds;
};

class Level {
public:
    void addPlatform(float x, float y, float w, float h);

    // Resolve todas as colisoes (topo one-way + lados sólidos).
    // Retorna true se houve pelo menos uma colisao.
    bool resolveCollision(PhysicsBody& body) const;

    const std::vector<Platform>& platforms()    const { return m_platforms; }
    int                          platformCount() const { return (int)m_platforms.size(); }
    void                         clear()               { m_platforms.clear(); }

private:
    static constexpr float SKIN_WIDTH = 1.0f;
    std::vector<Platform> m_platforms;
};

} // namespace logic
