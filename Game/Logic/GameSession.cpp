#include "Logic/GameSession.h"

#include "Core/CampaignID.h"
#include "Core/CampaignLoader.h"
#include "Core/Config.h"
#include "Core/GameAction.h"
#include "Core/Viewport.h"
#include "Logic/RunHistory.h"

#include <cmath>
#include <ctime>

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

void GameSession::setGhostReplay(RunReplay replay) {
    ghostReplay_ = std::move(replay);
    ghostTickIndex_ = 0;
}

bool GameSession::loadGhostReplay(const std::filesystem::path& path) {
    auto loaded = RunReplay::loadFromFile(path);
    if (!loaded) return false;
    setGhostReplay(std::move(*loaded));
    return true;
}

void GameSession::tryLoadDefaultGhostReplay() {
    if (profilePath_.empty() || campaignID_.empty()) return;
    const std::filesystem::path replayFile =
        std::filesystem::path(profilePath_).parent_path() / "Replays" / ("latest_" + campaignID_ + ".replay");
    if (std::filesystem::exists(replayFile)) {
        loadGhostReplay(replayFile);
    }
}

bool GameSession::selectCampaign(std::size_t index, float logicalWidth) {
    if (availableCampaigns_.empty() || index >= availableCampaigns_.size()) {
        return false;
    }

    const auto& desc = availableCampaigns_[index];
    auto levels = core::CampaignLoader::load(desc.playlistFile);
    if (levels.empty()) {
        return false;
    }

    activeCampaignIndex_ = index;
    campaignID_ = core::computeCampaignID(desc.playlistFile.parent_path().string());
    campaignName_ = desc.name;
    campaignEditorPath_ = desc.playlistFile.string();
    campaignRuntime_ = CampaignRuntime(std::move(levels));

    tryLoadDefaultGhostReplay();
    return true;
}

void GameSession::selectNextCampaign(float logicalWidth) {
    if (availableCampaigns_.empty()) return;
    const std::size_t nextIdx = (activeCampaignIndex_ + 1) % availableCampaigns_.size();
    selectCampaign(nextIdx, logicalWidth);
}

void GameSession::selectPreviousCampaign(float logicalWidth) {
    if (availableCampaigns_.empty()) return;
    const std::size_t prevIdx = activeCampaignIndex_ == 0
                                    ? availableCampaigns_.size() - 1
                                    : activeCampaignIndex_ - 1;
    selectCampaign(prevIdx, logicalWidth);
}

gfx::MenuRenderSnapshot GameSession::menuRenderSnapshot() const {
    gfx::MenuRenderSnapshot snapshot;
    snapshot.selection = stateMachine_.menuSelection();
    snapshot.activeCampaignIndex = activeCampaignIndex_;
    snapshot.ghostEnabled = ghostEnabled_;
    snapshot.activeHasReplay = ghostReplay_.has_value();
    snapshot.accountName = playerName_.empty() ? "Convidado" : playerName_;
    snapshot.campaigns.reserve(availableCampaigns_.size());

    for (const auto& desc : availableCampaigns_) {
        gfx::MenuCampaignItem item;
        item.id = desc.id;
        item.name = desc.name;
        auto levels = core::CampaignLoader::load(desc.playlistFile);
        item.levelCount = levels.size();

        const std::string cid = core::computeCampaignID(desc.playlistFile.parent_path().string());
        const auto* rec = userProfile_.findRecord(cid);
        if (rec) {
            item.bestTimeSeconds = rec->bestTimeSeconds;
            item.completions = rec->completions;
        }

        if (!profilePath_.empty()) {
            const std::filesystem::path replayFile =
                std::filesystem::path(profilePath_).parent_path() / "Replays" / ("latest_" + cid + ".replay");
            item.hasReplay = std::filesystem::exists(replayFile);
        } else {
            item.hasReplay = false;
        }

        snapshot.campaigns.push_back(std::move(item));
    }
    return snapshot;
}

gfx::RenderHudSnapshot GameSession::hudSnapshot() const {
    gfx::RenderHudSnapshot hud;
    hud.elapsedTime = elapsedTime_;
    hud.isReplayWatch = (stateMachine_.state() == core::GameState::REPLAY_WATCH);
    hud.replayTotalTime = (hud.isReplayWatch && ghostReplay_) ? ghostReplay_->header().claimedElapsedSeconds : 0.0f;
    const auto* rec = userProfile_.findRecord(campaignID_);
    if (rec) {
        hud.personalBestTime = rec->bestTimeSeconds;
    }

    const float y = player_.body.position.y;
    hud.altitudeMeters = (y > 0.0f) ? std::floor(y / 16.0f) : 0.0f;

    hud.ghostAvailable = ghostReplay_.has_value();
    hud.ghostVisible = ghostEnabled_ && ghostPlayer_.has_value();
    if (hud.ghostVisible && ghostPlayer_) {
        hud.ghostDeltaMeters = (player_.body.position.y - ghostPlayer_->body.position.y) / 16.0f;
    }

    if (!campaignName_.empty()) {
        std::snprintf(hud.campaignName, sizeof(hud.campaignName), "%s", campaignName_.c_str());
    }

    return hud;
}

