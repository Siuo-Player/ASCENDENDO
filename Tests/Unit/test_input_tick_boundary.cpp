#include "../../external/doctest/doctest.h"
#include "../../Game/Logic/InputManager.h"
#include "../../Game/Core/KeyBindings.h"

using namespace logic;

TEST_SUITE("TickInput frame boundary") {

    TEST_CASE("press and release edges belong only to first tick of a frame") {
        InputManager input;
        core::KeyBindings bindings;

        input.injectRawState(false, false, true, true, false);

        const TickInput tick0 = input.tickInput(bindings, 0);
        const TickInput tick1 = input.tickInput(bindings, 1);

        CHECK(tick0.jumpHeld == true);
        CHECK(tick0.jumpPressed == true);
        CHECK(tick0.jumpReleased == false);

        CHECK(tick1.jumpHeld == true);
        CHECK(tick1.jumpPressed == false);
        CHECK(tick1.jumpReleased == false);

        input.injectRawState(false, false, false, false, true);

        const TickInput release0 = input.tickInput(bindings, 0);
        const TickInput release1 = input.tickInput(bindings, 1);

        CHECK(release0.jumpHeld == false);
        CHECK(release0.jumpPressed == false);
        CHECK(release0.jumpReleased == true);

        CHECK(release1.jumpHeld == false);
        CHECK(release1.jumpPressed == false);
        CHECK(release1.jumpReleased == false);
    }

    TEST_CASE("held actions are preserved across all fixed ticks derived from one frame") {
        InputManager input;
        core::KeyBindings bindings;

        input.injectRawState(true, false, false, false, false);

        for (std::size_t tick = 0; tick < 4; ++tick) {
            const TickInput semantic = input.tickInput(bindings, tick);
            CHECK(semantic.left == true);
            CHECK(semantic.right == false);
            CHECK(semantic.jumpHeld == false);
            CHECK(semantic.jumpPressed == false);
            CHECK(semantic.jumpReleased == false);
        }
    }
}
