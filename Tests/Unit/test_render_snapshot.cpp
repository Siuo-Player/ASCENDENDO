// =============================================================================
// Tests/Unit/test_render_snapshot.cpp
// =============================================================================
#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/RenderSnapshot.h"

#include <type_traits>

using namespace gfx;

TEST_SUITE("RenderSnapshot") {
    TEST_CASE("é composto apenas por dados de apresentação") {
        static_assert(std::is_default_constructible_v<RenderSnapshot>);
        static_assert(std::is_copy_constructible_v<RenderSnapshot>);
        static_assert(std::is_copy_assignable_v<RenderSnapshot>);

        RenderSnapshot snapshot;
        snapshot.player.bounds = {10.0f, 20.0f, 32.0f, 32.0f};
        snapshot.player.facingDirection = -1.0f;
        snapshot.platforms.push_back({0.0f, 300.0f, 640.0f, 16.0f});
        snapshot.flag.visible = true;
        snapshot.flag.bounds = {600.0f, 260.0f, 16.0f, 64.0f};

        const RenderSnapshot copy = snapshot;
        CHECK(copy.player.bounds.x == 10.0f);
        CHECK(copy.player.facingDirection == -1.0f);
        CHECK(copy.platforms.size() == 1);
        CHECK(copy.flag.visible);
        CHECK(copy.flag.bounds.height == 64.0f);
    }
}
