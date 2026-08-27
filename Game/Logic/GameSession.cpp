#include "Logic/GameSession.h"

#include "Core/Config.h"
#include "Core/GameAction.h"
#include "Core/Viewport.h"
#include "Logic/RunHistory.h"

namespace logic {

void GameSession::resetGame(float logicalWidth) {
    player_ = logic::Player{};
    player_.body.position = {config::LOGICAL_WIDTH / 2.0f, 40.0f};
    world_ = logic::PhysicsWorld{};
    elapsedTime_ = 0.0f;

    campaignRuntime_.loadInitialLevel(level_, logicalWidth);
    stateMachine_.enterPlaying();
}

void GameSession::beginPlaying(float logicalWidth) {
    resetGame(logicalWidth);
}

void GameSession::openEditor(gfx::GameState returnState) noexcept {
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
    const gfx::GameState currentState = stateMachine_.state();

    const bool pausePressed =
        core::isActionJustPressed(bindings, input, core::GameAction::Pause);
    const bool quitPressed =
        core::isActionJustPressed(bindings, input, core::GameAction::Quit);
    const bool openEditorPressed =
        core::isActionJustPressed(bindings, input, core::GameAction::OpenEditor);

    switch (currentState) {
    case gfx::GameState::PLAYING:
        elapsedTime_ += dt;

        if (openEditorPressed) {
            openEditor(gfx::GameState::PLAYING);
        } else if (quitPressed) {
            editorSession_.cancelInteraction();
            stateMachine_.returnToMenu();
        } else if (pausePressed) {
            stateMachine_.pause();
        } else {
            simulation_.advance(dt, input, player_, world_, level_);

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
                stateMachine_.enterCredits(gfx::GameState::MENU);
            }
        }
        break;

    case gfx::GameState::PAUSED: {
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
                stateMachine_.enterCredits(gfx::GameState::PAUSED);
            } else {
                stateMachine_.returnToMenu();
            }
        }
        break;
    }

    case gfx::GameState::CREDITS:
        if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) ||
            pausePressed) {
            stateMachine_.returnFromCredits();
        }
        break;

    case gfx::GameState::MENU: {
        if (quitPressed) {
            result.quitRequested = true;
            break;
        }

        if (openEditorPressed) {
            openEditor(gfx::GameState::MENU);
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
                openEditor(gfx::GameState::MENU);
            } else if (stateMachine_.menuSelection() == 2) {
                stateMachine_.enterCredits(gfx::GameState::MENU);
            } else {
                result.quitRequested = true;
            }
        }
        break;
    }

    case gfx::GameState::EDITOR:
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
