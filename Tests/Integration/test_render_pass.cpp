#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include "../../Game/Graphics/RenderPass.h"
#include <vector>

using namespace gfx;

TEST_SUITE("RenderPass") {
    TEST_CASE("Criacao e destruicao do RenderPass") {
        Window win;
        VulkanContext ctx;
        Swapchain swapchain;
        RenderPass renderPass;

        // 1. Inicializar os alicerces (Janela -> Vulkan -> Surface -> Swapchain)
        REQUIRE(win.create(800, 600, "RenderPass Test"));
        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        REQUIRE(ctx.init(false, exts));
        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);
        REQUIRE(ctx.createSurface(surface));
        REQUIRE(swapchain.init(&ctx, &win));

        // 2. O teste principal: Criar o Render Pass
        bool ok = renderPass.init(&ctx, &swapchain);
        REQUIRE(ok == true);
        CHECK(renderPass.isInitialized());
        CHECK(renderPass.handle() != VK_NULL_HANDLE);
        
        MESSAGE("RenderPass criado com sucesso!");
    }
}