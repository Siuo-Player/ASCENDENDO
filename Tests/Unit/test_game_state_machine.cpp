#include "doctest/doctest.h"
#include "Game/Core/GameStateMachine.h"

using core::GameStateMachine;
using gfx::GameState;

TEST_SUITE("GameStateMachine") {
    TEST_CASE("starts in requested state") {
        GameStateMachine machine(GameState::MENU);
        CHECK(machine.state() == GameState::MENU);
        CHECK(machine.menuSelection() == 0);
    }

    TEST_CASE("relative selection wraps in both directions") {
        GameStateMachine machine;
        machine.selectRelative(-1, 4);
        CHECK(machine.menuSelection() == 3);
        machine.selectRelative(2, 4);
        CHECK(machine.menuSelection() == 1);
    }

    TEST_CASE("selection handles invalid counts") {
        GameStateMachine machine;
        machine.select(2, 0);
        CHECK(machine.menuSelection() == 0);
        machine.selectRelative(1, 0);
        CHECK(machine.menuSelection() == 0);
    }

    TEST_CASE("editor remembers return state") {
        GameStateMachine machine;
        machine.enterEditor(GameState::PLAYING);
        CHECK(machine.state() == GameState::EDITOR);
        machine.returnFromEditor();
        CHECK(machine.state() == GameState::PLAYING);
    }

    TEST_CASE("credits remembers return state") {
        GameStateMachine machine;
        machine.enterCredits(GameState::PAUSED);
        CHECK(machine.state() == GameState::CREDITS);
        machine.returnFromCredits();
        CHECK(machine.state() == GameState::PAUSED);
    }

    TEST_CASE("pause and resume only affect gameplay") {
        GameStateMachine machine(GameState::MENU);
        machine.pause();
        CHECK(machine.state() == GameState::MENU);

        machine.enterPlaying();
        machine.pause();
        CHECK(machine.state() == GameState::PAUSED);
        machine.resume();
        CHECK(machine.state() == GameState::PLAYING);
    }

    TEST_CASE("menu and playing transitions reset selection") {
        GameStateMachine machine;
        machine.select(3, 4);
        machine.enterPlaying();
        CHECK(machine.menuSelection() == 0);
        machine.select(2, 4);
        machine.returnToMenu();
        CHECK(machine.state() == GameState::MENU);
        CHECK(machine.menuSelection() == 0);
    }
}
