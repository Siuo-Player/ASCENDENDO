#pragma once

#include "Logic/Physics.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace logic {

// Declarative representation of one .lvl document.
// A level has fixed width and is composed of N logical 640x360 screens.
// World coordinates use increasing Y for upward progression.
// No Vulkan/GLFW/runtime ownership or streaming state belongs here.
struct LevelData {
    std::string name = "Sem Nome";
    std::vector<AABB> platforms;
    std::optional<Vec2> spawnPosition;
    std::optional<AABB> flag;

    // Backward-compatible default: old .lvl files without SCREENS are one screen.
    std::size_t screenCount = 1;
};

} // namespace logic
