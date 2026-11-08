// =============================================================================
//  Game/Logic/JumpTrajectory.cpp
// =============================================================================

#include "Logic/JumpTrajectory.h"

#include <algorithm>
#include <cmath>

namespace logic {

float JumpTrajectory::initialSpeed(float chargeRatio) noexcept {
    const float clampedCharge = std::clamp(chargeRatio, 0.0f, 1.0f);
    return config::PLAYER_MIN_JUMP +
        (config::PLAYER_MAX_JUMP - config::PLAYER_MIN_JUMP) * clampedCharge;
}

Vec2 JumpTrajectory::initialVelocity(float chargeRatio, float facingDirection) noexcept {
    const float speed = initialSpeed(chargeRatio);
    const float dir = (facingDirection < 0.0f) ? -1.0f : 1.0f;
    return {
        speed * COS60 * dir,
        speed * SIN60
    };
}

Vec2 JumpTrajectory::positionAt(const Vec2& origin, const Vec2& initialVel, float t) noexcept {
    if (t <= 0.0f) return origin;
    return {
        origin.x + initialVel.x * t,
        origin.y + initialVel.y * t + 0.5f * config::GRAVITY * t * t
    };
}

Vec2 JumpTrajectory::velocityAt(const Vec2& initialVel, float t) noexcept {
    if (t <= 0.0f) return initialVel;
    return {
        initialVel.x,
        initialVel.y + config::GRAVITY * t
    };
}

float JumpTrajectory::timeToApex(float initialVy) noexcept {
    if (initialVy <= 0.0f) return 0.0f;
    return -initialVy / config::GRAVITY; // config::GRAVITY is negative (-980)
}

float JumpTrajectory::apexHeight(float initialVy) noexcept {
    if (initialVy <= 0.0f) return 0.0f;
    const float tApex = timeToApex(initialVy);
    return initialVy * tApex + 0.5f * config::GRAVITY * tApex * tApex;
}

TrajectoryEnvelope JumpTrajectory::computeEnvelope(const Vec2& origin, float facingDirection) noexcept {
    const auto minVel = initialVelocity(0.0f, facingDirection);
    const auto maxVel = initialVelocity(1.0f, facingDirection);

    const float minApex = apexHeight(minVel.y);
    const float maxApex = apexHeight(maxVel.y);

    const float tFlightMin = 2.0f * timeToApex(minVel.y);
    const float tFlightMax = 2.0f * timeToApex(maxVel.y);

    const float minReach = origin.x + minVel.x * tFlightMin;
    const float maxReach = origin.x + maxVel.x * tFlightMax;

    TrajectoryEnvelope envelope;
    envelope.minApexHeight = minApex;
    envelope.maxApexHeight = maxApex;
    envelope.minReachX = std::min(minReach, maxReach);
    envelope.maxReachX = std::max(minReach, maxReach);
    return envelope;
}

std::vector<TrajectoryPoint> JumpTrajectory::sampleTrajectory(
    const Vec2& origin,
    float chargeRatio,
    float facingDirection,
    float totalDuration,
    float dt) {
    std::vector<TrajectoryPoint> points;
    if (dt <= 0.0f || totalDuration <= 0.0f) return points;

    const auto v0 = initialVelocity(chargeRatio, facingDirection);
    const std::size_t numSamples = static_cast<std::size_t>(totalDuration / dt) + 1;
    points.reserve(numSamples);

    for (float t = 0.0f; t <= totalDuration + 1e-5f; t += dt) {
        points.push_back({
            t,
            positionAt(origin, v0, t),
            velocityAt(v0, t)
        });
    }

    return points;
}

} // namespace logic
