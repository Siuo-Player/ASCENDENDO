// =============================================================================
//  ASCENDENDO — Entry Point
// =============================================================================
#include "Game/Graphics/Window.h"
#include "Game/Graphics/VulkanContext.h"
#include "Game/Graphics/Swapchain.h"
#include "Game/Graphics/RenderPass.h"
#include "Game/Graphics/Pipeline.h"
#include "Game/Graphics/Renderer.h"
#include "Game/Graphics/Camera.h"
#include "Game/Logic/InputManager.h"
#include "Game/Logic/Player.h"
#include "Game/Logic/Physics.h"
#include "Game/Core/Config.h"

#include <chrono>
#include <iostream>

using namespace gfx;
using namespace logic;

int main() {
    std::cout << "[ASCENDENDO] A iniciar motor...\n";

    // O BLOCO MÁGICO: Garante que as classes são destruídas na ordem correta
    {
        Window win;
        VulkanContext ctx;
        Swapchain swapchain;
        RenderPass renderPass;
        Pipeline pipeline;
        Renderer renderer;
        InputManager input;

        if (!win.create(1280, 720, "ASCENDENDO - Motor Fisico")) return -1;
        
        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        ctx.init(false, exts);
        
        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        ctx.createSurface(surface);
        swapchain.init(&ctx, &win);
        renderPass.init(&ctx, &swapchain);
        pipeline.init(&ctx, &swapchain, &renderPass);
        renderer.init(&ctx, &swapchain, &renderPass, &pipeline);

        input.registerWithWindow(win.handle());

        PhysicsWorld world;
        Camera camera;
        Player player;
        
        player.body.position = { config::LOGICAL_WIDTH / 2.0f, 400.0f };

        auto lastTime = std::chrono::high_resolution_clock::now();
        std::cout << "[ASCENDENDO] Motor a correr! A/D para mover, SPACE para saltar.\n";

        while (!win.shouldClose()) {
            auto currentTime = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;

            input.beginFrame();
            win.pollEvents();

            if (input.isKeyDown(Key::ESCAPE)) break;

            int steps = world.advance(dt);
            for (int i = 0; i < steps; ++i) {
                player.update(input, world, config::FIXED_STEP);
            }

            if (!renderer.drawFrame(player, camera)) break;
        }

        vkDeviceWaitIdle(ctx.device());
    } // Aqui, todas as variaveis acima sao destruidas. O Renderer morre antes do VulkanContext!

    std::cout << "[ASCENDENDO] Motor encerrado com sucesso.\n";
    return 0;
}