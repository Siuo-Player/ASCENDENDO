#pragma once
// =============================================================================
//  Game/Logic/Level.h
//
//  @version 6.9
// =============================================================================

#include "Logic/Physics.h"
#include <vector>
#include <string>

namespace logic {

struct Platform {
    AABB bounds;
};

class Level {
public:
    std::string name = "Sem Nome";
    
    // Suporte para o objetivo final (Bandeira)
    bool hasFlag = false;
    AABB flagBounds;

    bool loadFromFile(const std::string& filepath, float maxWidth);
    void addPlatform(float x, float y, float w, float h);
    bool resolveCollision(PhysicsBody& body) const;

    const std::vector<Platform>& platforms()     const { return m_platforms; }
    int                          platformCount() const { return (int)m_platforms.size(); }
    void                         clear()               { m_platforms.clear(); }

private:
    static constexpr float SKIN_WIDTH = 1.0f;
    std::vector<Platform> m_platforms;
};

} // namespace logic