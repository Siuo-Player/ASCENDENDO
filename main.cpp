// =============================================================================
//  ASCENDENDO — Entry Point
//
//  Runtime state machine: MENU / PLAYING / PAUSED / CREDITS / EDITOR.
//  The entry point owns process lifetime; application helpers own setup/data
//  transformation so this file stays small and cohesive.
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
#include "Game/Graphics/RendererFacade.h"
#include "Game/Graphics/Camera.h"
#include "Game/Core/ApplicationHelpers.h"
#include "Game/Core/ApplicationGraphics.h"
#include "Game/Logic/InputManager.h"
#include "Game/Logic/Player.h"
#include "Game/Logic/Physics.h"
#include "Game/Logic/Level.h"
#include "Game/Logic/RunHistory.h"
#include "Game/Logic/EditorSession.h"
#include "Game/Core/Config.h"
#include "Game/Core/CampaignID.h"
#include "Game/Core/GameAction.h"
#include "Game/Core/KeyBindings.h"
#include "Game/Core/Viewport.h"

#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <string>
#include <vector>

using namespace gfx;
using namespace logic;

static const std::string CAMPAIGN_NAME = "Campanha Principal";
static const std::string LEVELS_DIR = "Game/Assets/Levels";
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
    const int screenWidth = mode ? mode->width : 1280;
    const int screenHeight = mode ? mode->height : 720;
    if (!mode) {
        std::cerr << "[AVISO] Nao foi possivel obter o monitor primario; a usar 1280x720.\n";
    }

    {
        Window win;
        VulkanContext ctx;
        Swapchain swapchain;
        RenderPass renderPass;
        Pipeline pipeline;
        TextPipeline textPipeline;
        FontRenderer font;
        SpritePipeline spritePipeline;
        SpriteRenderer playerSprite;
        RendererFacade renderer;
        InputManager input;
        core::KeyBindings bindings;

        if (!app::initializeGraphics(
                win, ctx, swapchain, renderPass, pipeline,
                textPipeline, font, spritePipeline, playerSprite,
                renderer, screenWidth, screenHeight)) {
            return -1;
        }

        input.registerWithWindow(win.handle());

        if (bindings.loadFromFile(CONTROLS_CFG_PATH)) {
            std::cout << "[ASCENDENDO] Controlos carregados de " << CONTROLS_CFG_PATH << ".\n";
        } else {
            std::cout << "[ASCENDENDO] " << CONTROLS_CFG_PATH
                      << " nao encontrado -- a usar controlos por omissao.\n";
        }

        const std::vector<std::string> campaign = app::loadCampaignLevels(LEVELS_DIR);

        std::string campaignID = core::computeCampaignID(LEVELS_DIR);
        std::cout << "[ASCENDENDO] Campaign ID: "
                  << (campaignID.empty() ? "(indisponivel)" : campaignID) << "\n";

        Level level;
        PhysicsWorld world;
        Camera camera;
        Player player;
        EditorSession editorSession(campaign.size() <= 1);

        int currentLevelIndex = 0;
        float currentSpawnY = 0.0f;
        GameState state = GameState::MENU;
        GameState editorReturnState = GameState::MENU;
        int menuSel = 0;
        float elapsedTime = 0.0f;
        GameState creditsReturnState = GameState::MENU;

        auto resetGame = [&]() {
            player = logic::Player{};
            player.body.position = {config::LOGICAL_WIDTH / 2.0f, 40.0f};
            camera = gfx::Camera{};
            world = logic::PhysicsWorld{};
            level.clear();
            currentLevelIndex = 0;
            currentSpawnY = 0.0f;
            elapsedTime = 0.0f;

            if (!campaign.empty()) {
                currentSpawnY = level.appendFromFile(
                    campaign[0], config::LOGICAL_WIDTH, 0.0f);
                currentLevelIndex = 1;
            }

            state = GameState::PLAYING;
            menuSel = 0;
            app::setPlayingTitle(win.handle());
        };

        auto openEditor = [&](GameState returnState) {
            editorReturnState = returnState;
            camera = gfx::Camera{};
            editorSession.cancelInteraction();
            state = GameState::EDITOR;
            menuSel = 0;
            app::setEditorTitle(win.handle());
        };

        auto clickedMenuBox = [&](int count) -> int {
            if (!input.isMouseButtonJustPressed(MouseButton::LEFT)) return -1;
            core::LogicalPoint pt = core::windowToLogical(
                input.cursorX(), input.cursorY(),
                (int32_t)win.width(), (int32_t)win.height(),
                config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT);
            return core::hitTestMenuBox(pt.x, pt.y, count, config::LOGICAL_WIDTH);
        };

        app::setMenuTitle(win.handle());

        auto lastTime = std::chrono::high_resolution_clock::now();
        std::cout << "[ASCENDENDO] MENU: A/D navegar | ESPACO confirmar | E editor | Q sair\n";

        while (!win.shouldClose()) {
            auto now = std::chrono::high_resolution_clock::now();
            float dt = std::chrono::duration<float>(now - lastTime).count();
            lastTime = now;

            input.beginFrame();
            win.pollEvents();

            const bool pausePressed = core::isActionJustPressed(bindings, input, core::GameAction::Pause);
            const bool quitPressed = core::isActionJustPressed(bindings, input, core::GameAction::Quit);
            const bool openEditorPressed = core::isActionJustPressed(bindings, input, core::GameAction::OpenEditor);

            if (state == GameState::PLAYING) {
                elapsedTime += dt;

                if (openEditorPressed) {
                    openEditor(GameState::PLAYING);
                } else if (quitPressed) {
                    editorSession.cancelInteraction();
                    state = GameState::MENU;
                    menuSel = 0;
                    app::setMenuTitle(win.handle());
                } else if (pausePressed) {
                    state = GameState::PAUSED;
                    menuSel = 0;
                    glfwSetWindowTitle(win.handle(),
                        "ASCENDENDO | PAUSA | A/D navegar  ESPACO confirmar  Q menu  ESC continuar");
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
                        const bool recorded = logic::recordRun(
                            RUNS_CSV_PATH, CAMPAIGN_NAME, campaignID, elapsedTime);

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
                    app::setPlayingTitle(win.handle());
                } else if (quitPressed) {
                    state = GameState::MENU;
                    menuSel = 0;
                    app::setMenuTitle(win.handle());
                } else {
                    int clickedPaused = clickedMenuBox(3);
                    if (clickedPaused >= 0) menuSel = clickedPaused;

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UILeft)) app::navigateMenu(menuSel, -1, 3);
                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIRight)) app::navigateMenu(menuSel, +1, 3);

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) || clickedPaused >= 0) {
                        if (menuSel == 0) {
                            state = GameState::PLAYING;
                            app::setPlayingTitle(win.handle());
                        } else if (menuSel == 1) {
                            creditsReturnState = GameState::PAUSED;
                            state = GameState::CREDITS;
                            glfwSetWindowTitle(win.handle(), "ASCENDENDO | Creditos | ESPACO para continuar");
                        } else {
                            state = GameState::MENU;
                            menuSel = 0;
                            app::setMenuTitle(win.handle());
                        }
                    }
                }

            } else if (state == GameState::CREDITS) {
                if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) || pausePressed) {
                    state = creditsReturnState;
                    menuSel = 0;
                    if (state == GameState::MENU) app::setMenuTitle(win.handle());
                    else if (state == GameState::PLAYING) app::setPlayingTitle(win.handle());
                    else glfwSetWindowTitle(win.handle(),
                        "ASCENDENDO | PAUSA | A/D navegar  ESPACO confirmar  Q menu  ESC continuar");
                }

            } else if (state == GameState::MENU) {
                if (quitPressed) {
                    break;
                }

                if (openEditorPressed) {
                    openEditor(GameState::MENU);
                } else {
                    int clickedMenu = clickedMenuBox(4);
                    if (clickedMenu >= 0) menuSel = clickedMenu;

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UILeft)) app::navigateMenu(menuSel, -1, 4);
                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIRight)) app::navigateMenu(menuSel, +1, 4);

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) || clickedMenu >= 0) {
                        if (menuSel == 0) {
                            resetGame();
                        } else if (menuSel == 1) {
                            openEditor(GameState::MENU);
                        } else if (menuSel == 2) {
                            creditsReturnState = GameState::MENU;
                            state = GameState::CREDITS;
                            glfwSetWindowTitle(win.handle(), "ASCENDENDO | Creditos | ESPACO para continuar");
                        } else {
                            break;
                        }
                    }
                }

            } else if (state == GameState::EDITOR) {
                if (pausePressed) {
                    editorSession.cancelInteraction();
                    state = editorReturnState;
                    menuSel = 0;
                    if (state == GameState::PLAYING) app::setPlayingTitle(win.handle());
                    else app::setMenuTitle(win.handle());
                } else {
                    editorSession.update(input, bindings,
                                        (int32_t)win.width(), (int32_t)win.height());
                }
            }

            const gfx::RenderSnapshot renderSnapshot = app::buildRenderSnapshot(player, level);
            if (state == GameState::EDITOR) {
                const logic::EditorRenderSnapshot editorSnapshot = editorSession.renderSnapshot();
                renderer.attachEditorSnapshot(&editorSnapshot);
            } else {
                renderer.attachEditorSnapshot(nullptr);
            }

            if (!renderer.drawFrame(renderSnapshot, camera, state, menuSel, elapsedTime)) {
                std::cerr << "[ERRO] Renderer falhou ao desenhar o estado atual.\n";
                break;
            }
        }

        vkDeviceWaitIdle(ctx.device());
    }

    glfwTerminate();
    std::cout << "[ASCENDENDO] Motor encerrado com sucesso.\n";
    return 0;
}
