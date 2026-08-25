#pragma once
// =============================================================================
// Game/Logic/LevelEditorValidator.h
// =============================================================================
// Validador nativo do editor. Mantém o mesmo modelo físico simplificado usado
// por Development/AI_Validation/ai_validator.py, mas funciona em memória.

#include <string>
#include <vector>

namespace logic {
class LevelEditorDocument;

struct EditorValidationResult {
    bool valid = false;
    bool reachesGoal = false;
    int reachablePlatforms = 0;
    int totalPlatforms = 0;
    std::string message;
    std::vector<bool> platformReachable;
};

EditorValidationResult validateEditorDocument(const LevelEditorDocument& document);

} // namespace logic
