// =============================================================================
//  Tests/Integration/test_renderer.cpp
//
//  @version 5.2
//  @history
//    v5.2 — adicionado teste da Pipeline integrada no Renderer (Quadrado Vermelho)
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include "../../Game/Graphics/RenderPass.h"
#include "../../Game/Graphics/Pipeline.h"
#include "../../Game/Graphics/Renderer.h"
#include <vector>
#include <vulkan/vulkan.h>

using namespace gfx;

TEST_SUITE("Renderer") {

    TEST_CASE("init e drawFrame: Quadrado Vermelho na Tela") {
        Window       win;
        VulkanContext ctx;
        Swapchain    swapchain;
        RenderPass   renderPass;
        Pipeline     pipeline;
        Renderer     renderer;

        REQUIRE(win.create(800, 600, "ASCENDENDO v5.2 | Quadrado Vermelho com Letterbox"));

        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        REQUIRE(ctx.init(false, exts));

        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);
        REQUIRE(ctx.createSurface(surface));
        REQUIRE(swapchain.init(&ctx, &win));
        REQUIRE(renderPass.init(&ctx, &swapchain));
        REQUIRE(pipeline.init(&ctx, &swapchain, &renderPass));
        
        // Renderer agora recebe a pipeline! (4 argumentos)
        REQUIRE(renderer.init(&ctx, &swapchain, &renderPass, &pipeline));

        // ~120 frames = ~2 segundos de desenho antes de fechar a janela
        constexpr int FRAMES = 120;
        int successFrames = 0;

        for (int i = 0; i < FRAMES; ++i) {
            win.pollEvents();
            // Fundo preto absoluto (0, 0, 0)
            if (renderer.drawFrame(0.0f, 0.0f, 0.0f)) {
                ++successFrames;
            } else {
                break; 
            }
        }

        vkDeviceWaitIdle(ctx.device());
        CHECK(successFrames == FRAMES);
        MESSAGE("Fase 5.2 CONCLUIDA! Motor a renderizar Geometria e Shaders!");
    }
}