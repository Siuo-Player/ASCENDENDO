#pragma once

#include "Core/GameStateMachine.h"
#include "Core/KeyBindings.h"
#include "Core/Viewport.h"
#include "Logic/CampaignEditor.h"
#include "Logic/CampaignEditorSnapshot.h"
#include "Logic/CampaignRuntime.h"
#include "Logic/EditorSession.h"
#include "Logic/InputManager.h"
#include "Logic/Level.h"
#include "Logic/LevelDataIO.h"
#include "Logic/LevelDataValidator.h"
#include "Logic/Physics.h"
#include "Logic/Player.h"
#include "Logic/SimulationOrchestrator.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <optional>
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
    bool beginPlayingLevel(std::size_t levelIndex, float logicalWidth);

    bool beginPlayingLevelForCapture(std::size_t levelIndex, float logicalWidth) {
        if (levelIndex >= campaignRuntime_.levelCount()) return false;

        const auto& levelPath = campaignRuntime_.levels()[levelIndex];
        const std::optional<LevelData> data = LevelDataIO::load(levelPath);
        if (!data || !LevelDataValidator::validate(*data)) return false;

        level_.clear();
        level_.appendFromData(*data, logicalWidth, 0.0f);
        player_ = logic::Player{};
        player_.body.position = level_.spawnPosition;
        world_ = logic::PhysicsWorld{};
        elapsedTime_ = 0.0f;
        stateMachine_.enterPlaying();
        return true;
    }

    void configureCampaignEditor(std::string campaignFilePath);
    bool openCampaignEditor(core::GameState returnState);
    bool openSelectedCampaignLevel();
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
    std::size_t currentCampaignLevelIndex() const noexcept {
        if (campaignRuntime_.levelCount() == 0) return 0;
        const std::size_t next = campaignRuntime_.currentLevelIndex();
        return next == 0 ? 0 : std::min(next - 1, campaignRuntime_.levelCount() - 1);
    }

    Player& player() noexcept { return player_; }
    const Player& player() const noexcept { return player_; }
    Level& level() noexcept { return level_; }
    const Level& level() const noexcept { return level_; }
    EditorSession& editorSession() noexcept { return editorSession_; }
    const EditorSession& editorSession() const noexcept { return editorSession_; }
    CampaignEditorDocument& campaignEditor() noexcept { return campaignEditor_; }
    const CampaignEditorDocument& campaignEditor() const noexcept { return campaignEditor_; }
    CampaignEditorRenderSnapshot campaignEditorSnapshot() const;
    bool campaignEditorDirty() const noexcept { return campaignEditorDirty_; }

private:
    core::LogicalPoint menuPointer(const InputManager& input,
                                   int32_t windowWidth,
                                   int32_t windowHeight,
                                   float logicalWidth) const;

    int clickedMenuBox(const InputManager& input,
                       int32_t windowWidth,
                       int32_t windowHeight,
                       int count,
                       float logicalWidth) const;

    int hoveredMenuBox(const InputManager& input,
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
    CampaignEditorDocument campaignEditor_;
    core::GameStateMachine stateMachine_;

    std::string campaignEditorPath_;
    bool campaignEditorLoaded_ = false;
    bool campaignEditorDirty_ = false;
    std::string campaignID_;
    std::string runsCsvPath_;
    float elapsedTime_ = 0.0f;
};

} // namespace logic
