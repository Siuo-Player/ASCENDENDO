#pragma once

#include "Core/GameStateMachine.h"
#include "Core/KeyBindings.h"
#include "Logic/CampaignRuntime.h"
#include "Logic/EditorSession.h"
#include "Logic/InputManager.h"
#include "Logic/Level.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Logic/SimulationOrchestrator.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace logic {

struct GameSessionUpdateResult {
    bool quitRequested = false;
    bool campaignCompleted = false;
    bool runRecorded = false;
    bool stateChanged = false;
    float completionElapsedSeconds = 0.0f;
};

class GameSession {
public:
    GameSession(std::vector<std::filesystem::path> campaign,
                std::string campaignID,
                std::string runsCsvPath)
        : campaignRuntime_(std::move(campaign)),
          editorSession_(campaignRuntime_.levelCount() <= 1),
          campaignID_(std::move(campaignID)),
          runsCsvPath_(std::move(runsCsvPath)) {}

    GameSession(const GameSession&) = delete;
    GameSession& operator=(const GameSession&) = delete;
    GameSession(GameSession&&) = delete;
    GameSession& operator=(GameSession&&) = delete;

    void beginPlaying(float logicalWidth);
    void openEditor(core::GameState returnState) noexcept;

    GameSessionUpdateResult update(float dt,
                                   const InputManager& input,
                                   const core::KeyBindings& bindings,
                                   int32_t windowWidth,
                                   int32_t windowHeight,
                                   float logicalWidth,
                                   float logicalHeight);

    core::GameState state() const noexcept { return stateMachine_.state(); }
    int menuSelection() const noexcept { return stateMachine_.menuSelection(); }
    float elapsedTime() const noexcept { return elapsedTime_; }
    const std::string& campaignID() const noexcept { return campaignID_; }

    Player& player() noexcept { return player_; }
    const Player& player() const noexcept { return player_; }
    Level& level() noexcept { return level_; }
    const Level& level() const noexcept { return level_; }
    EditorSession& editorSession() noexcept { return editorSession_; }
    const EditorSession& editorSession() const noexcept { return editorSession_; }

private:
    int clickedMenuBox(const InputManager& input,
                       int32_t windowWidth,
                       int32_t windowHeight,
                       int count,
                       float logicalWidth) const;

    void resetGame(float logicalWidth);

    CampaignRuntime campaignRuntime_;
    Level level_;
    PhysicsWorld world_;
    SimulationOrchestrator simulation_;
    Player player_;
    EditorSession editorSession_;
    core::GameStateMachine stateMachine_;

    std::string campaignID_;
    std::string runsCsvPath_;
    float elapsedTime_ = 0.0f;
};

} // namespace logic
