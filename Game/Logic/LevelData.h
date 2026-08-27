#pragma once

#include "Logic/Physics.h"

#include <optional>
#include <string>
#include <vector>

namespace logic {

// Declarative, local representation of one .lvl document.
// No Vulkan/GLFW/runtime ownership or streaming state belongs here.
struct LevelData {
    std::string name = "Sem Nome";
    std::vector<AABB> platforms;
    Vec2 spawnPosition{};
    std::optional<AABB> flag;
};

} // namespace logic
