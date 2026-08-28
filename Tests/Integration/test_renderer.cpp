// =============================================================================
//  Tests/Integration/test_renderer.cpp
//
//  Validação de integração da API pública RendererFacade após o cut-over.
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include "../../Game/Graphics/RenderPass.h"
#include "../../Game/Graphics/Pipeline.h"
#include "../../Game/Graphics/RendererFacade.h"
#include "../../Game/Graphics/Camera.h"
#include "../../Game/Graphics/GameState.h"
#include "../../Game/Graphics/RenderSnapshotBuilder.h"
#include "../../Game/Logic/Player.h"
#include "../../Game/Logic/Level.h"
#include "../../Game/Logic/Physics.h"
#include "../../Game/Core/Config.h"
#include <vector>

using namespace gfx;
using namespace logic;

TEST_SUITE("Renderer") {

    TEST_CASE("drawFrame: snapshot permanece válido durante a simulacao") {
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

        Player player;
        Level level;
        PhysicsWorld world;
        Camera camera;

        player.body.width  = 32.0f;
        player.body.height = 32.0f;
        player.body.position = {
            (config::LOGICAL_WIDTH / 2.0f) - 16.0f,
            300.0f
        };

        constexpr int FRAMES = 120;
        int successFrames = 0;

        for (int i = 0; i < FRAMES; ++i) {
            win.pollEvents();
            world.step(player.body, config::FIXED_STEP);
            const RenderSnapshot snapshot = buildRenderSnapshot(player, level);

            if (renderer.drawFrame(snapshot, camera, RenderState::PLAYING, 0,
                                   i * config::FIXED_STEP)) {
                ++successFrames;
            } else {
                break;
            }
        }

        vkDeviceWaitIdle(ctx.device());
        CHECK(successFrames == FRAMES);
        MESSAGE("RendererFacade manteve o contrato de drawFrame durante a simulacao.");
    }
}