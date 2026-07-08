// =============================================================================
//  ASCENDENDO — Entry Point
//
//  @version 8.1
//  @history
//    v7.1  — Campaign streaming + nivel nativo
//    v7.5  — GameState (PLAYING / CREDITS / MENU), FLAG visual,
//             ecra de creditos, menu simples com A/D + ESPACO
//    v7.6  — TextPipeline + FontRenderer (texto TTF real via stb_truetype).
//             Se o .ttf ou os shaders text.*.spv nao existirem, o jogo
//             continua a funcionar (fallback automatico para BitmapFont).
//    v8.1  — GameState::PAUSED (ESC pausa em vez de sair; SAIR passa a ser
//             uma opcao explicita nos menus). Timer de run (pausa em
//             qualquer menu, retoma em PLAYING). Registo de runs completas
//             (Development/Runs/runs.csv) com ID deterministico da
//             campanha (CampaignID.h). "Creditos" acessivel a partir de
//             MENU e PAUSED, regressa ao estado que o chamou.
//    v8.2  — SpritePipeline + SpriteRenderer (jogador desenhado como
//             pixel-art via Game/Assets/Sprites/personagem.png, gerado a
//             partir do .pixil pelo reorganize.py). Fallback gracioso
//             para rectangulo solido se o PNG nao existir.
// =============================================================================
#include "Game/Graphics/Window.h"
#include "Game/Graphics/VulkanContext.h"
#include "Game/Graphics/Swapchain.h"
#include "Game/Graphics/RenderPass.h"
#include "Game/Graphics/Pipeline.h"
#include "Game/Graphics/TextPipeline.h"
#include "Game/Graphics/FontRenderer.h"
#include "Game/Graphics/SpritePipeline.h"
#include "Game/Graphics/SpriteRenderer.h"
#include "Game/Graphics/Renderer.h"
#include "Game/Graphics/Camera.h"
#include "Game/Logic/InputManager.h"
#include "Game/Logic/Player.h"
#include "Game/Logic/Physics.h"
#include "Game/Logic/Level.h"
#include "Game/Logic/RunHistory.h"
#include "Game/Core/Config.h"
#include "Game/Core/CampaignID.h"

#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace gfx;
using namespace logic;

// Nome de campanha para o registo de runs. Fixo por agora — passara a ser
// dinamico quando existir seleccao de multiplas campanhas (Fase 9).
static const std::string CAMPAIGN_NAME = "Campanha Principal";
static const std::string LEVELS_DIR    = "Game/Assets/Levels";
static const std::string RUNS_CSV_PATH = "Development/Runs/runs.csv";

