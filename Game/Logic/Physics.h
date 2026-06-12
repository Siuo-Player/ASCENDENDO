#pragma once
// =============================================================================
//  Game/Logic/Physics.h
//
//  @version 7.1
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
    
    // O tamanho do corpo agora é consumido diretamente da Configuração Global
    float width = config::PLAYER_WIDTH;
    float height = config::PLAYER_HEIGHT;
    
    bool isGrounded = false;

    AABB bounds() const {
        return { position, {position.x + width, position.y + height} };
    }
};

class PhysicsWorld {
public:
    // Mantém compatibilidade com os testes existentes
    static constexpr float FIXED_STEP = config::FIXED_STEP;
    static constexpr float GRAVITY    = config::GRAVITY;

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