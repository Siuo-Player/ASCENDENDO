// =============================================================================
//  Game/Logic/Level.cpp
//
//  @version 6.9
// =============================================================================

#include "Logic/Level.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iostream>

namespace logic {

bool Level::loadFromFile(const std::string& filepath, float maxWidth) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[ERRO] Nao foi possivel abrir o nivel: " << filepath << "\n";
        return false;
    }

    clear(); 
    name = "Sem Nome";
    hasFlag = false;
    
    std::string line;
    int lineNum = 0;

    while (std::getline(file, line)) {
        lineNum++;
        if (line.empty() || line[0] == '#') continue; 

        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "NAME") {
            std::getline(iss >> std::ws, name);
        } else if (type == "PLATFORM") {
            float x, y, w, h;
            if (iss >> x >> y >> w >> h) {
                if (x < 0.0f || (x + w) > maxWidth) {
                    std::cerr << "[AVISO] Nivel '" << name << "', Linha " << lineNum 
                              << ": Plataforma fora dos limites laterais! X=" << x << "\n";
                }
                addPlatform(x, y, w, h);
            }
        } else if (type == "FLAG") {
            float x, y, w, h;
            if (iss >> x >> y >> w >> h) {
                hasFlag = true;
                flagBounds = AABB{{x, y}, {x + w, y + h}};
            }
        }
    }
    
    std::cout << "[ASCENDENDO] Nivel '" << name << "' carregado. " 
              << platformCount() << " plataformas.\n";
    return true;
}

void Level::addPlatform(float x, float y, float w, float h) {
    m_platforms.push_back({ AABB{ {x, y}, {x + w, y + h} } });
}

bool Level::resolveCollision(PhysicsBody& body) const {
    bool collided = false;

    for (const auto& platform : m_platforms) {
        AABB bodyAABB = body.bounds();
        AABB probe = bodyAABB;
        probe.min.y -= SKIN_WIDTH;

        if (!probe.overlaps(platform.bounds)) continue;

        if (!bodyAABB.overlaps(platform.bounds)) {
            if (body.velocity.y <= 0.0f) {
                body.position.y = platform.bounds.max.y;
                body.velocity.y = 0.0f;
                body.isGrounded  = true;
                collided = true;
            }
            continue; 
        }

        float exitLeft  = bodyAABB.max.x  - platform.bounds.min.x;
        float exitRight = platform.bounds.max.x - bodyAABB.min.x;
        float exitUp    = platform.bounds.max.y - bodyAABB.min.y;
        float exitDown  = bodyAABB.max.y  - platform.bounds.min.y;

        float minExitX = std::min(exitLeft,  exitRight);
        float minExitY = std::min(exitUp,    exitDown);

        bool lateralCollision = std::abs(body.velocity.x) > std::abs(body.velocity.y) && minExitX <= minExitY;

        if (lateralCollision) {
            body.position.x = (exitLeft < exitRight) ? platform.bounds.min.x - body.width : platform.bounds.max.x;                
            body.velocity.x = -body.velocity.x * 0.3f;
            collided = true;
        } else {
            if (exitUp <= exitDown) {
                if (body.velocity.y <= 0.0f) {          
                    body.position.y = platform.bounds.max.y;
                    body.velocity.y = 0.0f;
                    body.isGrounded  = true;
                    collided = true;
                }
            } else {
                if (body.velocity.y > 0.0f) {           
                    body.position.y = platform.bounds.min.y - body.height;
                    body.velocity.y = -body.velocity.y * 0.3f;
                    body.velocity.x = body.velocity.x * 0.9f; 
                    collided = true;
                }
            }
        }
    }
    return collided;
}

} // namespace logic