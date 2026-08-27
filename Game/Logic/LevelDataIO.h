#pragma once

#include "Logic/LevelData.h"

#include <filesystem>
#include <optional>

namespace logic {

class LevelDataIO {
public:
    static std::optional<LevelData> load(const std::filesystem::path& path);

    static bool save(const LevelData& data,
                     const std::filesystem::path& path);
};

} // namespace logic
