// =============================================================================
//  Tests/Integration/test_sprite_pipeline.cpp
//
//  @version 8.2
//  @history
//    v8.2 — criado. Testa SpritePipeline (pipeline dedicada com sampler
//            NEAREST) e SpriteRenderer (carga de PNG via stb_image +
//            upload do atlas para a GPU). Requer GPU/driver Vulkan real,
//            tal como test_text_pipeline.cpp.
//
//  Se este teste falhar com "SpritePipeline nao inicializou", confirma que
//  os shaders sprite.vert.spv/sprite.frag.spv existem em
//  Game/Assets/Shaders/ (correr 'make shaders'). Se falhar com
//  "SpriteRenderer nao inicializou", confirma que
//  Game/Assets/Sprites/personagem.png existe (reorganize.py converte o .pixil
//  automaticamente quando colocado na raiz).
// =============================================================================

#include "../../external/doctest/doctest.h"
#include "../../Game/Graphics/VulkanContext.h"
#include "../../Game/Graphics/Window.h"
#include "../../Game/Graphics/Swapchain.h"
#include "../../Game/Graphics/RenderPass.h"
#include "../../Game/Graphics/SpritePipeline.h"
#include "../../Game/Graphics/SpriteRenderer.h"
#include <vector>

using namespace gfx;

TEST_SUITE("Fase 8.2 — Sprite Pipeline (pixel-art)") {

    TEST_CASE("SpritePipeline: criacao com descriptor set layout") {
        Window        win;
        VulkanContext ctx;
        Swapchain     swapchain;
        RenderPass    renderPass;
        SpritePipeline spritePipeline;

        REQUIRE(win.create(800, 600, "SpritePipeline Test"));
        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        REQUIRE(ctx.init(false, exts));

        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(surface != VK_NULL_HANDLE);
        REQUIRE(ctx.createSurface(surface));
        REQUIRE(swapchain.init(&ctx, &win));
        REQUIRE(renderPass.init(&ctx, &swapchain));

        bool ok = spritePipeline.init(&ctx, &swapchain, &renderPass);
        REQUIRE(ok == true);

        CHECK(spritePipeline.isInitialized());
        CHECK(spritePipeline.handle() != VK_NULL_HANDLE);
        CHECK(spritePipeline.layout() != VK_NULL_HANDLE);
        CHECK(spritePipeline.descriptorSetLayout() != VK_NULL_HANDLE);

        MESSAGE("SpritePipeline criada com sucesso (sampler NEAREST, descriptor set incluido).");
    }

    TEST_CASE("SpriteRenderer: carga do PNG do jogador + upload para a GPU") {
        Window        win;
        VulkanContext ctx;
        Swapchain     swapchain;
        RenderPass    renderPass;
        SpritePipeline spritePipeline;
        SpriteRenderer sprite;

        REQUIRE(win.create(800, 600, "SpriteRenderer Test"));
        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        REQUIRE(ctx.init(false, exts));

        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        REQUIRE(ctx.createSurface(surface));
        REQUIRE(swapchain.init(&ctx, &win));
        REQUIRE(renderPass.init(&ctx, &swapchain));
        REQUIRE(spritePipeline.init(&ctx, &swapchain, &renderPass));

        // Caminho explicito — nao depender do valor por omissao da classe,
        // que ja mudou uma vez (player.png -> personagem.png) e pode
        // voltar a mudar. O teste deve continuar a testar o ficheiro REAL
        // que main.cpp carrega, nao um nome hipotetico.
        bool ok = sprite.init(&ctx, spritePipeline.descriptorSetLayout(),
                              "Game/Assets/Sprites/personagem.png");
        REQUIRE(ok == true);

        CHECK(sprite.isInitialized());
        CHECK(sprite.width()  > 0);
        CHECK(sprite.height() > 0);

        MESSAGE("SpriteRenderer: PNG carregado e enviado para a GPU com sucesso ("
                << sprite.width() << "x" << sprite.height() << ").");
    }
}
