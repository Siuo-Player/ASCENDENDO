#pragma once

#include "Core/GameState.h"

namespace core {

class GameStateMachine {
public:
    explicit GameStateMachine(GameState initialState = GameState::MENU) noexcept;

    GameState state() const noexcept { return state_; }
    GameState editorReturnState() const noexcept { return editorReturnState_; }
    GameState creditsReturnState() const noexcept { return creditsReturnState_; }
    int menuSelection() const noexcept { return menuSelection_; }

    void selectRelative(int delta, int count) noexcept;
    void select(int index, int count) noexcept;

    void enterPlaying() noexcept;
    void enterEditor(GameState returnState) noexcept;
    void enterCredits(GameState returnState) noexcept;
    void returnFromEditor() noexcept;
    void returnFromCredits() noexcept;
    void pause() noexcept;
    void resume() noexcept;
    void returnToMenu() noexcept;

    void resetMenuSelection() noexcept { menuSelection_ = 0; }

private:
    GameState state_;
    GameState editorReturnState_;
    GameState creditsReturnState_;
    int menuSelection_;
};

} // namespace core
