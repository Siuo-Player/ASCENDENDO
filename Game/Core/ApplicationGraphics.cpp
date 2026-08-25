#include "Core/ApplicationGraphics.h"

#include "Graphics/Window.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/Swapchain.h"
#include "Graphics/RenderPass.h"
#include "Graphics/Pipeline.h"
#include "Graphics/TextPipeline.h"
#include "Graphics/FontRenderer.h"
#include "Graphics/SpritePipeline.h"
#include "Graphics/SpriteRenderer.h"
#include "Graphics/RendererFacade.h"

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

namespace app {

bool initializeGraphics(
    gfx::Window& window,
    gfx::VulkanContext& context,
    gfx::Swapchain& swapchain,
    gfx::RenderPass& renderPass,
    gfx::Pipeline& pipeline,
    gfx::TextPipeline& textPipeline,
    gfx::FontRenderer& font,
    gfx::SpritePipeline& spritePipeline,
    gfx::SpriteRenderer& playerSprite,
    gfx::RendererFacade& renderer,
    int screenWidth,
    int screenHeight) {
    if (!window.create(screenWidth, screenHeight, "ASCENDENDO")) {
        std::cerr << "[ERRO] Nao foi possivel criar a janela GLFW.\n";
        return false;
    }

    std::vector<const char*> extensions;
    window.appendRequiredExtensions(extensions);
    if (!context.init(false, extensions)) {
        std::cerr << "[ERRO] Nao foi possivel inicializar Vulkan.\n";
        return false;
    }

    VkSurfaceKHR surface = window.createVulkanSurface(context.instance());
    if (surface == VK_NULL_HANDLE) {
        std::cerr << "[ERRO] Nao foi possivel criar a surface Vulkan.\n";
        return false;
    }
    if (!context.createSurface(surface)) {
        std::cerr << "[ERRO] Nao foi possivel associar a surface Vulkan ao contexto.\n";
        return false;
    }

    if (!swapchain.init(&context, &window)) {
        std::cerr << "[ERRO] Nao foi possivel inicializar o swapchain Vulkan.\n";
        return false;
    }
    if (!renderPass.init(&context, &swapchain)) {
        std::cerr << "[ERRO] Nao foi possivel criar o render pass Vulkan.\n";
        return false;
    }
    if (!pipeline.init(&context, &swapchain, &renderPass)) {
        std::cerr << "[ERRO] Nao foi possivel criar a pipeline grafica.\n";
        return false;
    }
    if (!renderer.init(&context, &swapchain, &renderPass, &pipeline)) {
        std::cerr << "[ERRO] Nao foi possivel inicializar o renderer.\n";
        return false;
    }

    if (textPipeline.init(&context, &swapchain, &renderPass) &&
        font.init(&context, textPipeline.descriptorSetLayout())) {
        renderer.attachText(&textPipeline, &font);
        std::cout << "[ASCENDENDO] Fonte TTF carregada (texto real em CREDITOS/MENU/PAUSA).\n";
    } else {
        std::cout << "[ASCENDENDO] Fonte TTF nao disponivel -- a usar BitmapFont (fallback).\n";
    }

    if (spritePipeline.init(&context, &swapchain, &renderPass) &&
        playerSprite.init(&context, spritePipeline.descriptorSetLayout(),
                          "Game/Assets/Sprites/personagem.png")) {
        renderer.attachSprite(&spritePipeline, &playerSprite);
        std::cout << "[ASCENDENDO] Sprite do jogador carregado ("
                  << playerSprite.width() << "x" << playerSprite.height() << ").\n";
    } else {
        std::cout << "[ASCENDENDO] Sprite do jogador nao disponivel -- a usar rectangulo (fallback).\n";
    }

    return true;
}

} // namespace app
