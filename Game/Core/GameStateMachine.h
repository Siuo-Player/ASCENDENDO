#pragma once

#include "Game/Graphics/GameState.h"

namespace core {

class GameStateMachine {
public:
    explicit GameStateMachine(gfx::GameState initialState = gfx::GameState::MENU) noexcept;

    gfx::GameState state() const noexcept { return state_; }
    gfx::GameState editorReturnState() const noexcept { return editorReturnState_; }
    gfx::GameState creditsReturnState() const noexcept { return creditsReturnState_; }
    int menuSelection() const noexcept { return menuSelection_; }

    void selectRelative(int delta, int count) noexcept;
    void select(int index, int count) noexcept;

    void enterPlaying() noexcept;
    void enterEditor(gfx::GameState returnState) noexcept;
    void enterCredits(gfx::GameState returnState) noexcept;
    void returnFromEditor() noexcept;
    void returnFromCredits() noexcept;
    void pause() noexcept;
    void resume() noexcept;
    void returnToMenu() noexcept;

    void resetMenuSelection() noexcept { menuSelection_ = 0; }

private:
    gfx::GameState state_;
    gfx::GameState editorReturnState_;
    gfx::GameState creditsReturnState_;
    int menuSelection_;
};

} // namespace core
