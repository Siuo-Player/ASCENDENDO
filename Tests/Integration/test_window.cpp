// =============================================================================
//  Tests/Integration/test_window.cpp
//
//  Testes de janela e superficie Vulkan.
// =============================================================================

#include "doctest/doctest.h"

#ifdef GLFW_AVAILABLE

#include "Graphics/Window.h"
#include "Graphics/VulkanContext.h"
#include <vector>

TEST_SUITE("Window") {
    TEST_CASE("create: janela 800x600 e criada com sucesso") {
        gfx::Window win;
        const bool ok = win.create(800, 600, "ASCENDENDO - Teste");

        REQUIRE(ok == true);
        CHECK(win.isCreated());
        CHECK(win.handle() != nullptr);
        CHECK(win.width() == 800);
        CHECK(win.height() == 600);

        win.destroy();
    }

    TEST_CASE("destroy: idempotente (2x sem crash)") {
        gfx::Window win;
        win.create(400, 300, "Teste Destroy");

        win.destroy();
        win.destroy();

        CHECK(!win.isCreated());
        CHECK(win.handle() == nullptr);
    }

    TEST_CASE("appendRequiredExtensions: lista nao vazia") {
        gfx::Window win;
        REQUIRE(win.create(100, 100, "Teste Exts"));

        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);

        CHECK(exts.size() >= 2);
        MESSAGE("Extensoes GLFW requeridas: ", exts.size());
        win.destroy();
    }
}

TEST_SUITE("Window + VulkanContext") {
    TEST_CASE("surface Vulkan criada e associada ao contexto") {
        gfx::Window win;
        REQUIRE(win.create(800, 600, "Surface Test"));

        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);

        gfx::VulkanContext ctx;
        REQUIRE(ctx.init(false, exts));

        const VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);

        CHECK(ctx.createSurface(surface) == true);
        CHECK(ctx.surface() != VK_NULL_HANDLE);
        CHECK(ctx.graphicsQueue() != VK_NULL_HANDLE);
        CHECK(ctx.presentQueue() != VK_NULL_HANDLE);
        CHECK(ctx.graphicsFamily() != UINT32_MAX);
        CHECK(ctx.presentFamily() != UINT32_MAX);

        VkBool32 presentSupported = VK_FALSE;
        REQUIRE(vkGetPhysicalDeviceSurfaceSupportKHR(
            ctx.physicalDevice(), ctx.presentFamily(), ctx.surface(), &presentSupported) == VK_SUCCESS);
        CHECK(presentSupported == VK_TRUE);

        ctx.shutdown();
        CHECK(!ctx.isInitialized());
        win.destroy();
    }
}

#endif // GLFW_AVAILABLE
