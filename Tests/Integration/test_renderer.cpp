// =============================================================================
//  Tests/Integration/test_renderer.cpp
//
//  @version 5.3
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include "../../Game/Graphics/RenderPass.h"
#include "../../Game/Graphics/Pipeline.h"
#include "../../Game/Graphics/Renderer.h"
#include "../../Game/Graphics/Camera.h"
#include "../../Game/Logic/Player.h"
#include "../../Game/Logic/Physics.h"
#include "../../Game/Core/Config.h"
#include <vector>

using namespace gfx;
using namespace logic;

TEST_SUITE("Renderer") {

    TEST_CASE("drawFrame: Fisica e Player injetados (Cair no chao)") {
        Window       win;
        VulkanContext ctx;
        Swapchain    swapchain;
        RenderPass   renderPass;
        Pipeline     pipeline;
        Renderer     renderer;

        REQUIRE(win.create(800, 600, "ASCENDENDO v5.3 | A Fisica Funciona!"));

        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        REQUIRE(ctx.init(false, exts));

        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(ctx.createSurface(surface));
        REQUIRE(swapchain.init(&ctx, &win));
        REQUIRE(renderPass.init(&ctx, &swapchain));
        REQUIRE(pipeline.init(&ctx, &swapchain, &renderPass));
        REQUIRE(renderer.init(&ctx, &swapchain, &renderPass, &pipeline));

        // ── CRIAR O MUNDO LÓGICO ──
        Player player;
        PhysicsWorld world;
        Camera camera;

        // O Jogador nasce no ar, no meio do ecrã
        player.body.width  = 32.0f;
        player.body.height = 32.0f;
        player.body.position = { (config::LOGICAL_WIDTH / 2.0f) - 16.0f, 300.0f };

        // ~120 frames = ~2 segundos para admirares a queda livre
        constexpr int FRAMES = 120;
        int successFrames = 0;

        for (int i = 0; i < FRAMES; ++i) {
            win.pollEvents();
            
            // MAGIA: A simulação acontece aqui
            world.step(player.body, config::FIXED_STEP);

            // Desenhamos a simulação!
            if (renderer.drawFrame(player, camera)) {
                ++successFrames;
            } else {
                break; 
            }
        }

        vkDeviceWaitIdle(ctx.device());
        CHECK(successFrames == FRAMES);
        MESSAGE("Fase 5.3 CONCLUIDA! O quadrado caiu com gravidade e colidiu com o chao invisivel.");
    }
}