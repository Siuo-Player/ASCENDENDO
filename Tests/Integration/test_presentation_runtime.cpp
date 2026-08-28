#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/PresentationRuntime.h"
#include "../../Game/Graphics/RendererFacade.h"
#include "../../Game/Graphics/Pipeline.h"
#include "../../Game/Graphics/RenderPass.h"
#include "../../Game/Graphics/Swapchain.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Camera.h"
#include "../../Game/Graphics/RenderSnapshotBuilder.h"
#include "../../Game/Logic/Player.h"
#include "../../Game/Logic/Level.h"

#include <filesystem>
#include <vector>

using namespace gfx;
using namespace logic;

TEST_SUITE("PresentationRuntime") {
    TEST_CASE("optional presentation resources do not gate core renderer") {
        Window win;
        VulkanContext ctx;
        Swapchain swapchain;
        RenderPass renderPass;
        Pipeline pipeline;
        RendererFacade renderer;

        REQUIRE(win.create(800, 600, "ASCENDENDO | PresentationRuntime"));

        std::vector<const char*> extensions;
        win.appendRequiredExtensions(extensions);
        REQUIRE(ctx.init(false, extensions));

        const VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);
        REQUIRE(ctx.createSurface(surface));
        REQUIRE(swapchain.init(&ctx, &win));
        REQUIRE(renderPass.init(&ctx, &swapchain));
        REQUIRE(pipeline.init(&ctx, &swapchain, &renderPass));
        REQUIRE(renderer.init(&ctx, &swapchain, &renderPass, &pipeline));

        const auto missingSprite =
            std::filesystem::temp_directory_path() / "ascendendo-missing-sprite.png";

        {
            PresentationRuntime presentation;
            REQUIRE(presentation.init(&ctx, &swapchain, &renderPass,
                                      &renderer, missingSprite));

            CHECK(renderer.isInitialized());
            CHECK_FALSE(presentation.spriteReady());
            const bool textOrNothingReady = presentation.textReady() || !presentation.anyReady();
            CHECK(textOrNothingReady);
        }

        CHECK(renderer.isInitialized());

        Player player;
        Level level;
        Camera camera;
        const RenderSnapshot snapshot = buildRenderSnapshot(player, level);
        CHECK(renderer.drawFrame(snapshot, camera, RenderState::PLAYING, 0, 0.0f));

        vkDeviceWaitIdle(ctx.device());
    }
}
