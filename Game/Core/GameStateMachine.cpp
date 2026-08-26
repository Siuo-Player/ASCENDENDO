#include "Game/Core/GameStateMachine.h"

namespace core {

GameStateMachine::GameStateMachine(gfx::GameState initialState) noexcept
    : state_(initialState),
      editorReturnState_(gfx::GameState::MENU),
      creditsReturnState_(gfx::GameState::MENU),
      menuSelection_(0) {}

void GameStateMachine::selectRelative(int delta, int count) noexcept {
    if (count <= 0) {
        menuSelection_ = 0;
        return;
    }

    menuSelection_ = (menuSelection_ + delta) % count;
    if (menuSelection_ < 0) {
        menuSelection_ += count;
    }
}

void GameStateMachine::select(int index, int count) noexcept {
    if (count <= 0) {
        menuSelection_ = 0;
        return;
    }

    menuSelection_ = index % count;
    if (menuSelection_ < 0) {
        menuSelection_ += count;
    }
}

void GameStateMachine::enterPlaying() noexcept {
    state_ = gfx::GameState::PLAYING;
    menuSelection_ = 0;
}

void GameStateMachine::enterEditor(gfx::GameState returnState) noexcept {
    editorReturnState_ = returnState;
    state_ = gfx::GameState::EDITOR;
    menuSelection_ = 0;
}

void GameStateMachine::enterCredits(gfx::GameState returnState) noexcept {
    creditsReturnState_ = returnState;
    state_ = gfx::GameState::CREDITS;
    menuSelection_ = 0;
}

void GameStateMachine::returnFromEditor() noexcept {
    state_ = editorReturnState_;
    menuSelection_ = 0;
}

void GameStateMachine::returnFromCredits() noexcept {
    state_ = creditsReturnState_;
    menuSelection_ = 0;
}

void GameStateMachine::pause() noexcept {
    if (state_ == gfx::GameState::PLAYING) {
        state_ = gfx::GameState::PAUSED;
        menuSelection_ = 0;
    }
}

void GameStateMachine::resume() noexcept {
    if (state_ == gfx::GameState::PAUSED) {
        state_ = gfx::GameState::PLAYING;
        menuSelection_ = 0;
    }
}

void GameStateMachine::returnToMenu() noexcept {
    state_ = gfx::GameState::MENU;
    menuSelection_ = 0;
}

} // namespace core
