// =============================================================================
//  ASCENDENDO — Entry Point
//
//  @version 7.5
//  @history
//    v7.1  — Campaign streaming + nivel nativo
//    v7.5  — GameState (PLAYING / CREDITS / MENU), FLAG visual,
//             ecra de creditos, menu simples com A/D + ESPACO
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
    GLFWmonitor*      primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode          = glfwGetVideoMode(primaryMonitor);
    int screenWidth  = mode->width;
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

        // ── Carregar lista de niveis da campanha ──────────────────────────────
        std::vector<std::string> campaign;
        {
            std::ifstream f("Game/Assets/Levels/campaign.txt");
            std::string line;
            while (std::getline(f, line)) {
                if (line.back() == '\r') line.pop_back();
                if (!line.empty() && line[0] != '#')
                    campaign.push_back("Game/Assets/Levels/" + line);
            }
        }

        // ── Estado do nivel ───────────────────────────────────────────────────
        Level        level;
        PhysicsWorld world;
        Camera       camera;
        Player       player;
        int          currentLevelIndex = 0;
        float        currentSpawnY     = 0.0f;

        // ── Estado do jogo ────────────────────────────────────────────────────
        GameState state   = GameState::PLAYING;
        int       menuSel = 0;   // 0 = Comecar  |  1 = Creditos

        // ── Funcao de reset / (re)inicio ─────────────────────────────────────
        auto resetGame = [&]() {
            player             = logic::Player{};
            player.body.position = { config::LOGICAL_WIDTH / 2.0f, 0.0f };
            camera             = gfx::Camera{};
            world              = logic::PhysicsWorld{};
            level.clear();
            currentLevelIndex  = 0;
            currentSpawnY      = 0.0f;

            // Chao absoluto + primeiro chunk
            level.addPlatform(0.0f, -10.0f, config::LOGICAL_WIDTH, 10.0f);
            if (!campaign.empty()) {
                currentSpawnY = level.appendFromFile(
                    campaign[0], config::LOGICAL_WIDTH, 0.0f);
                currentLevelIndex = 1;
            }

            state   = GameState::PLAYING;
            menuSel = 0;
        };

        // Arranque inicial
        resetGame();

        auto lastTime = std::chrono::high_resolution_clock::now();
        std::cout << "[ASCENDENDO] A/D = mover | SPACE = saltar | ESC = sair\n";

        while (!win.shouldClose()) {
            auto  now = std::chrono::high_resolution_clock::now();
            float dt  = std::chrono::duration<float>(now - lastTime).count();
            lastTime  = now;

            input.beginFrame();
            win.pollEvents();

            // ── ESC: fecha sempre ─────────────────────────────────────────────
            if (input.isKeyDown(Key::ESCAPE)) break;

            // ── Logica por estado ─────────────────────────────────────────────
            if (state == GameState::PLAYING) {
                // Fisica (Fixed Timestep)
                int steps = world.advance(dt);
                for (int i = 0; i < steps; ++i) {
                    player.update(input, world, config::FIXED_STEP);
                    level.resolveCollision(player.body);
                }

                // Camera tracking
                camera.follow(player.position(), dt);

                // Streaming: carregar proximo chunk quando o jogador se aproxima
                if (player.position().y > currentSpawnY - config::LOGICAL_HEIGHT) {
                    if (static_cast<size_t>(currentLevelIndex) < campaign.size()) {
                        currentSpawnY = level.appendFromFile(
                            campaign[currentLevelIndex],
                            config::LOGICAL_WIDTH, currentSpawnY);
                        currentLevelIndex++;
                    }
                }

                // Detecao de FLAG (fim da campanha)
                if (level.hasFlag &&
                    PhysicsWorld::collides(player.body.bounds(), level.flagBounds)) {
                    state = GameState::CREDITS;
                    std::cout
                        << "\n============================================\n"
                        << "  ASCENDENDO -- FIM DA CAMPANHA\n"
                        << "  Autor:         Rafael Gomes Bernardo\n"
                        << "  Auxiliado por: Claude (Anthropic)\n"
                        << "                 Gemini (Google)\n"
                        << "============================================\n"
                        << "  Pressiona ESPACO para continuar\n\n";
                }

            } else if (state == GameState::CREDITS) {
                // Qualquer tecla de accao avanca para o menu
                if (input.isKeyJustPressed(Key::SPACE)) {
                    state   = GameState::MENU;
                    menuSel = 0;
                }

            } else if (state == GameState::MENU) {
                // A/LEFT = opcao da esquerda (Comecar)
                if (input.isKeyJustPressed(Key::LEFT)  && menuSel != 0) menuSel = 0;
                // D/RIGHT = opcao da direita (Creditos)
                if (input.isKeyJustPressed(Key::RIGHT) && menuSel != 1) menuSel = 1;

                // ESPACO = confirmar seleccao
                if (input.isKeyJustPressed(Key::SPACE)) {
                    if (menuSel == 0) {
                        // Comecar: reinicia tudo
                        resetGame();
                    } else {
                        // Creditos: volta a mostrar os creditos
                        state = GameState::CREDITS;
                    }
                }
            }

            // ── Renderizacao ──────────────────────────────────────────────────
            if (!renderer.drawFrame(player, camera, &level, state, menuSel)) break;
        }

        vkDeviceWaitIdle(ctx.device());
    }

    glfwTerminate();
    std::cout << "[ASCENDENDO] Motor encerrado com sucesso.\n";
    return 0;
}
