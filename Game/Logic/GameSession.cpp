#include "Logic/GameSession.h"

#include "Core/Config.h"
#include "Core/GameAction.h"
#include "Core/Viewport.h"
#include "Logic/RunHistory.h"

#include <cmath>

namespace logic {

namespace {
constexpr std::size_t INVALID_INDEX = static_cast<std::size_t>(-1);
}

void GameSession::configureCampaignEditor(std::string campaignFilePath) {
    campaignEditorPath_ = std::move(campaignFilePath);
    campaignEditorLoaded_ = false;
    campaignEditorDirty_ = false;
}

bool GameSession::openCampaignEditor(core::GameState returnState) {
    if (campaignEditorPath_.empty()) return false;

    // Parse into a candidate first. CampaignEditorDocument owns the canonical
    // campaign.txt semantics; a failed reload must not destroy the current UI
    // selection or a valid in-memory document.
    CampaignEditorDocument candidate;
    if (!candidate.loadFromCampaignFile(campaignEditorPath_)) return false;
    campaignEditor_ = std::move(candidate);
    campaignEditorLoaded_ = true;
    campaignEditorDirty_ = false;
    stateMachine_.enterCampaignEditor(returnState);
    return true;
}

bool GameSession::openSelectedCampaignLevel() {
    if (!campaignEditorLoaded_) return false;
    const CampaignLevelBlock* selected = campaignEditor_.selectedLevel();
    if (!selected) return false;

    // LevelDataIO is the only technical level parser here. Do not change
    // campaign selection or the active Level Editor until parsing and
    // final-level validation have succeeded.
    const std::optional<LevelData> data = LevelDataIO::load(selected->path);
    if (!data) return false;

    const bool finalCampaignLevel =
        campaignEditor_.selectedIndex() + 1 == campaignEditor_.levelCount();
    const std::string levelName = data->name.empty() ? selected->name : data->name;

    if (!editorSession_.loadLevelData(*data,
                                      finalCampaignLevel,
                                      selected->path,
                                      levelName)) {
        return false;
    }

    editorSession_.cancelInteraction();
    stateMachine_.enterEditor(core::GameState::CAMPAIGN_EDITOR);
    return true;
}

CampaignEditorRenderSnapshot GameSession::campaignEditorSnapshot() const {
    CampaignEditorRenderSnapshot snapshot;
    snapshot.levels = campaignEditor_.levels();
    snapshot.selectedIndex = campaignEditor_.selectedIndex();
    snapshot.contentHeight = campaignEditor_.contentHeight();
    return snapshot;
}

void GameSession::resetGame(float logicalWidth) {
    // Bootstrap the new campaign state transactionally. A failed load must not
    // leave the session in PLAYING with an empty or stale level.
    CampaignRuntime candidateRuntime = campaignRuntime_;
    Level candidateLevel;
    if (!candidateRuntime.loadInitialLevel(candidateLevel, logicalWidth)) return;

    player_ = logic::Player{};
    player_.body.position = {config::LOGICAL_WIDTH / 2.0f, 40.0f};
    world_ = logic::PhysicsWorld{};
    elapsedTime_ = 0.0f;
    level_ = std::move(candidateLevel);
    campaignRuntime_ = std::move(candidateRuntime);

    stateMachine_.enterPlaying();
}

void GameSession::beginPlaying(float logicalWidth) {
    resetGame(logicalWidth);
}

bool GameSession::beginPlayingLevel(std::size_t levelIndex, float logicalWidth) {
    // Load and validate the requested level before replacing the live session
    // state, so an invalid/unreadable level leaves the current session intact.
    CampaignRuntime candidateRuntime = campaignRuntime_;
    Level candidateLevel;
    if (!candidateRuntime.loadLevelAt(candidateLevel, levelIndex, logicalWidth)) return false;

    player_ = logic::Player{};
    player_.body.position = {config::LOGICAL_WIDTH / 2.0f, 40.0f};
    world_ = logic::PhysicsWorld{};
    elapsedTime_ = 0.0f;
    level_ = std::move(candidateLevel);
    campaignRuntime_ = std::move(candidateRuntime);

    stateMachine_.enterPlaying();
    return true;
}

void GameSession::openEditor(core::GameState returnState) noexcept {
    editorSession_.cancelInteraction();
    stateMachine_.enterEditor(returnState);
}

int GameSession::clickedMenuBox(const InputManager& input,
                                int32_t windowWidth,
                                int32_t windowHeight,
                                int count,
                                float logicalWidth) const {
    if (!input.isMouseButtonJustPressed(MouseButton::LEFT)) return -1;

    const core::LogicalPoint pt = core::windowToLogical(
        input.cursorX(), input.cursorY(),
        windowWidth, windowHeight,
        static_cast<int32_t>(logicalWidth), config::LOGICAL_HEIGHT);

    return core::hitTestMenuBox(pt.x, pt.y, count, logicalWidth);
}

GameSessionUpdateResult GameSession::update(float dt,
                                             const InputManager& input,
                                             const core::KeyBindings& bindings,
                                             int32_t windowWidth,
                                             int32_t windowHeight,
                                             float logicalWidth,
                                             float logicalHeight) {
    GameSessionUpdateResult result;
    const core::GameState currentState = stateMachine_.state();

    const bool pausePressed =
        core::isActionJustPressed(bindings, input, core::GameAction::Pause);
    const bool quitPressed =
        core::isActionJustPressed(bindings, input, core::GameAction::Quit);
    const bool openEditorPressed =
        core::isActionJustPressed(bindings, input, core::GameAction::OpenEditor);
    const bool openCampaignEditorPressed =
        core::isActionJustPressed(bindings, input, core::GameAction::OpenCampaignEditor);

    const bool campaignPrevious =
        core::isActionJustPressed(bindings, input, core::GameAction::CampaignSelectPrevious);
    const bool campaignNext =
        core::isActionJustPressed(bindings, input, core::GameAction::CampaignSelectNext);
    const bool campaignEarlier =
        core::isActionJustPressed(bindings, input, core::GameAction::CampaignMoveEarlier);
    const bool campaignLater =
        core::isActionJustPressed(bindings, input, core::GameAction::CampaignMoveLater);
    const bool campaignOpen =
        core::isActionJustPressed(bindings, input, core::GameAction::CampaignOpenLevel);
    const bool campaignSave =
        core::isActionJustPressed(bindings, input, core::GameAction::CampaignSave);

    const float safeDt = (std::isfinite(dt) && dt >= 0.0f) ? dt : 0.0f;

    switch (currentState) {
    case core::GameState::PLAYING:
        elapsedTime_ += safeDt;

        if (openCampaignEditorPressed) {
            openCampaignEditor(core::GameState::PLAYING);
        } else if (openEditorPressed) {
            openEditor(core::GameState::PLAYING);
        } else if (quitPressed) {
            editorSession_.cancelInteraction();
            stateMachine_.returnToMenu();
        } else if (pausePressed) {
            stateMachine_.pause();
        } else {
            simulation_.advance(safeDt, input, bindings, player_, world_, level_);

            if (player_.position().y >
                campaignRuntime_.currentSpawnY() - logicalHeight) {
                campaignRuntime_.streamNextLevel(level_, logicalWidth);
            }

            if (level_.hasFlag &&
                PhysicsWorld::collides(player_.body.bounds(), level_.flagBounds)) {
                result.campaignCompleted = true;
                result.completionElapsedSeconds = elapsedTime_;
                result.runRecorded = logic::recordRun(
                    runsCsvPath_, "Campanha Principal", campaignID_, elapsedTime_);
                stateMachine_.enterCredits(core::GameState::MENU);
            }
        }
        break;

    case core::GameState::PAUSED: {
        if (pausePressed) {
            stateMachine_.resume();
            break;
        }
        if (quitPressed) {
            stateMachine_.returnToMenu();
            break;
        }

        const int clickedPaused =
            clickedMenuBox(input, windowWidth, windowHeight, 3, logicalWidth);
        if (clickedPaused >= 0) stateMachine_.select(clickedPaused, 3);

        if (core::isActionJustPressed(bindings, input, core::GameAction::UILeft)) {
            stateMachine_.selectRelative(-1, 3);
        }
        if (core::isActionJustPressed(bindings, input, core::GameAction::UIRight)) {
            stateMachine_.selectRelative(+1, 3);
        }

        if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) ||
            clickedPaused >= 0) {
            if (stateMachine_.menuSelection() == 0) {
                stateMachine_.resume();
            } else if (stateMachine_.menuSelection() == 1) {
                stateMachine_.enterCredits(core::GameState::PAUSED);
            } else {
                stateMachine_.returnToMenu();
            }
        }
        break;
    }

