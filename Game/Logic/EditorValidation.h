#pragma once

#include "Logic/LevelData.h"
#include "Logic/LevelEditorValidator.h"

#include <cstdint>
#include <future>
#include <string>
#include <vector>

namespace logic {

enum class EditorValidationState {
    IDLE,
    RUNNING,
    COMPLETE,
    STALE,
};

struct EditorAsyncValidationResult {
    EditorValidationState state = EditorValidationState::IDLE;
    bool valid = false;
    bool reachesGoal = false;
    int reachablePlatforms = 0;
    int totalPlatforms = 0;
    std::vector<bool> platformReachable;
    std::uint64_t generation = 0;
    std::string levelPath;
    std::string message;
};

class EditorValidationTask {
public:
    EditorValidationTask() = default;
    ~EditorValidationTask();

    EditorValidationTask(const EditorValidationTask&) = delete;
    EditorValidationTask& operator=(const EditorValidationTask&) = delete;

    bool start(LevelData snapshot, std::uint64_t generation, std::string levelPath);
    bool running() const;
    EditorAsyncValidationResult poll();
    void discard();

private:
    struct WorkResult {
        EditorValidationResult validation;
        std::uint64_t generation = 0;
        std::string levelPath;
    };

    std::future<WorkResult> m_future;
    EditorAsyncValidationResult m_result{};
};

} // namespace logic
