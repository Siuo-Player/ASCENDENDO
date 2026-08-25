#pragma once

#include "Logic/CampaignEditor.h"

#include <cstddef>
#include <vector>

namespace logic {

struct ValidatorRunPreview {
    std::size_t levelIndex = 0;
    float localX = 0.0f;
    float localY = 0.0f;
    bool transitioningIn = false;
    bool transitioningOut = false;
};

struct CampaignEditorRenderSnapshot {
    std::vector<CampaignLevelBlock> levels;
    std::vector<ValidatorRunPreview> validatorRuns;
    std::size_t selectedIndex = static_cast<std::size_t>(-1);
    float contentHeight = 0.0f;
};

} // namespace logic