int main() {
    std::cout << "[ASCENDENDO] A iniciar motor...\n";

    glfwInit();
    GLFWmonitor*      primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode          = glfwGetVideoMode(primaryMonitor);
    int screenWidth  = mode->width;
    int screenHeight = mode->height;

    {
        Window        win;
        VulkanContext ctx;
        Swapchain     swapchain;
        RenderPass    renderPass;
        Pipeline      pipeline;
        TextPipeline  textPipeline;
        FontRenderer  font;
        SpritePipeline spritePipeline;
        SpriteRenderer playerSprite;
        Renderer      renderer;
        InputManager  input;

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

        // Texto TTF real (CREDITS/MENU/PAUSED + timer HUD em PLAYING). Falha
        // graciosamente: se os shaders text.*.spv ou o .ttf nao existirem,
        // o jogo continua com o fallback BitmapFont.
        if (textPipeline.init(&ctx, &swapchain, &renderPass) &&
            font.init(&ctx, textPipeline.descriptorSetLayout())) {
            renderer.attachText(&textPipeline, &font);
            std::cout << "[ASCENDENDO] Fonte TTF carregada (texto real em CREDITOS/MENU/PAUSA).\n";
        } else {
            std::cout << "[ASCENDENDO] Fonte TTF nao disponivel -- a usar BitmapFont (fallback).\n";
        }

        // Sprite do jogador (pixel-art). Nome do ficheiro explicito aqui —
        // main.cpp e' a fonte da verdade sobre qual PNG representa o
        // jogador, sem depender de reorganize.py adivinhar isso a partir
        // do nome que Rafael der ao .pixil. Falha graciosamente: se o PNG
        // nao existir, o jogador continua a ser desenhado como rectangulo.
        if (spritePipeline.init(&ctx, &swapchain, &renderPass) &&
            playerSprite.init(&ctx, spritePipeline.descriptorSetLayout(),
                              "Game/Assets/Sprites/personagem.png")) {
            renderer.attachSprite(&spritePipeline, &playerSprite);
            std::cout << "[ASCENDENDO] Sprite do jogador carregado ("
                      << playerSprite.width() << "x" << playerSprite.height() << ").\n";
        } else {
            std::cout << "[ASCENDENDO] Sprite do jogador nao disponivel -- a usar rectangulo (fallback).\n";
        }

        input.registerWithWindow(win.handle());

        // ── Carregar lista de niveis da campanha ──────────────────────────────
        std::vector<std::string> campaign;
        {
            std::ifstream f(LEVELS_DIR + "/campaign.txt");
            std::string line;
            while (std::getline(f, line)) {
                if (!line.empty() && line.back() == '\r') line.pop_back();
                if (!line.empty() && line[0] != '#')
                    campaign.push_back(LEVELS_DIR + "/" + line);
            }
        }

        // ID deterministico da campanha actual (muda se qualquer .lvl ou a
        // ordem/composicao de campaign.txt mudar). Calculado uma vez — os
        // ficheiros nao mudam a meio de uma execucao.
        std::string campaignID = core::computeCampaignID(LEVELS_DIR);
        std::cout << "[ASCENDENDO] Campaign ID: "
                  << (campaignID.empty() ? "(indisponivel)" : campaignID) << "\n";

        // ── Estado do nivel ───────────────────────────────────────────────────
        Level        level;
        PhysicsWorld world;
        Camera       camera;
        Player       player;
        int          currentLevelIndex = 0;
        float        currentSpawnY     = 0.0f;

        // ── Estado do jogo ────────────────────────────────────────────────────
        GameState state             = GameState::PLAYING;
        int       menuSel           = 0;              // 0/1/2 conforme o menu activo
        float     elapsedTime       = 0.0f;            // timer da run (pausa fora de PLAYING)
        GameState creditsReturnState = GameState::MENU; // para onde voltar depois de CREDITS

        // ── Funcao de reset / (re)inicio ─────────────────────────────────────
        auto resetGame = [&]() {
            player             = logic::Player{};
            player.body.position = { config::LOGICAL_WIDTH / 2.0f, 40.0f }; // acima do chao (top=20)
            camera             = gfx::Camera{};
            world              = logic::PhysicsWorld{};
            level.clear();
            currentLevelIndex  = 0;
            currentSpawnY      = 0.0f;
            elapsedTime        = 0.0f;

            // Carregar primeiro chunk (inclui o chao full-width em inicio.lvl)
            if (!campaign.empty()) {
                currentSpawnY = level.appendFromFile(
                    campaign[0], config::LOGICAL_WIDTH, 0.0f);
                currentLevelIndex = 1;
            }

            state   = GameState::PLAYING;
            menuSel = 0;
            glfwSetWindowTitle(win.handle(), "ASCENDENDO");
        };

        // Navegacao com wraparound circular (0->1->2->0 e vice-versa) —
        // usada tanto por MENU (fim-de-run) como por PAUSED (3 opcoes cada).
        auto navigate = [&](int delta) {
            menuSel = (menuSel + delta + 3) % 3;
        };

        // Arranque inicial
        resetGame();

        auto lastTime = std::chrono::high_resolution_clock::now();
        std::cout << "[ASCENDENDO] A/D = mover | SPACE = saltar | ESC = pausa\n";

        while (!win.shouldClose()) {
            auto  now = std::chrono::high_resolution_clock::now();
            float dt  = std::chrono::duration<float>(now - lastTime).count();
            lastTime  = now;

            input.beginFrame();
            win.pollEvents();

            bool escPressed = input.isKeyJustPressed(Key::ESCAPE);

            // ── Logica por estado ─────────────────────────────────────────────
            if (state == GameState::PLAYING) {
                elapsedTime += dt; // timer so avanca em gameplay activo

                if (escPressed) {
                    state   = GameState::PAUSED;
                    menuSel = 0;
                    glfwSetWindowTitle(win.handle(),
                        "ASCENDENDO | PAUSA | A/D navegar  ESPACO confirmar  ESC continuar");
                } else {
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

                        bool recorded = logic::recordRun(RUNS_CSV_PATH, CAMPAIGN_NAME,
                                                         campaignID, elapsedTime);
                        std::cout
                            << "\n============================================\n"
                            << "  ASCENDENDO -- FIM DA CAMPANHA\n"
                            << "  Tempo:         " << logic::formatElapsed(elapsedTime) << "\n"
                            << "  Campaign ID:   " << campaignID << "\n"
                            << "  Registo:       " << (recorded ? "guardado em " + RUNS_CSV_PATH
                                                                 : "FALHOU (verificar permissoes)") << "\n"
                            << "  Autor:         Rafael Gomes Bernardo\n"
                            << "  Auxiliado por: Claude (Anthropic)\n"
                            << "                 Gemini (Google)\n"
                            << "============================================\n"
                            << "  Pressiona ESPACO para continuar\n\n";

                        creditsReturnState = GameState::MENU;
                        state = GameState::CREDITS;
                        glfwSetWindowTitle(win.handle(),
                            "ASCENDENDO | Creditos | ESPACO para continuar");
                    }
                }

            } else if (state == GameState::PAUSED) {
                if (escPressed) {
                    // ESC de novo: retoma o jogo (timer continua de onde ficou —
                    // nao foi incrementado enquanto estava em PAUSED).
                    state = GameState::PLAYING;
                    glfwSetWindowTitle(win.handle(), "ASCENDENDO");
                } else {
                    if (input.isKeyJustPressed(Key::LEFT))  navigate(-1);
                    if (input.isKeyJustPressed(Key::RIGHT)) navigate(+1);

                    if (input.isKeyJustPressed(Key::SPACE)) {
                        if (menuSel == 0) {           // CONTINUAR
                            state = GameState::PLAYING;
                            glfwSetWindowTitle(win.handle(), "ASCENDENDO");
                        } else if (menuSel == 1) {    // CREDITOS
                            creditsReturnState = GameState::PAUSED;
                            state = GameState::CREDITS;
                        } else {                        // SAIR
                            break;
                        }
                    }
                }

            } else if (state == GameState::CREDITS) {
                // SPACE ou ESC regressam ao estado que chamou os creditos.
                if (input.isKeyJustPressed(Key::SPACE) || escPressed) {
                    state   = creditsReturnState;
                    menuSel = 0;
                    if (state == GameState::MENU) {
                        glfwSetWindowTitle(win.handle(),
                            "ASCENDENDO | A/D navegar  ESPACO confirmar");
                    } else { // PAUSED
                        glfwSetWindowTitle(win.handle(),
                            "ASCENDENDO | PAUSA | A/D navegar  ESPACO confirmar  ESC continuar");
                    }
                }

            } else if (state == GameState::MENU) {
                // ESC no menu de fim-de-run nao faz nada especial (nao ha
                // gameplay para retomar — usar SAIR explicitamente).
                if (input.isKeyJustPressed(Key::LEFT))  navigate(-1);
                if (input.isKeyJustPressed(Key::RIGHT)) navigate(+1);

                if (input.isKeyJustPressed(Key::SPACE)) {
                    if (menuSel == 0) {           // COMECAR
                        resetGame();
                    } else if (menuSel == 1) {    // CREDITOS
                        creditsReturnState = GameState::MENU;
                        state = GameState::CREDITS;
                    } else {                        // SAIR
                        break;
                    }
                }
            }

            // ── Renderizacao ──────────────────────────────────────────────────
            if (!renderer.drawFrame(player, camera, &level, state, menuSel, elapsedTime))
                break;
        }

        vkDeviceWaitIdle(ctx.device());
    }

    glfwTerminate();
    std::cout << "[ASCENDENDO] Motor encerrado com sucesso.\n";
    return 0;
}
