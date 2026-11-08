#pragma once
// =============================================================================
//  Game/Logic/JumpTrajectory.h
//
//  Cálculo analítico da parábola balística do Commitment Jump a 60°.
//  Fundamentação teórica:
//    - Aramini, Lanzi & Loiacono (2018): AI Assisted Level Design in 2D Platformers
//    - Smith, Cha & Whitehead (2008): Framework for Analysis of 2D Platformers
// =============================================================================

#include "Core/Config.h"
#include "Logic/Physics.h"

#include <vector>

namespace logic {

struct TrajectoryPoint {
    float time = 0.0f;
    Vec2 position{0.0f, 0.0f};
    Vec2 velocity{0.0f, 0.0f};
};

struct TrajectoryEnvelope {
    float minReachX = 0.0f;
    float maxReachX = 0.0f;
    float minApexHeight = 0.0f;
    float maxApexHeight = 0.0f;
};

class JumpTrajectory {
public:
    static constexpr float COS60 = 0.5f;
    static constexpr float SIN60 = 0.8660254037844386f;

    // Calcula a velocidade inicial para uma dada fração de carga [0.0f, 1.0f]
    static float initialSpeed(float chargeRatio) noexcept;

    // Componentes de velocidade inicial (vx, vy)
    static Vec2 initialVelocity(float chargeRatio, float facingDirection) noexcept;

    // Posição contínua no instante t (em segundos)
    static Vec2 positionAt(const Vec2& origin, const Vec2& initialVel, float t) noexcept;

    // Velocidade no instante t
    static Vec2 velocityAt(const Vec2& initialVel, float t) noexcept;

    // Tempo até atingir o ápice do salto
    static float timeToApex(float initialVy) noexcept;

    // Altura máxima (delta Y) atingida acima da origem
    static float apexHeight(float initialVy) noexcept;

    // Envelope analítico de alcance (mínimo e máximo) a partir de uma origem
    static TrajectoryEnvelope computeEnvelope(const Vec2& origin, float facingDirection) noexcept;

    // Amostragem discreta da parábola para visualização ou diagnósticos
    static std::vector<TrajectoryPoint> sampleTrajectory(
        const Vec2& origin,
        float chargeRatio,
        float facingDirection,
        float totalDuration = 0.8f,
        float dt = config::FIXED_STEP);
};

} // namespace logic
