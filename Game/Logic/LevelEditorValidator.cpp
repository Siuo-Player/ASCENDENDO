#include "Logic/LevelEditorValidator.h"
#include "Logic/LevelEditor.h"
#include "Core/Config.h"

#include <cmath>
#include <deque>
#include <vector>
#include <algorithm>

namespace logic {

namespace {
const float G_VAL = 980.0f;
const float V_MAX = 600.0f;
const float ANGLE = 3.14159265358979323846f / 3.0f;
const float TOLERANCE = 0.90f;

const float VY_EFF = V_MAX * std::sin(ANGLE) * TOLERANCE;
const float VX_EFF = V_MAX * std::cos(ANGLE) * TOLERANCE;
const float MAX_JUMP = (VY_EFF * VY_EFF) / (2.0f * G_VAL);

struct Node {
    enum class Kind { GROUND, PLATFORM, GOAL } kind;
    AABB bounds{};
    std::size_t platformIndex = static_cast<std::size_t>(-1);
};

bool reachable(const Node& from, const Node& to) {
    const float yStart = from.bounds.min.y + from.bounds.height();
    float yEnd = 0.0f;
    float dx = 0.0f;

    if (to.kind == Node::Kind::GOAL) {
        yEnd = to.bounds.min.y;
        dx = std::max(0.0f, std::max(
            to.bounds.min.x - from.bounds.max.x,
            from.bounds.min.x - to.bounds.max.x));
    } else {
        yEnd = to.bounds.min.y + to.bounds.height();
        dx = std::max(0.0f, std::max(
            to.bounds.min.x - from.bounds.max.x,
            from.bounds.min.x - to.bounds.max.x));
    }

    const float dy = yEnd - yStart;
    if (dy > MAX_JUMP) return false;

    const float discriminant =
        VY_EFF * VY_EFF -
        2.0f * G_VAL * std::max(0.0f, dy);
    if (discriminant < 0.0f) return false;

    const float maxDx =
        VX_EFF * (VY_EFF + std::sqrt(discriminant)) / G_VAL;
    return dx <= maxDx;
}

} // namespace

EditorValidationResult validateEditorDocument(const LevelEditorDocument& document) {
    EditorValidationResult result;
    result.totalPlatforms = static_cast<int>(document.platformCount());
    result.platformReachable.assign(document.platformCount(), false);

    std::vector<Node> nodes;
    nodes.reserve(document.platformCount() + 2);
    nodes.push_back({Node::Kind::GROUND, {{0.0f, 0.0f}, {config::LOGICAL_WIDTH, 20.0f}}});

    for (std::size_t i = 0; i < document.platformCount(); ++i) {
        nodes.push_back({Node::Kind::PLATFORM, document.platforms()[i].bounds, i});
    }

    Node goal{};
    goal.kind = Node::Kind::GOAL;
    goal.bounds = document.hasFlag() && document.flag()
        ? *document.flag()
        : AABB{{0.0f, config::LOGICAL_HEIGHT}, {0.0f, config::LOGICAL_HEIGHT}};
    nodes.push_back(goal);

    std::vector<bool> visited(nodes.size(), false);
    std::deque<std::size_t> queue;
    visited[0] = true;
    queue.push_back(0);

    while (!queue.empty()) {
        const std::size_t current = queue.front();
        queue.pop_front();

        if (current == nodes.size() - 1) {
            result.reachesGoal = true;
            break;
        }

        for (std::size_t j = 1; j < nodes.size(); ++j) {
            if (visited[j] || !reachable(nodes[current], nodes[j])) continue;
            visited[j] = true;
            queue.push_back(j);
            if (nodes[j].kind == Node::Kind::PLATFORM) {
                result.platformReachable[nodes[j].platformIndex] = true;
                ++result.reachablePlatforms;
            }
        }
    }

    result.valid = result.reachesGoal;
    if (result.reachesGoal) {
        result.message = "CAMINHO OK — objetivo alcancavel";
    } else if (document.platformCount() == 0) {
        result.valid = !document.isFinalCampaignLevel() || !document.hasFlag();
        result.message = "SEM OBSTACULOS — adicione plataformas para testar";
    } else {
        result.message = "SEM CAMINHO — ajuste distancia/altura das plataformas";
    }

    return result;
}

} // namespace logic
