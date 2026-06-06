// =============================================================================
//  Tests/Unit/test_vulkan_context.cpp
//
//  Testes unitarios para gfx::VulkanContext.
//  Validam a interface publica sem testar detalhes de implementacao.
// =============================================================================

#include "doctest/doctest.h"
#include "Graphics/VulkanContext.h"
#include <string>

TEST_SUITE("VulkanContext") {

    // ── Teste 1 ───────────────────────────────────────────────────────────────
    TEST_CASE("init() devolve handles Vulkan validos") {
        gfx::VulkanContext ctx;
        bool ok = ctx.init(/*validationLayers=*/false);

        REQUIRE(ok == true);
        CHECK(ctx.isInitialized());
        CHECK(ctx.instance()       != VK_NULL_HANDLE);
        CHECK(ctx.physicalDevice() != VK_NULL_HANDLE);
        CHECK(ctx.device()         != VK_NULL_HANDLE);
        CHECK(ctx.graphicsQueue()  != VK_NULL_HANDLE);
        CHECK(ctx.graphicsFamily() != UINT32_MAX);

        ctx.shutdown();
    }

    // ── Teste 2 ───────────────────────────────────────────────────────────────
    TEST_CASE("GPU selecionada tem nome nao vazio") {
        gfx::VulkanContext ctx;
        REQUIRE(ctx.init(false));

        std::string name = ctx.deviceProperties().deviceName;
        CHECK(!name.empty());
        MESSAGE("GPU selecionada: ", name.c_str());

        ctx.shutdown();
    }

    // ── Teste 3 ───────────────────────────────────────────────────────────────
    TEST_CASE("shutdown() e idempotente") {
        gfx::VulkanContext ctx;
        ctx.init(false);

        ctx.shutdown();   // primeira vez: destroi tudo
        ctx.shutdown();   // segunda vez: nao deve crashar nem corromper estado

        CHECK(!ctx.isInitialized());
        CHECK(ctx.instance()       == VK_NULL_HANDLE);
        CHECK(ctx.physicalDevice() == VK_NULL_HANDLE);
        CHECK(ctx.device()         == VK_NULL_HANDLE);
        CHECK(ctx.graphicsQueue()  == VK_NULL_HANDLE);
    }

    // ── Teste 4 ───────────────────────────────────────────────────────────────
    TEST_CASE("contexto nao inicializado devolve VK_NULL_HANDLE") {
        gfx::VulkanContext ctx;
        // Sem chamar init() — todos os handles devem ser nulos

        CHECK(!ctx.isInitialized());
        CHECK(ctx.instance()       == VK_NULL_HANDLE);
        CHECK(ctx.physicalDevice() == VK_NULL_HANDLE);
        CHECK(ctx.device()         == VK_NULL_HANDLE);
        CHECK(ctx.graphicsQueue()  == VK_NULL_HANDLE);
    }

    // ── Teste 5 ───────────────────────────────────────────────────────────────
    TEST_CASE("init() e idempotente: chamar 2x e seguro") {
        gfx::VulkanContext ctx;
        bool first  = ctx.init(false);
        bool second = ctx.init(false);  // segunda chamada deve devolver true sem recriar

        CHECK(first  == true);
        CHECK(second == true);
        CHECK(ctx.isInitialized());

        ctx.shutdown();
    }

}
