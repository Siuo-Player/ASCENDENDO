#include "Logic/Level.h"
#include "Core/Config.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace logic {

float Level::appendFromData(const LevelData& data, float maxWidth, float offsetY) {
    hasFlag = false;
    name = data.name;

    float highestY = offsetY;
    for (const auto& localBounds : data.platforms) {
        if (localBounds.min.x < 0.0f || localBounds.max.x > maxWidth) {
            std::cerr << "[AVISO] Nivel '" << name
                      << "': Plataforma fora dos limites laterais! X="
                      << localBounds.min.x << "\n";
        }
        if (localBounds.max.y > config::LOGICAL_HEIGHT) {
            std::cerr << "[AVISO] Nivel '" << name
                      << "': Plataforma ultrapassa uma tela de altura (Y_local="
                      << localBounds.max.y << " > " << config::LOGICAL_HEIGHT << ")\n";
        }

        addPlatform(localBounds.min.x,
                    localBounds.min.y + offsetY,
                    localBounds.width(),
                    localBounds.height());
        highestY = std::max(highestY, localBounds.max.y + offsetY);
    }

    if (data.flag) {
        hasFlag = true;
        flagBounds = AABB{
            {data.flag->min.x, data.flag->min.y + offsetY},
            {data.flag->max.x, data.flag->max.y + offsetY}
        };

        if (data.flag->max.y > config::LOGICAL_HEIGHT) {
            std::cerr << "[AVISO] Nivel '" << name
                      << "': FLAG ultrapassa uma tela de altura (Y_local="
                      << data.flag->max.y << " > " << config::LOGICAL_HEIGHT << ")\n";
        }
        highestY = std::max(highestY, data.flag->max.y + offsetY);
    }

    std::cout << "[ASCENDENDO] Chunk colado em Y=" << offsetY
              << ". O topo do conteudo e " << highestY << "\n";
    return offsetY + config::LOGICAL_HEIGHT;
}

void Level::addPlatform(float x, float y, float w, float h) {
    m_platforms.push_back({AABB{{x, y}, {x + w, y + h}}});
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
                body.isGrounded = true;
                collided = true;
            }
            continue;
        }

        float exitLeft = bodyAABB.max.x - platform.bounds.min.x;
        float exitRight = platform.bounds.max.x - bodyAABB.min.x;
        float exitUp = platform.bounds.max.y - bodyAABB.min.y;
        float exitDown = bodyAABB.max.y - platform.bounds.min.y;

        float minExitX = std::min(exitLeft, exitRight);
        float minExitY = std::min(exitUp, exitDown);

        bool lateralCollision =
            std::abs(body.velocity.x) > std::abs(body.velocity.y) &&
            minExitX <= minExitY;

        if (lateralCollision) {
            body.position.x =
                (exitLeft < exitRight)
                    ? platform.bounds.min.x - body.width
                    : platform.bounds.max.x;
            body.velocity.x = -body.velocity.x * 0.3f;
            collided = true;
        } else {
            if (exitUp <= exitDown) {
                if (body.velocity.y <= 0.0f) {
                    body.position.y = platform.bounds.max.y;
                    body.velocity.y = 0.0f;
                    body.isGrounded = true;
                    collided = true;
                }
            } else if (body.velocity.y > 0.0f) {
                body.position.y = platform.bounds.min.y - body.height;
                body.velocity.y = -body.velocity.y * 0.3f;
                body.velocity.x *= 0.9f;
                collided = true;
            }
        }
    }
    return collided;
}

} // namespace logic
