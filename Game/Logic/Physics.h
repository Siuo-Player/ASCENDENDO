#pragma once
// =============================================================================
//  Game/Logic/Physics.h
//
//  @version 3.3
//  @history
//    v3.1 — criado (Vec2, AABB, PhysicsBody, PhysicsWorld + Fixed Timestep)
//    v3.3 — setAccumulator() exposto para suporte a Save States determinísticos
// =============================================================================

#include <cstdint>

namespace logic {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2 operator+(Vec2 o)  const { return {x + o.x, y + o.y}; }
    Vec2 operator-(Vec2 o)  const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s,   y * s  }; }
    Vec2& operator+=(Vec2 o)      { x += o.x; y += o.y; return *this; }
    bool  operator==(Vec2 o) const { return x == o.x && y == o.y; }
};

struct AABB {
    Vec2 min{};
    Vec2 max{};

    bool overlaps(const AABB& other) const;
    Vec2 center() const { return {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f}; }
    float width()  const { return max.x - min.x; }
    float height() const { return max.y - min.y; }
};

struct PhysicsBody {
    Vec2  position   = {};
    Vec2  velocity   = {};
    float width      = 32.0f;
    float height     = 64.0f;
    bool  isGrounded = false;

    AABB bounds() const {
        return { position, {position.x + width, position.y + height} };
    }
};

class PhysicsWorld {
public:
    static constexpr float GRAVITY    = -980.0f;
    static constexpr float FIXED_STEP = 1.0f / 60.0f;
    static constexpr float GROUND_Y   = 0.0f;

    void step(PhysicsBody& body, float dt) const;
    void jump(PhysicsBody& body, float force) const;
    static bool collides(const AABB& a, const AABB& b);
    int advance(float deltaTime);

    float accumulator() const { return m_accumulator; }
    void setAccumulator(float value) { m_accumulator = value; }

private:
    float m_accumulator = 0.0f;
};

} // namespace logic