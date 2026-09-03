#pragma once

#include "Logic/LevelData.h"

#include <future>
#include <string>

namespace logic {

enum class EditorValidationState {
    IDLE,
    RUNNING,
    COMPLETE,
};

struct EditorValidationResult {
    EditorValidationState state = EditorValidationState::IDLE;
    bool valid = false;
    std::string levelPath;
    std::string message;
};

class EditorValidationTask {
public:
    EditorValidationTask() = default;
    ~EditorValidationTask();

    EditorValidationTask(const EditorValidationTask&) = delete;
    EditorValidationTask& operator=(const EditorValidationTask&) = delete;

    bool start(LevelData snapshot, std::string levelPath);
    bool running() const;
    EditorValidationResult poll();
    void discard();

private:
    struct WorkResult {
        bool valid = false;
        std::string levelPath;
        std::string message;
    };

    std::future<WorkResult> m_future;
    EditorValidationResult m_result{};
};

} // namespace logic
