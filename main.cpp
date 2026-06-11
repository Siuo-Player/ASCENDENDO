// =============================================================================
//  ASCENDENDO — Entry Point
//
//  @version 6.2b
//  @history
//    v6.1  — RAII game loop + Commitment Jump
//    v6.2b — Level com 4 plataformas; resolveCollision no loop; drawFrame(&level)
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
#include "Game/Logic/Level.h"
#include "Game/Core/Config.h"

#include <chrono>
#include <iostream>

using namespace gfx;
using namespace logic;

int main() {
    std::cout << "[ASCENDENDO] A iniciar motor...\n";

    {
        Window       win;
        VulkanContext ctx;
        Swapchain    swapchain;
        RenderPass   renderPass;
        Pipeline     pipeline;
        Renderer     renderer;
        InputManager input;

        if (!win.create(1280, 720, "ASCENDENDO — Plataformas")) return -1;

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

        // ── Nivel de demonstracao (Fase 6.2b) ────────────────────────────────
        // Coordenadas: X=[0,640], Y=[0,360] (Y cresce para cima)
        Level level;
        level.addPlatform( 50.0f,  60.0f, 540.0f, 20.0f); // Chao elevado  (Y=60-80)
        level.addPlatform(150.0f, 140.0f, 300.0f, 20.0f); // Plataforma media (Y=140-160)
        level.addPlatform( 70.0f, 220.0f, 180.0f, 20.0f); // Alta esquerda  (Y=220-240)
        level.addPlatform(390.0f, 200.0f, 170.0f, 20.0f); // Alta direita   (Y=200-220)

        PhysicsWorld world;
        Camera       camera;
        Player       player;

        // Jogador inicia acima do ecra e cai para as plataformas
        player.body.position = { config::LOGICAL_WIDTH / 2.0f, 400.0f };

        auto lastTime = std::chrono::high_resolution_clock::now();
        std::cout << "[ASCENDENDO] Motor a correr! A/D mover, SPACE saltar.\n";

        while (!win.shouldClose()) {
            auto  now = std::chrono::high_resolution_clock::now();
            float dt  = std::chrono::duration<float>(now - lastTime).count();
            lastTime  = now;

            input.beginFrame();
            win.pollEvents();

            if (input.isKeyDown(Key::ESCAPE)) break;

            // Fixed Timestep: fisica + resolucao de colisao por passo
            int steps = world.advance(dt);
            for (int i = 0; i < steps; ++i) {
                player.update(input, world, config::FIXED_STEP);
                level.resolveCollision(player.body); // one-way platforms
            }

            if (!renderer.drawFrame(player, camera, &level)) break;
        }

        vkDeviceWaitIdle(ctx.device());
    }

    std::cout << "[ASCENDENDO] Motor encerrado com sucesso.\n";
    return 0;
}
