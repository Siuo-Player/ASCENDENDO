// =============================================================================
//  ASCENDENDO — Entry Point
//
//  @version 6.9
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
#include <fstream>
#include <vector>
#include <string>

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

        if (!win.create(1280, 720, "ASCENDENDO — Playlist Engine")) return -1;

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

        // ── LER A PLAYLIST DE NÍVEIS (campaign.txt) ───────────────────────────
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

        int currentLevelIndex = 0;
        Level level;
        
        if (!campaign.empty()) {
            if (!level.loadFromFile(campaign[currentLevelIndex], config::LOGICAL_WIDTH)) {
                std::cerr << "[ASCENDENDO] Erro a carregar " << campaign[currentLevelIndex] << "\n";
            }
        } else {
            std::cerr << "[ASCENDENDO] AVISO: Playlist (campaign.txt) nao encontrada ou vazia!\n";
            level.addPlatform(0.0f, 0.0f, config::LOGICAL_WIDTH, 20.0f);
        }

        PhysicsWorld world;
        Camera       camera;
        Player       player;

        player.body.position = { config::LOGICAL_WIDTH / 2.0f, 0.0f };

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

            camera.follow(player.position(), dt);

            // ── Transição Dinâmica e Objetivo Final ───────────────────────────
            bool levelComplete = false;

            if (level.hasFlag) {
                // Último nível: Ganha ao tocar na bandeira
                if (PhysicsWorld::collides(player.body.bounds(), level.flagBounds)) {
                    levelComplete = true;
                }
            } else {
                // Nível normal: Ganha ao chegar à altura fixa do nível
                if (player.position().y >= config::LOGICAL_HEIGHT) {
                    levelComplete = true;
                }
            }

            if (levelComplete) {
                currentLevelIndex++;
                if (static_cast<size_t>(currentLevelIndex) < campaign.size()) {
                    level.loadFromFile(campaign[currentLevelIndex], config::LOGICAL_WIDTH);
                    player.body.position = { config::LOGICAL_WIDTH / 2.0f, 0.0f };
                    camera.position.y = 0.0f;                                     
                } else {
                    std::cout << "\n=========================================\n";
                    std::cout << " 🎉 PARABÉNS! COMPLETÁSTE A CAMPANHA! 🎉\n";
                    std::cout << "=========================================\n\n";
                    break;
                }
            }

            if (!renderer.drawFrame(player, camera, &level)) break;
        }

        vkDeviceWaitIdle(ctx.device());
    }

    std::cout << "[ASCENDENDO] Motor encerrado com sucesso.\n";
    return 0;
}