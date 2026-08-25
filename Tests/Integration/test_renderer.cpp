// =============================================================================
//  Tests/Integration/test_renderer.cpp
//
//  Validação da API pública RendererFacade usando apenas presentation data.
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include "../../Game/Graphics/RenderPass.h"
#include "../../Game/Graphics/Pipeline.h"
#include "../../Game/Graphics/RendererFacade.h"
#include "../../Game/Graphics/RenderSnapshot.h"
#include "../../Game/Graphics/Camera.h"
#include "../../Game/Core/Config.h"
#include <vector>

using namespace gfx;

TEST_SUITE("Renderer") {

    TEST_CASE("drawFrame: RenderSnapshot mantem frame rendererizavel") {
        Window        win;
        VulkanContext ctx;
        Swapchain     swapchain;
        RenderPass    renderPass;
        Pipeline      pipeline;
        RendererFacade renderer;

        REQUIRE(win.create(800, 600, "ASCENDENDO | RendererFacade"));

        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        REQUIRE(ctx.init(false, exts));

        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);
        REQUIRE(ctx.createSurface(surface));
        REQUIRE(swapchain.init(&ctx, &win));
        REQUIRE(renderPass.init(&ctx, &swapchain));
        REQUIRE(pipeline.init(&ctx, &swapchain, &renderPass));
        REQUIRE(renderer.init(&ctx, &swapchain, &renderPass, &pipeline));

        Camera camera;
        RenderSnapshot snapshot;
        snapshot.player.bounds = {
            (config::LOGICAL_WIDTH / 2.0f) - 16.0f,
            300.0f,
            32.0f,
            32.0f
        };
        snapshot.player.facingDirection = 1.0f;
        snapshot.platforms.push_back({0.0f, 360.0f, config::LOGICAL_WIDTH, 16.0f});

        constexpr int FRAMES = 120;
        int successFrames = 0;

        for (int i = 0; i < FRAMES; ++i) {
            win.pollEvents();
            snapshot.player.bounds.y += 0.1f;

            if (renderer.drawFrame(snapshot, camera,
                                   GameState::PLAYING, 0,
                                   i * config::FIXED_STEP)) {
                ++successFrames;
            } else {
                break;
            }
        }

        vkDeviceWaitIdle(ctx.device());
        CHECK(successFrames == FRAMES);
        MESSAGE("RendererFacade manteve o contrato de frame baseado em RenderSnapshot.");
    }
}
