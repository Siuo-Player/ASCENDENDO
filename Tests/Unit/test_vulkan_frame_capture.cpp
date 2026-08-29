#include "Graphics/VulkanFrameCapture.h"

#include <doctest/doctest.h>

TEST_CASE("VulkanFrameCapture only advertises the canonical swapchain format") {
    gfx::VulkanFrameCapture capture;

    CHECK(capture.supports(VK_FORMAT_B8G8R8A8_SRGB));
    CHECK_FALSE(capture.supports(VK_FORMAT_R8G8B8A8_SRGB));
    CHECK_FALSE(capture.supports(VK_FORMAT_B8G8R8A8_UNORM));
}
