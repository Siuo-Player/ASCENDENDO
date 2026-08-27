#pragma once

#include "Logic/LevelData.h"

#include <filesystem>
#include <optional>

namespace logic {

class LevelDataIO {
public:
    // Current declarative .lvl schema. Files without VERSION are accepted as
    // the historical schema 0 for backwards compatibility; new saves emit v1.
    static constexpr int CURRENT_VERSION = 1;

    static std::optional<LevelData> load(const std::filesystem::path& path);

    static bool save(const LevelData& data,
                     const std::filesystem::path& path);
};

} // namespace logic
