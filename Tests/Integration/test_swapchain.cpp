#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include <vector>

using namespace gfx;

TEST_SUITE("Swapchain") {
    TEST_CASE("Criacao, recreacao e destruicao do Swapchain") {
        Window win;
        VulkanContext ctx;
        Swapchain swapchain;

        REQUIRE(win.create(800, 600, "Swapchain Test"));

        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        REQUIRE(ctx.init(false, exts));

        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);
        REQUIRE(ctx.createSurface(surface));

        REQUIRE(swapchain.init(&ctx, &win));
        REQUIRE(swapchain.isInitialized());
        REQUIRE(swapchain.handle() != VK_NULL_HANDLE);
        CHECK(swapchain.extent().width == 800);
        CHECK(swapchain.extent().height == 600);
        CHECK(swapchain.imageCount() >= 2);

        REQUIRE(swapchain.recreate());
        CHECK(swapchain.isInitialized());
        CHECK(swapchain.handle() != VK_NULL_HANDLE);
        CHECK(swapchain.extent().width == 800);
        CHECK(swapchain.extent().height == 600);
        CHECK(swapchain.imageCount() >= 2);
    }
}
