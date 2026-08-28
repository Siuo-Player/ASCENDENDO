#pragma once

#include "Logic/LevelData.h"

namespace logic {

class LevelDataValidator {
public:
    // Validates semantic geometry invariants required by the current runtime model.
    // Parsing/syntax validation remains the responsibility of LevelDataIO.
    static bool validate(const LevelData& data) noexcept;
};

} // namespace logic
