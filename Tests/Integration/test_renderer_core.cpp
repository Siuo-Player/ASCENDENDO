#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include "../../Game/Graphics/RenderPass.h"
#include "../../Game/Graphics/Pipeline.h"
#include "../../Game/Graphics/RendererCore.h"
#include <vector>

using namespace gfx;

TEST_SUITE("RendererCore") {
    TEST_CASE("Recreacao mantem o core operacional") {
        Window win;
        VulkanContext ctx;
        Swapchain swapchain;
        RenderPass renderPass;
        Pipeline pipeline;
        RendererCore core;

        REQUIRE(win.create(800, 600, "RendererCore lifecycle test"));

        std::vector<const char*> extensions;
        win.appendRequiredExtensions(extensions);
        REQUIRE(ctx.init(false, extensions));

        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);
        REQUIRE(ctx.createSurface(surface));
        REQUIRE(swapchain.init(&ctx, &win));
        REQUIRE(renderPass.init(&ctx, &swapchain));
        REQUIRE(pipeline.init(&ctx, &swapchain, &renderPass));
        REQUIRE(core.init(&ctx, &swapchain, &renderPass, &pipeline));

        CHECK(core.isInitialized());
        CHECK(core.swapchainExtent().width == swapchain.extent().width);
        CHECK(core.swapchainExtent().height == swapchain.extent().height);

        REQUIRE(core.recreateSwapchain());
        CHECK(core.isInitialized());
        CHECK(core.swapchainExtent().width == swapchain.extent().width);
        CHECK(core.swapchainExtent().height == swapchain.extent().height);

        REQUIRE(core.recreateSwapchain());
        CHECK(core.isInitialized());

        vkDeviceWaitIdle(ctx.device());
    }
}
