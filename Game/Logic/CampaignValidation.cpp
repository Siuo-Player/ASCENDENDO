#include "Logic/CampaignValidation.h"

#include "Core/Config.h"
#include "Logic/LevelDataIO.h"
#include "Logic/LevelDataValidator.h"

#include <algorithm>
#include <cmath>
#include <queue>

namespace logic {
namespace {

constexpr float VY_EFF = config::PLAYER_MAX_JUMP * 0.8660254037844386f * 0.90f;
constexpr float VX_EFF = config::PLAYER_MAX_JUMP * 0.5f * 0.90f;
constexpr float GRAVITY_MAGNITUDE = 980.0f;
constexpr float MAX_JUMP = (VY_EFF * VY_EFF) / (2.0f * GRAVITY_MAGNITUDE);

struct Surface {
    float x;
    float y;
    float w;
    float h;

    float right() const noexcept { return x + w; }
    float top() const noexcept { return y + h; }
};

bool edgeReachable(const Surface& source, const Surface& target) {
    const float yStart = source.top();
    const float yEnd = target.top();
    const float horizontalGap = std::max(
        0.0f,
        std::max(target.x - source.right(), source.x - target.right()));
    const float verticalGap = yEnd - yStart;

    if (verticalGap > MAX_JUMP) return false;

    const float discriminant =
        VY_EFF * VY_EFF - 2.0f * GRAVITY_MAGNITUDE * std::max(0.0f, verticalGap);
    if (discriminant < 0.0f) return false;

    const float maxHorizontalDistance =
        VX_EFF * (VY_EFF + std::sqrt(discriminant)) / GRAVITY_MAGNITUDE;
    return horizontalGap <= maxHorizontalDistance;
}

bool physicallyPassable(const LevelData& data, std::size_t& reachablePlatforms) {
    const Surface ground{0.0f, 0.0f, config::LOGICAL_WIDTH, Level::AUTO_GROUND_HEIGHT};
    std::vector<Surface> surfaces;
    surfaces.reserve(data.platforms.size() + 1);
    surfaces.push_back(ground);
    for (const AABB& platform : data.platforms) {
        surfaces.push_back({platform.min.x, platform.min.y,
                            platform.width(), platform.height()});
    }

    std::vector<bool> visited(surfaces.size(), false);
    std::queue<std::size_t> queue;
    visited[0] = true;
    queue.push(0);

    while (!queue.empty()) {
        const std::size_t source = queue.front();
        queue.pop();
        for (std::size_t target = 1; target < surfaces.size(); ++target) {
            if (visited[target]) continue;
            if (!edgeReachable(surfaces[source], surfaces[target])) continue;
            visited[target] = true;
            queue.push(target);
        }
    }

    reachablePlatforms = 0;
    for (std::size_t i = 1; i < visited.size(); ++i)
        reachablePlatforms += visited[i] ? 1U : 0U;
    return reachablePlatforms == data.platforms.size();
}

} // namespace

CampaignValidationSnapshot validateCampaignForAdmin(
    const std::vector<std::string>& levelPaths) {
    CampaignValidationSnapshot snapshot;
    snapshot.entries.reserve(levelPaths.size());

    for (const std::string& path : levelPaths) {
        CampaignValidationEntry entry;
        const auto level = LevelDataIO::load(path);
        if (!level || !LevelDataValidator::validate(*level)) {
            entry.name = path;
            snapshot.entries.push_back(std::move(entry));
            continue;
        }

        entry.name = level->name.empty() ? path : level->name;
        entry.totalPlatforms = level->platforms.size();
        entry.valid = physicallyPassable(*level, entry.reachablePlatforms);
        if (entry.valid) ++snapshot.validLevels;
        snapshot.entries.push_back(std::move(entry));
    }

    return snapshot;
}

} // namespace logic
