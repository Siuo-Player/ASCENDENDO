// =============================================================================
//  Tests/Unit/test_swapchain.cpp
//
//  Lifecycle/precondition coverage for the Vulkan swapchain owner.
// =============================================================================
#include "doctest/doctest.h"
#include "Graphics/Swapchain.h"

using namespace gfx;

TEST_SUITE("Swapchain lifecycle") {

    TEST_CASE("recreate without an initialized swapchain fails closed") {
        Swapchain swapchain;

        CHECK_FALSE(swapchain.recreate());
        CHECK(swapchain.handle() == VK_NULL_HANDLE);
        CHECK_FALSE(swapchain.isInitialized());
        CHECK(swapchain.imageCount() == 0);
        CHECK(swapchain.imageViews().empty());
        CHECK(swapchain.images().empty());
    }

    TEST_CASE("cleanup on an uninitialized swapchain is a no-op") {
        Swapchain swapchain;

        swapchain.cleanup();

        CHECK(swapchain.handle() == VK_NULL_HANDLE);
        CHECK_FALSE(swapchain.isInitialized());
        CHECK(swapchain.imageViews().empty());
        CHECK(swapchain.images().empty());
    }
}
