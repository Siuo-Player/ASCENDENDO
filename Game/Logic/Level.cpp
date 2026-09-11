#include "Logic/Level.h"
#include "Core/Config.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace logic {

float Level::appendFromData(const LevelData& data,
                            float maxWidth,
                            float offsetY,
                            bool finalCampaignLevel) {
    hasFlag = false;
    name = data.name;

    if (!m_spawnInitialized) {
        spawnPosition = data.spawnPosition.value_or(
            Vec2{config::LOGICAL_WIDTH / 2.0f, 40.0f});
        spawnPosition.y += offsetY;
        m_spawnInitialized = true;
    }

    float highestY = offsetY;
    const AABB* highestPlatform = nullptr;
    for (const auto& localBounds : data.platforms) {
        if (localBounds.min.x < 0.0f || localBounds.max.x > maxWidth) {
            std::cerr << "[AVISO] Nivel '" << name
                      << "': Plataforma fora dos limites laterais! X="
                      << localBounds.min.x << "\n";
        }
        if (localBounds.max.y >
            static_cast<float>(data.screenCount) * config::LOGICAL_HEIGHT) {
            std::cerr << "[AVISO] Nivel '" << name
                      << "': Plataforma ultrapassa a altura declarada do nivel (Y_local="
                      << localBounds.max.y << ")\n";
        }

        addPlatform(localBounds.min.x,
                    localBounds.min.y + offsetY,
                    localBounds.width(),
                    localBounds.height());
        highestY = std::max(highestY, localBounds.max.y + offsetY);

        if (highestPlatform == nullptr ||
            localBounds.max.y > highestPlatform->max.y ||
            (localBounds.max.y == highestPlatform->max.y &&
             localBounds.width() > highestPlatform->width()) ||
            (localBounds.max.y == highestPlatform->max.y &&
             localBounds.width() == highestPlatform->width() &&
             localBounds.min.x < highestPlatform->min.x)) {
            highestPlatform = &localBounds;
        }
    }

    if (finalCampaignLevel && highestPlatform != nullptr) {
        hasFlag = true;
        flagBounds = AABB{
            {highestPlatform->min.x, highestPlatform->max.y + offsetY},
            {highestPlatform->max.x,
             highestPlatform->max.y + offsetY + AUTO_FLAG_HEIGHT}
        };
        highestY = std::max(highestY, flagBounds.max.y);
    }

    const float chunkHeight =
        static_cast<float>(data.screenCount) * config::LOGICAL_HEIGHT;
    std::cout << "[ASCENDENDO] Chunk colado em Y=" << offsetY
              << ". O topo do conteudo e " << highestY
              << "; altura declarada=" << chunkHeight << "\n";
    return offsetY + chunkHeight;
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