    case core::GameState::CREDITS:
        if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) ||
            pausePressed) {
            stateMachine_.returnFromCredits();
        }
        break;

    case core::GameState::MENU: {
        if (quitPressed) {
            result.quitRequested = true;
            break;
        }

        if (openCampaignEditorPressed) {
            openCampaignEditor(core::GameState::MENU);
            break;
        }
        if (openEditorPressed) {
            openEditor(core::GameState::MENU);
            break;
        }

        const int clickedMenu =
            clickedMenuBox(input, windowWidth, windowHeight, 4, logicalWidth);
        if (clickedMenu >= 0) stateMachine_.select(clickedMenu, 4);

        if (core::isActionJustPressed(bindings, input, core::GameAction::UILeft)) {
            stateMachine_.selectRelative(-1, 4);
        }
        if (core::isActionJustPressed(bindings, input, core::GameAction::UIRight)) {
            stateMachine_.selectRelative(+1, 4);
        }

        if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) ||
            clickedMenu >= 0) {
            if (stateMachine_.menuSelection() == 0) {
                resetGame(logicalWidth);
            } else if (stateMachine_.menuSelection() == 1) {
                openEditor(core::GameState::MENU);
            } else if (stateMachine_.menuSelection() == 2) {
                stateMachine_.enterCredits(core::GameState::MENU);
            } else {
                result.quitRequested = true;
            }
        }
        break;
    }

    case core::GameState::CAMPAIGN_EDITOR: {
        if (quitPressed || pausePressed) {
            campaignEditorDirty_ = false;
            stateMachine_.returnFromCampaignEditor();
            break;
        }

        const std::size_t count = campaignEditor_.levelCount();
        if (count == 0) break;

        const std::size_t selected = campaignEditor_.selectedIndex();
        if (campaignPrevious && selected != INVALID_INDEX) {
            campaignEditor_.select(selected == 0 ? count - 1 : selected - 1);
        }
        if (campaignNext && selected != INVALID_INDEX) {
            campaignEditor_.select((selected + 1) % count);
        }

        const std::size_t current = campaignEditor_.selectedIndex();
        if (campaignEarlier && current != INVALID_INDEX && current > 0) {
            campaignEditor_.moveLevel(current, current - 1);
            campaignEditorDirty_ = true;
        }
        if (campaignLater && current != INVALID_INDEX && current + 1 < count) {
            campaignEditor_.moveLevel(current, current + 1);
            campaignEditorDirty_ = true;
        }

        if (campaignSave) {
            const CampaignSaveResult saved =
                campaignEditor_.saveToCampaignFile(campaignEditorPath_);
            if (saved.success) campaignEditorDirty_ = false;
        }

        if (campaignOpen) {
            openSelectedCampaignLevel();
        }
        break;
    }

    case core::GameState::EDITOR:
        if (pausePressed) {
            editorSession_.cancelInteraction();
            stateMachine_.returnFromEditor();
        } else {
            editorSession_.update(input, bindings, windowWidth, windowHeight);
        }
        break;
    }

    result.stateChanged = currentState != stateMachine_.state();
    return result;
}

} // namespace logic
