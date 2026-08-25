#pragma once
// =============================================================================
//  Game/Logic/Physics.h
// =============================================================================

#include "Core/Config.h"

namespace logic {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
};

struct AABB {
    Vec2 min;
    Vec2 max;

    bool overlaps(const AABB& o) const {
        return max.x > o.min.x && min.x < o.max.x &&
               max.y > o.min.y && min.y < o.max.y;
    }

    float width() const { return max.x - min.x; }
    float height() const { return max.y - min.y; }
};

struct PhysicsBody {
    Vec2 position;
    Vec2 velocity;
    float width = config::PLAYER_WIDTH;
    float height = config::PLAYER_HEIGHT;
    bool isGrounded = false;

    AABB bounds() const {
        return { position, {position.x + width, position.y + height} };
    }
};

class PhysicsWorld {
public:
    static constexpr float FIXED_STEP = config::FIXED_STEP;
    static constexpr float GRAVITY = config::GRAVITY;

    // Defensive limit: a long hitch/minimize must not create an unbounded
    // catch-up storm. 15 steps ~= 250 ms at 60 Hz.
    static constexpr int MAX_STEPS_PER_ADVANCE = 15;
    static constexpr float MAX_FRAME_TIME = FIXED_STEP * MAX_STEPS_PER_ADVANCE;

    int advance(float dt);
    void step(PhysicsBody& body, float dt);
    void jump(PhysicsBody& body, float force);

    static bool collides(const AABB& a, const AABB& b) {
        return a.overlaps(b);
    }

    float accumulator() const { return m_accumulator; }
    void setAccumulator(float val) { m_accumulator = val; }

private:
    float m_accumulator = 0.0f;
};

} // namespace logic
