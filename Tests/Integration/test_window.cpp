// =============================================================================
//  Tests/Integration/test_window.cpp
//
//  Testes de janela e superficie Vulkan.
//
//  Estado sem GLFW instalado: este ficheiro compila mas nao regista
//  nenhum test case (bloco inteiro dentro de #ifdef GLFW_AVAILABLE).
//
//  Para ativar: instalar GLFW em external/glfw/ — o Makefile deteta
//  automaticamente e define GLFW_AVAILABLE na proxima compilacao.
//
//  Ver README.md, Seccao 8.1, para instrucoes de instalacao.
// =============================================================================

#include "doctest/doctest.h"

#ifdef GLFW_AVAILABLE

#include "Graphics/Window.h"
#include "Graphics/VulkanContext.h"
#include <vector>

TEST_SUITE("Window") {

    // ── Teste 1 ───────────────────────────────────────────────────────────────
    TEST_CASE("create: janela 800x600 e criada com sucesso") {
        gfx::Window win;
        bool ok = win.create(800, 600, "ASCENDENDO - Teste");

        REQUIRE(ok == true);
        CHECK(win.isCreated());
        CHECK(win.handle() != nullptr);
        CHECK(win.width()  == 800);
        CHECK(win.height() == 600);

        win.destroy();
    }

    // ── Teste 2 ───────────────────────────────────────────────────────────────
    TEST_CASE("destroy: idempotente (2x sem crash)") {
        gfx::Window win;
        win.create(400, 300, "Teste Destroy");

        win.destroy();
        win.destroy(); // segunda chamada deve ser segura

        CHECK(!win.isCreated());
        CHECK(win.handle() == nullptr);
    }

    // ── Teste 3 ───────────────────────────────────────────────────────────────
    TEST_CASE("appendRequiredExtensions: lista nao vazia") {
        gfx::Window win;
        REQUIRE(win.create(100, 100, "Teste Exts"));

        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);

        // GLFW precisa de pelo menos VK_KHR_surface + extensao de plataforma
        CHECK(exts.size() >= 2);
        MESSAGE("Extensoes GLFW requeridas: ", exts.size());

        win.destroy();
    }

}

TEST_SUITE("Window + VulkanContext") {

    // ── Teste 4 ───────────────────────────────────────────────────────────────
    TEST_CASE("surface Vulkan criada e associada ao contexto") {
        // 1. Criar janela
        gfx::Window win;
        REQUIRE(win.create(800, 600, "Surface Test"));

        // 2. Obter extensoes necessarias para surface
        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);

        // 3. Inicializar VulkanContext com essas extensoes
        gfx::VulkanContext ctx;
        REQUIRE(ctx.init(false, exts));

        // 4. Criar surface a partir da janela
        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);

        // 5. Associar surface ao contexto
        bool ok = ctx.createSurface(surface);
        CHECK(ok == true);
        CHECK(ctx.surface() != VK_NULL_HANDLE);

        // 6. shutdown() deve destruir a surface tambem
        ctx.shutdown();
        CHECK(!ctx.isInitialized());

        win.destroy();
    }

}

#endif // GLFW_AVAILABLE
