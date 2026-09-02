#include "doctest/doctest.h"
#include "Graphics/PlatformAssetSelector.h"

TEST_SUITE("Platform asset request mapping") {
    TEST_CASE("RegionCell mapping preserves selector metadata") {
        const gfx::compositor::RegionCell cell{
            2, 3, 436.25f, 89.5f, 7,
            gfx::compositor::Left | gfx::compositor::Right,
            gfx::compositor::TopologyClass::Interior};

        const auto request = gfx::assets::makePlatformAssetRequest(cell, true, 2);

        CHECK(static_cast<int>(request.topology) ==
              static_cast<int>(gfx::compositor::TopologyClass::Interior));
        CHECK(request.widthCells == 1);
        CHECK(request.heightCells == 1);
        CHECK(request.material == 7);
        CHECK(request.scale == 2);
        CHECK(request.mirrored);
    }

    TEST_CASE("default mapping preserves cell material and neutral presentation flags") {
        const gfx::compositor::RegionCell cell{
            0, 0, 0.25f, 100.5f, 11,
            gfx::compositor::None,
            gfx::compositor::TopologyClass::Isolated};

        const auto request = gfx::assets::makePlatformAssetRequest(cell);

        CHECK(static_cast<int>(request.topology) ==
              static_cast<int>(gfx::compositor::TopologyClass::Isolated));
        CHECK(request.widthCells == 1);
        CHECK(request.heightCells == 1);
        CHECK(request.material == 11);
        CHECK(request.scale == 1);
        CHECK_FALSE(request.mirrored);
    }
}