void GameSession::resetGame(float logicalWidth) {
    CampaignRuntime candidateRuntime = campaignRuntime_;
    Level candidateLevel;
    if (!candidateRuntime.loadInitialLevel(candidateLevel, logicalWidth)) return;

    player_ = logic::Player{};
    player_.body.position = candidateLevel.spawnPosition;
    player_.body.isGrounded = true;
    world_ = logic::PhysicsWorld{};
    elapsedTime_ = 0.0f;
    level_ = std::move(candidateLevel);
    campaignRuntime_ = std::move(candidateRuntime);

    currentReplay_.clear();
    currentReplay_.header().campaignID = campaignID_;
    currentReplay_.header().campaignName = campaignName_;
    currentReplay_.header().playerID = playerID_;
    currentReplay_.header().playerName = playerName_;

    if (ghostReplay_ && ghostReplay_->header().campaignID == campaignID_) {
        ghostPlayer_ = logic::Player{};
        ghostPlayer_->body.position = level_.spawnPosition;
        ghostPlayer_->body.isGrounded = true;
        ghostWorld_ = logic::PhysicsWorld{};
        ghostTickIndex_ = 0;
    } else {
        ghostPlayer_.reset();
        ghostTickIndex_ = 0;
    }

    stateMachine_.enterPlaying();
}

void GameSession::beginPlaying(float logicalWidth) {
    resetGame(logicalWidth);
}

bool GameSession::beginWatchingBestReplay(float logicalWidth) {
    if (!ghostReplay_ || ghostReplay_->inputs().empty()) {
        return false;
    }

    CampaignRuntime candidateRuntime = campaignRuntime_;
    Level candidateLevel;
    if (!candidateRuntime.loadInitialLevel(candidateLevel, logicalWidth)) {
        return false;
    }

    player_ = logic::Player{};
    player_.body.position = candidateLevel.spawnPosition;
    player_.body.isGrounded = true;
    world_ = logic::PhysicsWorld{};
    elapsedTime_ = 0.0f;
    replayWatchTickIndex_ = 0;
    level_ = std::move(candidateLevel);
    campaignRuntime_ = std::move(candidateRuntime);

    ghostPlayer_.reset();
    ghostTickIndex_ = 0;

    stateMachine_.enterReplayWatch();
    return true;
}

