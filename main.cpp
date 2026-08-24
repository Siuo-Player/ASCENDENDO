// =============================================================================
//  ASCENDENDO — Entry Point
//
//  @version 9.3
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
//    v9.1  — KeyBindings (Fase 9.1): Pause/UIConfirm/UILeft/UIRight/Quit
//             deixam de verificar Key::X directamente e passam por
//             core::isActionJustPressed(bindings, input, GameAction::X).
//             Nova accao Quit (default Key::Q) sai do jogo directamente a
//             partir de PAUSED/MENU, sem precisar navegar ate "Sair".
//             Carrega Development/Settings/controls.cfg se existir; senao
//             usa os defaults (identicos ao comportamento anterior a este
//             sistema existir). MoveLeft/MoveRight/Jump NAO estao ligados
//             ainda -- Player.cpp continua a usar isLeft()/isRight()/
//             isKeyDown(Key::SPACE) directamente (ver nota em KeyBindings.h).
//    v9.2  — Rato (Fase 9.2): PAUSED e MENU aceitam clique esquerdo nas 3
//             caixas (clickedMenuBox(), Core/Viewport.h) -- clique
//             seleciona E confirma na mesma accao, tal como um botao
//             normal. Geometria das caixas espelha EXACTAMENTE as
//             constantes inline em Renderer.cpp (nao foi tocado). CREDITS
//             continua so' por teclado (nao pedido; facil de estender).
//    v9.3  — GameState::EDITOR (Fase 9.3): infra-estrutura + acesso.
//             navigate()/clickedMenuBox() generalizados por `count` (MENU
//             passa a ter 4 opcoes -- ganhou EDITOR -- PAUSED continua com
//             3). Acesso duplo: tecla dedicada (OpenEditor, default E) OU
//             opcao visivel no menu -- os dois, como pedido. Dentro do
//             EDITOR: camara livre (MoveLeft/MoveRight para X, novos
//             EditorPanUp/EditorPanDown para Y -- W/S por omissao), sem
//             fisica (Player/PhysicsWorld simplesmente nao sao chamados
//             neste estado). ESC (Pause) regressa a MENU.
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
#include "Game/Core/GameAction.h"
#include "Game/Core/KeyBindings.h"
#include "Game/Core/Viewport.h"

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
static const std::string CONTROLS_CFG_PATH = "Development/Settings/controls.cfg";

