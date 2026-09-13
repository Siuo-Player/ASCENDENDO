#pragma once

#include "Logic/Physics.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace logic {

// In-memory representation of one .lvl document.
// Authored level files contain NAME/SCREENS/PLATFORM only.
// spawnPosition and flag remain optional in memory for compatibility with
// existing editor/runtime interfaces, but the parser and serializer never
// accept or emit them; both are derived by the campaign/runtime layers.
struct LevelData {
    std::string name = "Sem Nome";
    std::vector<AABB> platforms;
    std::optional<Vec2> spawnPosition;
    std::optional<AABB> flag;

    // Validation metadata only. This is never serialized and lets background
    // editor validation reconstruct the same final/non-final policy as the
    // live document without making authored goal state persistent.
    bool finalCampaignLevel = false;

    // Default: one logical 640x360 screen.
    std::size_t screenCount = 1;
};

} // namespace logic
