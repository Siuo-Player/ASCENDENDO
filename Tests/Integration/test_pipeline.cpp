// =============================================================================
//  Tests/Integration/test_pipeline.cpp
//
//  @version 5.1
//  @history
//    v5.1 — testar carregamento dos shaders e criacao do pipeline state
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include "../../Game/Graphics/RenderPass.h"
#include "../../Game/Graphics/Pipeline.h"
#include <vector>

using namespace gfx;

TEST_SUITE("Fase 5.1 — Graphics Pipeline") {
    TEST_CASE("Criacao da Pipeline e Shaders") {
        Window win;
        VulkanContext ctx;
        Swapchain swapchain;
        RenderPass renderPass;
        Pipeline pipeline;

        REQUIRE(win.create(800, 600, "Pipeline Test"));
        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        REQUIRE(ctx.init(false, exts));

        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);
        REQUIRE(ctx.createSurface(surface));

        REQUIRE(swapchain.init(&ctx, &win));
        REQUIRE(renderPass.init(&ctx, &swapchain));

        // ── O Teste Principal ──
        bool ok = pipeline.init(&ctx, &swapchain, &renderPass);
        
        // Se este REQUIRE falhar, significa que o Makefile não compilou
        // os Shaders ou o Vulkan não os conseguiu ler da pasta Assets!
        REQUIRE(ok == true);
        
        CHECK(pipeline.isInitialized());
        CHECK(pipeline.handle() != VK_NULL_HANDLE);
        CHECK(pipeline.layout() != VK_NULL_HANDLE);

        MESSAGE("Graphics Pipeline criada com sucesso e Shaders carregados!");
    }
}