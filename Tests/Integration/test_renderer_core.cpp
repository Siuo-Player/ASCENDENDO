#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include "../../Game/Graphics/RenderPass.h"
#include "../../Game/Graphics/Pipeline.h"
#include "../../Game/Graphics/RendererCore.h"
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

using namespace gfx;

namespace {

class CurrentPathGuard {
public:
    explicit CurrentPathGuard(const std::filesystem::path& path)
        : previous_(std::filesystem::current_path()) {
        std::filesystem::current_path(path);
    }

    ~CurrentPathGuard() {
        std::error_code error;
        std::filesystem::current_path(previous_, error);
    }

private:
    std::filesystem::path previous_;
};

VkResult failDeviceWaitIdle(VkDevice) {
    return VK_ERROR_DEVICE_LOST;
}

} // namespace

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

        const auto uniqueId = std::chrono::steady_clock::now().time_since_epoch().count();
        const auto temporaryRoot = std::filesystem::temp_directory_path() /
            (std::string("ascendendo-pipeline-failure-") + std::to_string(uniqueId));
        REQUIRE(std::filesystem::create_directories(temporaryRoot));

        Pipeline retryablePipeline;
        {
            CurrentPathGuard pathGuard(temporaryRoot);
            CHECK_FALSE(retryablePipeline.init(&ctx, &swapchain, &renderPass));
            CHECK_FALSE(retryablePipeline.isInitialized());
            CHECK(retryablePipeline.handle() == VK_NULL_HANDLE);
            CHECK(retryablePipeline.layout() == VK_NULL_HANDLE);
        }

        REQUIRE(retryablePipeline.init(&ctx, &swapchain, &renderPass));
        CHECK(retryablePipeline.isInitialized());

        std::error_code cleanupError;
        std::filesystem::remove_all(temporaryRoot, cleanupError);

        vkDeviceWaitIdle(ctx.device());
    }

    TEST_CASE("Falha de wait-idle preserva o estado operacional") {
        Window win;
        VulkanContext ctx;
        Swapchain swapchain;
        RenderPass renderPass;
        Pipeline pipeline;
        RendererCore core(&failDeviceWaitIdle);

        REQUIRE(win.create(800, 600, "RendererCore wait-idle failure test"));

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

        const VkExtent2D before = core.swapchainExtent();
        CHECK(core.isInitialized());

        CHECK_FALSE(core.recreateSwapchain());
        CHECK(core.isInitialized());

        const VkExtent2D after = core.swapchainExtent();
        CHECK(after.width == before.width);
        CHECK(after.height == before.height);

        vkDeviceWaitIdle(ctx.device());
    }
}
