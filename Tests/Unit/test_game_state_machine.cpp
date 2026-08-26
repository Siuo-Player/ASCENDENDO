#include "doctest/doctest.h"
#include "Core/GameStateMachine.h"

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

    TEST_CASE("absolute selection wraps in both directions") {
        GameStateMachine machine;
        machine.select(-1, 4);
        CHECK(machine.menuSelection() == 3);
        machine.select(6, 4);
        CHECK(machine.menuSelection() == 2);
    }

    TEST_CASE("selection handles invalid counts") {
        GameStateMachine machine;
        machine.select(2, 0);
        CHECK(machine.menuSelection() == 0);
        machine.selectRelative(1, 0);
        CHECK(machine.menuSelection() == 0);
    }

    TEST_CASE("entering playing resets menu selection") {
        GameStateMachine machine;
        machine.select(3, 4);
        machine.enterPlaying();
        CHECK(machine.state() == GameState::PLAYING);
        CHECK(machine.menuSelection() == 0);
    }

    TEST_CASE("editor remembers return state") {
        GameStateMachine machine;
        machine.enterEditor(GameState::PLAYING);
        CHECK(machine.state() == GameState::EDITOR);
        machine.returnFromEditor();
        CHECK(machine.state() == GameState::PLAYING);
        CHECK(machine.menuSelection() == 0);
    }

    TEST_CASE("editor from menu returns to menu") {
        GameStateMachine machine;
        machine.enterEditor(GameState::MENU);
        CHECK(machine.state() == GameState::EDITOR);
        machine.returnFromEditor();
        CHECK(machine.state() == GameState::MENU);
    }

    TEST_CASE("credits remembers return state") {
        GameStateMachine machine;
        machine.enterCredits(GameState::PAUSED);
        CHECK(machine.state() == GameState::CREDITS);
        machine.returnFromCredits();
        CHECK(machine.state() == GameState::PAUSED);
    }

    TEST_CASE("successful campaign returns from credits to menu") {
        GameStateMachine machine;
        machine.enterCredits(GameState::MENU);
        CHECK(machine.state() == GameState::CREDITS);
        machine.returnFromCredits();
        CHECK(machine.state() == GameState::MENU);
        CHECK(machine.menuSelection() == 0);
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

    TEST_CASE("pause exit to menu resets selection") {
        GameStateMachine machine(GameState::PLAYING);
        machine.pause();
        machine.select(2, 3);
        machine.returnToMenu();
        CHECK(machine.state() == GameState::MENU);
        CHECK(machine.menuSelection() == 0);
    }

    TEST_CASE("full menu to editor to menu flow") {
        GameStateMachine machine(GameState::MENU);
        machine.select(1, 4);
        machine.enterEditor(GameState::MENU);
        CHECK(machine.state() == GameState::EDITOR);
        machine.returnFromEditor();
        CHECK(machine.state() == GameState::MENU);
        CHECK(machine.menuSelection() == 0);
    }

    TEST_CASE("full play pause credits resume flow preserves explicit return states") {
        GameStateMachine machine;
        machine.enterPlaying();
        machine.pause();
        CHECK(machine.state() == GameState::PAUSED);
        machine.enterCredits(GameState::PAUSED);
        CHECK(machine.state() == GameState::CREDITS);
        machine.returnFromCredits();
        CHECK(machine.state() == GameState::PAUSED);
        machine.resume();
        CHECK(machine.state() == GameState::PLAYING);
    }
}
