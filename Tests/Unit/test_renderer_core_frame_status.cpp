#include "Graphics/RendererCore.h"

#include <doctest/doctest.h>

TEST_SUITE("RendererCore Vulkan frame status") {
    TEST_CASE("successful acquire or present is Ready") {
        CHECK(gfx::RendererCore::classifyVulkanResult(VK_SUCCESS) ==
              gfx::RendererCore::FrameStatus::Ready);
    }

    TEST_CASE("out of date and suboptimal request swapchain recreation") {
        CHECK(gfx::RendererCore::classifyVulkanResult(VK_ERROR_OUT_OF_DATE_KHR) ==
              gfx::RendererCore::FrameStatus::SwapchainNeedsRecreate);
        CHECK(gfx::RendererCore::classifyVulkanResult(VK_SUBOPTIMAL_KHR) ==
              gfx::RendererCore::FrameStatus::SwapchainNeedsRecreate);
    }

    TEST_CASE("other Vulkan failures are fatal") {
        CHECK(gfx::RendererCore::classifyVulkanResult(VK_ERROR_DEVICE_LOST) ==
              gfx::RendererCore::FrameStatus::Fatal);
        CHECK(gfx::RendererCore::classifyVulkanResult(VK_TIMEOUT) ==
              gfx::RendererCore::FrameStatus::Fatal);
        CHECK(gfx::RendererCore::classifyVulkanResult(VK_ERROR_INITIALIZATION_FAILED) ==
              gfx::RendererCore::FrameStatus::Fatal);
    }
}