int main() {
    std::cout << "[ASCENDENDO] A iniciar motor...\n";

    if (!glfwInit()) {
        std::cerr << "[ERRO] GLFW nao conseguiu inicializar.\n";
        return -1;
    }

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = primaryMonitor ? glfwGetVideoMode(primaryMonitor) : nullptr;
    // Nao assumir que existe monitor primario: ambientes sem monitor/WM
    // (remote desktop, CI, algumas VMs) podem devolver nullptr.
    const int screenWidth  = mode ? mode->width  : 1280;
    const int screenHeight = mode ? mode->height : 720;
    if (!mode) {
        std::cerr << "[AVISO] Nao foi possivel obter o monitor primario; a usar 1280x720.\n";
    }

    {
        Window         win;
        VulkanContext  ctx;
        Swapchain      swapchain;
        RenderPass     renderPass;
        Pipeline       pipeline;
        TextPipeline   textPipeline;
        FontRenderer   font;
        SpritePipeline spritePipeline;
        SpriteRenderer playerSprite;
        Renderer       renderer;
        InputManager   input;
        core::KeyBindings bindings;

        if (!win.create(screenWidth, screenHeight, "ASCENDENDO")) {
            std::cerr << "[ERRO] Nao foi possivel criar a janela GLFW.\n";
            return -1;
        }

        std::vector<const char*> exts;
        win.appendRequiredExtensions(exts);
        if (!ctx.init(false, exts)) {
            std::cerr << "[ERRO] Nao foi possivel inicializar Vulkan.\n";
            return -1;
        }

        VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
        if (surface == VK_NULL_HANDLE) {
            std::cerr << "[ERRO] Nao foi possivel criar a surface Vulkan.\n";
            return -1;
        }
        if (!ctx.createSurface(surface)) {
            std::cerr << "[ERRO] Nao foi possivel associar a surface Vulkan ao contexto.\n";
            // Se createSurface() falhar, o handle ainda nao e propriedade do
            // contexto; evitar tentar usa-lo com uma swapchain inexistente.
            return -1;
        }

        if (!swapchain.init(&ctx, &win)) {
            std::cerr << "[ERRO] Nao foi possivel inicializar o swapchain Vulkan.\n";
            return -1;
        }
        if (!renderPass.init(&ctx, &swapchain)) {
            std::cerr << "[ERRO] Nao foi possivel criar o render pass Vulkan.\n";
            return -1;
        }
        if (!pipeline.init(&ctx, &swapchain, &renderPass)) {
            std::cerr << "[ERRO] Nao foi possivel criar a pipeline grafica.\n";
            return -1;
        }
        if (!renderer.init(&ctx, &swapchain, &renderPass, &pipeline)) {
            std::cerr << "[ERRO] Nao foi possivel inicializar o renderer.\n";
            return -1;
        }

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

        // Controlos: se Development/Settings/controls.cfg existir, usa-o;
        // senao mantem os defaults (identicos ao comportamento do jogo
        // antes deste sistema existir). Mesmo padrao de falha graciosa
        // usado para a fonte TTF e o sprite do jogador.
        if (bindings.loadFromFile(CONTROLS_CFG_PATH)) {
            std::cout << "[ASCENDENDO] Controlos carregados de " << CONTROLS_CFG_PATH << ".\n";
        } else {
            std::cout << "[ASCENDENDO] " << CONTROLS_CFG_PATH
                      << " nao encontrado -- a usar controlos por omissao.\n";
        }

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
        GameState state              = GameState::PLAYING;
        int       menuSel            = 0;
        float     elapsedTime        = 0.0f;
        GameState creditsReturnState = GameState::MENU;

        // ── Funcao de reset / (re)inicio ─────────────────────────────────────
        auto resetGame = [&]() {
            player              = logic::Player{};
            player.body.position = { config::LOGICAL_WIDTH / 2.0f, 40.0f };
            camera              = gfx::Camera{};
            world               = logic::PhysicsWorld{};
            level.clear();
            currentLevelIndex   = 0;
            currentSpawnY       = 0.0f;
            elapsedTime         = 0.0f;

            if (!campaign.empty()) {
                currentSpawnY = level.appendFromFile(
                    campaign[0], config::LOGICAL_WIDTH, 0.0f);
                currentLevelIndex = 1;
            }

            state   = GameState::PLAYING;
            menuSel = 0;
            glfwSetWindowTitle(win.handle(), "ASCENDENDO");
        };

        auto navigate = [&](int delta, int count) {
            menuSel = (menuSel + delta + count) % count;
        };

        auto clickedMenuBox = [&](int count) -> int {
            if (!input.isMouseButtonJustPressed(MouseButton::LEFT)) return -1;
            core::LogicalPoint pt = core::windowToLogical(
                input.cursorX(), input.cursorY(),
                (int32_t)win.width(), (int32_t)win.height(),
                config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT);
            return core::hitTestMenuBox(pt.x, pt.y, count, config::LOGICAL_WIDTH);
        };

        resetGame();

        auto lastTime = std::chrono::high_resolution_clock::now();
        std::cout << "[ASCENDENDO] A/D = mover | SPACE = saltar | ESC = pausa\n";

        while (!win.shouldClose()) {
            auto  now = std::chrono::high_resolution_clock::now();
            float dt  = std::chrono::duration<float>(now - lastTime).count();
            lastTime  = now;

            input.beginFrame();
            win.pollEvents();

            bool pausePressed = core::isActionJustPressed(bindings, input, core::GameAction::Pause);

            if (state == GameState::PLAYING) {
                elapsedTime += dt;

                if (pausePressed) {
                    state   = GameState::PAUSED;
                    menuSel = 0;
                    glfwSetWindowTitle(win.handle(),
                        "ASCENDENDO | PAUSA | A/D navegar  ESPACO confirmar  ESC continuar");
                } else {
                    int steps = world.advance(dt);
                    for (int i = 0; i < steps; ++i) {
                        player.update(input, world, config::FIXED_STEP);
                        level.resolveCollision(player.body);
                    }

                    camera.follow(player.position(), dt);

                    if (player.position().y > currentSpawnY - config::LOGICAL_HEIGHT) {
                        if (static_cast<size_t>(currentLevelIndex) < campaign.size()) {
                            currentSpawnY = level.appendFromFile(
                                campaign[currentLevelIndex],
                                config::LOGICAL_WIDTH, currentSpawnY);
                            currentLevelIndex++;
                        }
                    }

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
                if (pausePressed) {
                    state = GameState::PLAYING;
                    glfwSetWindowTitle(win.handle(), "ASCENDENDO");
                } else {
                    if (core::isActionJustPressed(bindings, input, core::GameAction::Quit)) break;

                    int clickedPaused = clickedMenuBox(3);
                    if (clickedPaused >= 0) menuSel = clickedPaused;

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UILeft))  navigate(-1, 3);
                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIRight)) navigate(+1, 3);

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) || clickedPaused >= 0) {
                        if (menuSel == 0) {
                            state = GameState::PLAYING;
                            glfwSetWindowTitle(win.handle(), "ASCENDENDO");
                        } else if (menuSel == 1) {
                            creditsReturnState = GameState::PAUSED;
                            state = GameState::CREDITS;
                        } else {
                            break;
                        }
                    }
                }

            } else if (state == GameState::CREDITS) {
                if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) || pausePressed) {
                    state   = creditsReturnState;
                    menuSel = 0;
                    if (state == GameState::MENU) {
                        glfwSetWindowTitle(win.handle(),
                            "ASCENDENDO | A/D navegar  ESPACO confirmar");
                    } else {
                        glfwSetWindowTitle(win.handle(),
                            "ASCENDENDO | PAUSA | A/D navegar  ESPACO confirmar  ESC continuar");
                    }
                }

            } else if (state == GameState::MENU) {
                if (core::isActionJustPressed(bindings, input, core::GameAction::Quit)) break;

                if (core::isActionJustPressed(bindings, input, core::GameAction::OpenEditor)) {
                    camera  = gfx::Camera{};
                    state   = GameState::EDITOR;
                    menuSel = 0;
                    glfwSetWindowTitle(win.handle(),
                        "ASCENDENDO | EDITOR | A/D/W/S deslocar  ESC sair");
                } else {
                    int clickedMenu = clickedMenuBox(4);
                    if (clickedMenu >= 0) menuSel = clickedMenu;

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UILeft))  navigate(-1, 4);
                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIRight)) navigate(+1, 4);

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) || clickedMenu >= 0) {
                        if (menuSel == 0) {
                            resetGame();
                        } else if (menuSel == 1) {
                            camera  = gfx::Camera{};
                            state   = GameState::EDITOR;
                            menuSel = 0;
                            glfwSetWindowTitle(win.handle(),
                                "ASCENDENDO | EDITOR | A/D/W/S deslocar  ESC sair");
                        } else if (menuSel == 2) {
                            creditsReturnState = GameState::MENU;
                            state = GameState::CREDITS;
                        } else {
                            break;
                        }
                    }
                }

            } else if (state == GameState::EDITOR) {
                if (core::isActionJustPressed(bindings, input, core::GameAction::Pause)) {
                    state   = GameState::MENU;
                    menuSel = 0;
                    glfwSetWindowTitle(win.handle(),
                        "ASCENDENDO | A/D navegar  ESPACO confirmar");
                } else {
                    float dx = 0.0f, dy = 0.0f;
                    if (core::isActionHeld(bindings, input, core::GameAction::MoveLeft))     dx -= 1.0f;
                    if (core::isActionHeld(bindings, input, core::GameAction::MoveRight))    dx += 1.0f;
                    if (core::isActionHeld(bindings, input, core::GameAction::EditorPanUp))   dy += 1.0f;
                    if (core::isActionHeld(bindings, input, core::GameAction::EditorPanDown)) dy -= 1.0f;

                    camera.position.x += dx * config::EDITOR_CAMERA_PAN_SPEED * dt;
                    camera.position.y += dy * config::EDITOR_CAMERA_PAN_SPEED * dt;
                }
            }

            if (!renderer.drawFrame(player, camera, &level, state, menuSel, elapsedTime))
                break;
        }

        vkDeviceWaitIdle(ctx.device());
    }

    glfwTerminate();
    std::cout << "[ASCENDENDO] Motor encerrado com sucesso.\n";
    return 0;
}
