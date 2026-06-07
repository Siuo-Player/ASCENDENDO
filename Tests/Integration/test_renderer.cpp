// =============================================================================
//  Tests/Integration/test_renderer.cpp
//
//  @version 2.6
//  @history
//    v2.6  — criado (teste visual: 500 frames de azul escuro)
//    v2.60 — fix: remover shouldClose() do loop (janela fecha automaticamente)
//
//  TESTE VISUAL: este teste abre uma janela durante ~8 segundos (500 frames
//  a ~60 fps com V-Sync). Se vires a janela azul escuro, a Fase 2.6 esta
//  completa e o motor grafico base esta funcional.
//
//  Pipeline testada:
//    Window -> VulkanContext -> Surface -> Swapchain -> RenderPass -> Renderer
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include "../../Game/Graphics/RenderPass.h"
#include "../../Game/Graphics/Renderer.h"
#include <vector>
#include <vulkan/vulkan.h>

using namespace gfx;

TEST_SUITE("Renderer") {

    TEST_CASE("init: todos os recursos criados com sucesso") {
        Window       win;
        VulkanContext ctx;
        Swapchain    swapchain;
        RenderPass   renderPass;
        Renderer     renderer;

        // Pipeline de inicializacao completa
        REQUIRE(win.create(800, 600, "ASCENDENDO - Renderer Test"));

        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        REQUIRE(ctx.init(false, exts));

        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);
        REQUIRE(ctx.createSurface(surface));

        REQUIRE(swapchain.init(&ctx, &win));
        CHECK(swapchain.imageCount() >= 2);       // double buffering confirmado
        CHECK(swapchain.imageViews().size() >= 2); // image views criadas

        REQUIRE(renderPass.init(&ctx, &swapchain));
        REQUIRE(renderer.init(&ctx, &swapchain, &renderPass));

        CHECK(renderer.isInitialized());

        vkDeviceWaitIdle(ctx.device());
        MESSAGE("Renderer inicializado. Swapchain com ", swapchain.imageCount(), " imagens.");
    }

    TEST_CASE("drawFrame: 500 frames sem crash (janela azul escuro ~8s)") {
        Window       win;
        VulkanContext ctx;
        Swapchain    swapchain;
        RenderPass   renderPass;
        Renderer     renderer;

        REQUIRE(win.create(800, 600, "ASCENDENDO v2.6 | NAO FECHAR — fecha automaticamente"));

        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        REQUIRE(ctx.init(false, exts));

        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);
        REQUIRE(ctx.createSurface(surface));
        REQUIRE(swapchain.init(&ctx, &win));
        REQUIRE(renderPass.init(&ctx, &swapchain));
        REQUIRE(renderer.init(&ctx, &swapchain, &renderPass));

        // ── Mini render loop ──────────────────────────────────────────────────
        // Cor: azul escuro (r=0.05, g=0.05, b=0.20)
        // ~500 frames com V-Sync = ~8 segundos de janela visivel
        constexpr int FRAMES = 500;
        int successFrames = 0;

        for (int i = 0; i < FRAMES; ++i) {  // nao fechar a janela — o teste termina automaticamente
            win.pollEvents();
            if (renderer.drawFrame(0.05f, 0.05f, 0.20f)) {
                ++successFrames;
            } else {
                break; // falha irrecuperavel
            }
        }

        // GPU deve terminar antes de destruir os recursos
        vkDeviceWaitIdle(ctx.device());

        // Todos os frames devem ter sido renderizados com sucesso
        CHECK(successFrames == FRAMES);

        MESSAGE("Fase 2.6 CONCLUIDA! ", successFrames, "/", FRAMES,
                " frames renderizados. Motor grafico base operacional.");
    }

}