bool GameSession::beginPlayingLevel(std::size_t levelIndex, float logicalWidth) {
    CampaignRuntime candidateRuntime = campaignRuntime_;
    Level candidateLevel;
    if (!candidateRuntime.loadLevelAt(candidateLevel, levelIndex, logicalWidth)) return false;

    player_ = logic::Player{};
    player_.body.position = candidateLevel.spawnPosition;
    player_.body.isGrounded = true;
    world_ = logic::PhysicsWorld{};
    elapsedTime_ = 0.0f;
    level_ = std::move(candidateLevel);
    campaignRuntime_ = std::move(candidateRuntime);

    currentReplay_.clear();
    currentReplay_.header().campaignID = campaignID_;
    currentReplay_.header().campaignName = campaignName_;
    currentReplay_.header().playerID = playerID_;
    currentReplay_.header().playerName = playerName_;

    ghostPlayer_.reset();
    ghostTickIndex_ = 0;

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
    const bool toggleGhostPressed =
        core::isActionJustPressed(bindings, input, core::GameAction::ToggleGhost);

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
    case core::GameState::REPLAY_WATCH: {
        if (quitPressed || pausePressed) {
            stateMachine_.returnToMenu();
            break;
        }

        const int steps = world_.advance(safeDt);
        for (int i = 0; i < steps; ++i) {
            if (ghostReplay_ && replayWatchTickIndex_ < ghostReplay_->inputs().size()) {
                const TickInput& tickIn = ghostReplay_->inputs()[replayWatchTickIndex_++];
                player_.update(tickIn, world_, config::FIXED_STEP);
                level_.resolveCollision(player_.body);
                elapsedTime_ += config::FIXED_STEP;

                const float streamTriggerY =
                    campaignRuntime_.currentSpawnY() - config::CAMPAIGN_STREAM_PRELOAD_DISTANCE;
                if (player_.position().y >= streamTriggerY) {
                    campaignRuntime_.streamNextLevel(level_, logicalWidth);
                }

                if (level_.hasFlag &&
                    PhysicsWorld::collides(player_.body.bounds(), level_.flagBounds)) {
                    stateMachine_.returnToMenu();
                    break;
                }
            } else {
                stateMachine_.returnToMenu();
                break;
            }
        }
        break;
    }

    case core::GameState::PLAYING:
        if (toggleGhostPressed) {
            toggleGhost();
        } else if (openCampaignEditorPressed) {
            openCampaignEditor(core::GameState::PLAYING);
        } else if (openEditorPressed) {
            openEditor(core::GameState::PLAYING);
        } else if (quitPressed) {
            editorSession_.cancelInteraction();
            stateMachine_.returnToMenu();
        } else if (pausePressed) {
            stateMachine_.pause();
        } else {
            const int simulatedSteps =
                simulation_.advance(safeDt, input, bindings, player_, world_, level_, &currentReplay_);
            elapsedTime_ += static_cast<float>(simulatedSteps) * config::FIXED_STEP;

            // Ghost simulation
            if (ghostEnabled_ && ghostPlayer_ && ghostReplay_) {
                for (int s = 0; s < simulatedSteps; ++s) {
                    if (ghostTickIndex_ < ghostReplay_->inputs().size()) {
                        const TickInput& ghostIn = ghostReplay_->inputs()[ghostTickIndex_++];
                        ghostPlayer_->update(ghostIn, ghostWorld_, config::FIXED_STEP);
                        level_.resolveCollision(ghostPlayer_->body);
                    }
                }
            }

            const float streamTriggerY =
                campaignRuntime_.currentSpawnY() - config::CAMPAIGN_STREAM_PRELOAD_DISTANCE;
            if (player_.position().y >= streamTriggerY) {
                campaignRuntime_.streamNextLevel(level_, logicalWidth);
            }

            if (level_.hasFlag &&
                PhysicsWorld::collides(player_.body.bounds(), level_.flagBounds)) {
                result.campaignCompleted = true;
                result.completionElapsedSeconds = elapsedTime_;
                result.runRecorded = logic::recordRun(
                    runsCsvPath_, campaignName_, campaignID_, elapsedTime_);
                userProfile_.recordCompletion(campaignID_, elapsedTime_);
                if (!profilePath_.empty()) {
                    logic::ProfileStorage::save(profilePath_, userProfile_);
                }

                currentReplay_.header().claimedElapsedSeconds = elapsedTime_;
                currentReplay_.header().totalTicks = static_cast<uint32_t>(currentReplay_.inputs().size());
                currentReplay_.header().timestamp = static_cast<int64_t>(std::time(nullptr));
                lastCompletedReplay_ = currentReplay_;

                if (!profilePath_.empty()) {
                    std::error_code ec;
                    const std::filesystem::path replayDir =
                        std::filesystem::path(profilePath_).parent_path() / "Replays";
                    std::filesystem::create_directories(replayDir, ec);
                    if (!ec) {
                        const std::string filename = "latest_" + campaignID_ + ".replay";
                        lastCompletedReplay_->saveToFile(replayDir / filename);
                        ghostReplay_ = lastCompletedReplay_;
                    }
                }

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

        if (toggleGhostPressed) {
            toggleGhost();
            break;
        }

        const bool watchReplayPressed =
            core::isActionJustPressed(bindings, input, core::GameAction::WatchReplay);
        if (watchReplayPressed) {
            if (hasGhostReplay()) {
                beginWatchingBestReplay(logicalWidth);
                break;
            }
        }

        if (openCampaignEditorPressed) {
            openCampaignEditor(core::GameState::MENU);
            break;
        }
        if (openEditorPressed) {
            openEditor(core::GameState::MENU);
            break;
        }

        // Alternar campanha seleccionada atraves de atalhos de teclado (W/S, Up/Down, [, ])
        const bool prevCamp = input.isKeyJustPressed(Key::W) ||
                              input.isKeyJustPressed(Key::UP) ||
                              input.isKeyJustPressed(Key::LBRACKET);
        const bool nextCamp = input.isKeyJustPressed(Key::S) ||
                              input.isKeyJustPressed(Key::DOWN) ||
                              input.isKeyJustPressed(Key::RBRACKET);

        if (prevCamp) {
            selectPreviousCampaign(logicalWidth);
        } else if (nextCamp) {
            selectNextCampaign(logicalWidth);
        }

        // Clique com o rato na caixa de selecao de campanhas (Y entre 220 e 276)
        if (input.isMouseButtonJustPressed(MouseButton::LEFT)) {
            const core::LogicalPoint pt = core::windowToLogical(
                input.cursorX(), input.cursorY(),
                windowWidth, windowHeight,
                static_cast<int32_t>(logicalWidth), config::LOGICAL_HEIGHT);
            if (pt.y >= 220.0f && pt.y <= 276.0f && pt.x >= 44.0f && pt.x <= 596.0f) {
                if (pt.x < logicalWidth / 2.0f) {
                    selectPreviousCampaign(logicalWidth);
                } else {
                    selectNextCampaign(logicalWidth);
                }
            }
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
