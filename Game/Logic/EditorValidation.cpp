#include "Logic/EditorValidation.h"
#include "Logic/LevelEditor.h"
#include "Logic/Level.h"

#include <chrono>
#include <utility>

namespace logic {

EditorValidationTask::~EditorValidationTask() {
    discard();
}

bool EditorValidationTask::start(LevelData snapshot,
                                 std::uint64_t generation,
                                 std::string levelPath) {
    if (running()) return false;

    m_result = {};
    m_result.state = EditorValidationState::RUNNING;
    m_result.generation = generation;
    m_result.levelPath = levelPath;

    m_future = std::async(std::launch::async,
                          [snapshot = std::move(snapshot),
                           generation,
                           levelPath = std::move(levelPath)]() mutable {
                              WorkResult result;
                              result.generation = generation;
                              result.levelPath = std::move(levelPath);

                              LevelEditorDocument document(
                                  snapshot.finalCampaignLevel,
                                  {{0.0f, 0.0f},
                                   {config::LOGICAL_WIDTH, Level::AUTO_GROUND_HEIGHT}},
                                  snapshot.screenCount);

                              if (!document.restoreFromLevelData(
                                      snapshot,
                                      snapshot.finalCampaignLevel)) {
                                  result.validation.valid = false;
                                  result.validation.totalPlatforms =
                                      static_cast<int>(snapshot.platforms.size());
                                  result.validation.message = "Documento invalido para validacao";
                                  return result;
                              }

                              result.validation = validateEditorDocument(document);
                              return result;
                          });
    return true;
}

bool EditorValidationTask::running() const {
    if (!m_future.valid()) return false;
    return m_future.wait_for(std::chrono::seconds(0)) != std::future_status::ready;
}

EditorAsyncValidationResult EditorValidationTask::poll() {
    if (!m_future.valid()) return m_result;
    if (running()) return m_result;

    const WorkResult work = m_future.get();
    m_result.state = EditorValidationState::COMPLETE;
    m_result.valid = work.validation.valid;
    m_result.reachesGoal = work.validation.reachesGoal;
    m_result.reachablePlatforms = work.validation.reachablePlatforms;
    m_result.totalPlatforms = work.validation.totalPlatforms;
    m_result.platformReachable = work.validation.platformReachable;
    m_result.generation = work.generation;
    m_result.levelPath = work.levelPath;
    m_result.message = work.validation.message;
    return m_result;
}

void EditorValidationTask::discard() {
    if (!m_future.valid()) return;
    m_future.wait();
    m_future = {};
    m_result = {};
}

} // namespace logic
