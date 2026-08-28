// =============================================================================
// Tests/Unit/test_render_snapshot.cpp
// =============================================================================
#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/RenderSnapshot.h"
#include "../../Game/Graphics/RenderSnapshotBuilder.h"
#include "../../Game/Logic/Level.h"
#include "../../Game/Logic/Player.h"

#include <type_traits>

using namespace gfx;
using namespace logic;

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

    TEST_CASE("builder copia apenas os dados usados pelo world pass") {
        Player player;
        Level level;

        player.body.position = {32.0f, 64.0f};
        player.facingDirection = -1.0f;
        level.addPlatform(0.0f, 100.0f, 120.0f, 20.0f);
        level.addPlatform(50.0f, 160.0f, 80.0f, 16.0f);
        level.hasFlag = true;
        level.flagBounds = {{90.0f, 200.0f}, {110.0f, 260.0f}};

        const RenderSnapshot snapshot = buildRenderSnapshot(player, level);

        REQUIRE(snapshot.platforms.size() == 2);
        CHECK(snapshot.platforms[0].x == doctest::Approx(0.0f));
        CHECK(snapshot.platforms[0].y == doctest::Approx(100.0f));
        CHECK(snapshot.platforms[0].width == doctest::Approx(120.0f));
        CHECK(snapshot.platforms[0].height == doctest::Approx(20.0f));
        CHECK(snapshot.platforms[1].x == doctest::Approx(50.0f));
        CHECK(snapshot.platforms[1].y == doctest::Approx(160.0f));
        CHECK(snapshot.platforms[1].width == doctest::Approx(80.0f));
        CHECK(snapshot.platforms[1].height == doctest::Approx(16.0f));
        CHECK(snapshot.flag.visible);
        CHECK(snapshot.flag.bounds.x == doctest::Approx(90.0f));
        CHECK(snapshot.flag.bounds.y == doctest::Approx(200.0f));
        CHECK(snapshot.flag.bounds.width == doctest::Approx(20.0f));
        CHECK(snapshot.flag.bounds.height == doctest::Approx(60.0f));
        CHECK(snapshot.player.bounds.x == doctest::Approx(32.0f));
        CHECK(snapshot.player.bounds.y == doctest::Approx(64.0f));
        CHECK(snapshot.player.facingDirection == doctest::Approx(-1.0f));
    }

    TEST_CASE("snapshot fica independente de alterações posteriores no runtime") {
        Player player;
        Level level;
        player.body.position = {1.0f, 2.0f};
        level.addPlatform(3.0f, 4.0f, 5.0f, 6.0f);

        const RenderSnapshot snapshot = buildRenderSnapshot(player, level);

        player.body.position = {100.0f, 200.0f};
        level.addPlatform(20.0f, 30.0f, 40.0f, 50.0f);

        REQUIRE(snapshot.platforms.size() == 1);
        CHECK(snapshot.platforms[0].x == doctest::Approx(3.0f));
        CHECK(snapshot.platforms[0].y == doctest::Approx(4.0f));
        CHECK(snapshot.player.bounds.x == doctest::Approx(1.0f));
        CHECK(snapshot.player.bounds.y == doctest::Approx(2.0f));
    }
}
