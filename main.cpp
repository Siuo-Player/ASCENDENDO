// =============================================================================
//  ASCENDENDO — Entry Point
//
//  @version 7.3
//  @history
//    v7.1  — Level streaming + Campaign + fullscreen + FLAG detection
//    v7.2  — Debug HUD: força do salto no terminal
//    v7.3  — FIX: spawn do jogador estava DENTRO da primeira plataforma (Y=0).
//            Corrigido para Y=40 (queda visivel ate pousar no chao do chunk).
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

// Necessário para detetar a resolução nativa do monitor
#include <GLFW/glfw3.h> 

#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace gfx;
using namespace logic;

int main() {
    std::cout << "[ASCENDENDO] A iniciar motor...\n";

    glfwInit();
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    int screenWidth = mode->width;
    int screenHeight = mode->height;

    {
        Window       win;
        VulkanContext ctx;
        Swapchain    swapchain;
        RenderPass   renderPass;
        Pipeline     pipeline;
        Renderer     renderer;
        InputManager input;

        if (!win.create(screenWidth, screenHeight, "ASCENDENDO")) return -1;

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

        std::vector<std::string> campaign;
        std::ifstream campaignFile("Game/Assets/Levels/campaign.txt");
        if (campaignFile.is_open()) {
            std::string line;
            while (std::getline(campaignFile, line)) {
                if (!line.empty() && line[0] != '#') {
                    if (line.back() == '\r') line.pop_back();
                    campaign.push_back("Game/Assets/Levels/" + line);
                }
            }
        }

        Level level;
        int currentLevelIndex = 0;
        float currentSpawnY = 0.0f;

        // Base sólida inicial
        level.addPlatform(0.0f, -10.0f, config::LOGICAL_WIDTH, 10.0f);

        if (!campaign.empty()) {
            currentSpawnY = level.appendFromFile(campaign[currentLevelIndex], config::LOGICAL_WIDTH, currentSpawnY);
            currentLevelIndex++;
        }

        PhysicsWorld world;
        Camera       camera;
        Player       player;

        // Jogador nasce 40px acima do chao do primeiro chunk (PLATFORM ... 0 ... h=20).
        // Antes era Y=0.0f, o que colocava o corpo DENTRO da plataforma (Y=[0,20]).
        // Pequena margem (40) garante uma queda visivel e natural ate pousar.
        player.body.position = { config::LOGICAL_WIDTH / 2.0f, 40.0f };

        auto lastTime = std::chrono::high_resolution_clock::now();
        std::cout << "[ASCENDENDO] A/D = mover | SPACE = saltar | ESC = sair\n";

        while (!win.shouldClose()) {
            auto  now = std::chrono::high_resolution_clock::now();
            float dt  = std::chrono::duration<float>(now - lastTime).count();
            lastTime  = now;

            input.beginFrame();
            win.pollEvents();
            if (input.isKeyDown(Key::ESCAPE)) break;

            int steps = world.advance(dt);
            for (int i = 0; i < steps; ++i) {
                player.update(input, world, config::FIXED_STEP);
                level.resolveCollision(player.body);
            }

            // ── Debug HUD: força do salto no terminal ─────────────────────────────
            // Remove quando a barra de força Vulkan estiver pronta (Fase 7.3).
            if (player.isCharging)
                std::cout << "\r  [SPACE] Força: " << static_cast<int>(player.chargeRatio() * 100) << "%   " << std::flush;
            else
                std::cout << "\r                              \r" << std::flush;

            camera.follow(player.position(), dt);

            if (player.position().y > currentSpawnY - config::LOGICAL_HEIGHT) {
                if (static_cast<size_t>(currentLevelIndex) < campaign.size()) {
                    currentSpawnY = level.appendFromFile(campaign[currentLevelIndex], config::LOGICAL_WIDTH, currentSpawnY);
                    currentLevelIndex++;
                }
            }

            if (level.hasFlag && PhysicsWorld::collides(player.body.bounds(), level.flagBounds)) {
                std::cout << "\n=========================================\n";
                std::cout << " 🎉 PARABÉNS! ALCANÇASTE O TOPO! 🎉\n";
                std::cout << "=========================================\n\n";
                break;
            }

            if (!renderer.drawFrame(player, camera, &level)) break;
        }

        vkDeviceWaitIdle(ctx.device());
    }

    glfwTerminate();
    std::cout << "[ASCENDENDO] Motor encerrado com sucesso.\n";
    return 0;
}