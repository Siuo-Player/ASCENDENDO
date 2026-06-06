#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include <vector>

using namespace gfx;

TEST_SUITE("Swapchain") {
    TEST_CASE("Criacao e destruicao do Swapchain") {
        Window win;
        VulkanContext ctx;
        Swapchain swapchain;

        // 1. Criar janela
        REQUIRE(win.create(800, 600, "Swapchain Test"));
        
        // 2. Criar contexto Vulkan com extensoes da janela
        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        REQUIRE(ctx.init(false, exts));

        // 3. Criar a Surface (Usando o teu metodo correto!)
        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);
        REQUIRE(ctx.createSurface(surface));

        // 4. O momento da verdade: Criar Swapchain
        bool ok = swapchain.init(&ctx, &win);
        
        REQUIRE(ok == true);
        CHECK(swapchain.isInitialized());
        CHECK(swapchain.handle() != VK_NULL_HANDLE);
        CHECK(swapchain.extent().width == 800);
        CHECK(swapchain.extent().height == 600);
        
        MESSAGE("Swapchain criado com sucesso! Formato: ", swapchain.imageFormat());
    }
}