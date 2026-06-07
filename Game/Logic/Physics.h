#pragma once
// =============================================================================
//  Game/Logic/Physics.h
//
//  @version 3.1
//  @history
//    v3.1 — criado (Vec2, AABB, PhysicsBody, PhysicsWorld + Fixed Timestep)
//
//  Física determinística: resultados idênticos em qualquer hardware.
//  Base para save states e replays exatos (ver README, Secção 1).
//
//  Fixed Timestep: a física corre sempre a 60 Hz, independente da taxa
//  de renderização. O render interpola posições entre passos de física.
// =============================================================================

#include <cstdint>

namespace logic {

// ── Tipos de Dados ────────────────────────────────────────────────────────────

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2 operator+(Vec2 o)  const { return {x + o.x, y + o.y}; }
    Vec2 operator-(Vec2 o)  const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s,   y * s  }; }
    Vec2& operator+=(Vec2 o)      { x += o.x; y += o.y; return *this; }
    bool  operator==(Vec2 o) const { return x == o.x && y == o.y; }
};

// Caixa de colisão alinhada aos eixos (Axis-Aligned Bounding Box)
struct AABB {
    Vec2 min{};
    Vec2 max{};

    bool overlaps(const AABB& other) const;
    Vec2 center() const { return {(min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f}; }
    float width()  const { return max.x - min.x; }
    float height() const { return max.y - min.y; }
};

// ── Corpo Físico ──────────────────────────────────────────────────────────────

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

// ── Motor de Física ───────────────────────────────────────────────────────────

class PhysicsWorld {
public:
    // Constantes da física do jogo
    static constexpr float GRAVITY    = -980.0f;       // px/s² (gravidade para baixo)
    static constexpr float FIXED_STEP = 1.0f / 60.0f; // passo fixo: 60 Hz
    static constexpr float GROUND_Y   = 0.0f;          // Y do chão (colisão simples)

    // ── Passo de Física ───────────────────────────────────────────────────────
    // Aplicar um tick de fisica (FIXED_STEP segundos) ao corpo.
    // Inclui: gravidade, integração de Euler, colisão com o chão.
    void step(PhysicsBody& body, float dt) const;

    // ── Salto ─────────────────────────────────────────────────────────────────
    // Aplica força vertical ao corpo se estiver no chão.
    // force > 0 = para cima. Valores típicos: 400–800 px/s.
    void jump(PhysicsBody& body, float force) const;

    // ── Colisão AABB ──────────────────────────────────────────────────────────
    static bool collides(const AABB& a, const AABB& b);

    // ── Fixed Timestep Accumulator ────────────────────────────────────────────
    // Chamar com o deltaTime do frame; retorna quantos passos de física executar.
    // Uso:
    //   int steps = world.advance(deltaTime);
    //   for (int i = 0; i < steps; ++i) world.step(player, FIXED_STEP);
    int advance(float deltaTime);

    float accumulator() const { return m_accumulator; }

private:
    float m_accumulator = 0.0f;
};

} // namespace logic
