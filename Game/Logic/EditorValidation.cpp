#include "Logic/EditorValidation.h"
#include "Logic/LevelDataValidator.h"

#include <chrono>
#include <utility>

namespace logic {

EditorValidationTask::~EditorValidationTask() {
    discard();
}

bool EditorValidationTask::start(LevelData snapshot, std::string levelPath) {
    if (running()) return false;

    m_result = {};
    m_result.state = EditorValidationState::RUNNING;
    m_result.levelPath = levelPath;

    m_future = std::async(std::launch::async,
                          [snapshot = std::move(snapshot), levelPath = std::move(levelPath)]() mutable {
                              WorkResult result;
                              result.levelPath = std::move(levelPath);
                              result.valid = LevelDataValidator::validate(snapshot);
                              result.message = result.valid
                                  ? "Representação semântica válida"
                                  : "Representação semântica inválida";
                              return result;
                          });
    return true;
}

bool EditorValidationTask::running() const {
    if (!m_future.valid()) return false;
    return m_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
}

EditorValidationResult EditorValidationTask::poll() {
    if (!m_future.valid()) return m_result;
    if (running()) return m_result;

    const WorkResult work = m_future.get();
    m_result.state = EditorValidationState::COMPLETE;
    m_result.valid = work.valid;
    m_result.levelPath = work.levelPath;
    m_result.message = work.message;
    return m_result;
}

void EditorValidationTask::discard() {
    if (!m_future.valid()) return;
    m_future.wait();
    m_future = {};
    m_result = {};
}

} // namespace logic
