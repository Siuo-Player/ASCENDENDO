// =============================================================================
//  ASCENDENDO — Entry Point
//
//  Runtime state machine: MENU / PLAYING / PAUSED / CREDITS / EDITOR.
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
#include "Game/Logic/InputManager.h"
#include "Game/Logic/Player.h"
#include "Game/Logic/Physics.h"
#include "Game/Logic/SimulationOrchestrator.h"
#include "Game/Logic/Level.h"
#include "Game/Logic/RunHistory.h"
#include "Game/Logic/EditorSession.h"
#include "Game/Logic/CampaignRuntime.h"
#include "Game/Core/Config.h"
#include "Game/Core/CampaignID.h"
#include "Game/Core/CampaignLoader.h"
#include "Game/Core/GameAction.h"
#include "Game/Core/GameStateMachine.h"
#include "Game/Core/KeyBindings.h"
#include "Game/Core/Viewport.h"
#include "Game/Core/RuntimePaths.h"

#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <vector>
#include <string>

using namespace gfx;
using namespace logic;

static const std::string CAMPAIGN_NAME = "Campanha Principal";

namespace {

class GlfwRuntime {
public:
    GlfwRuntime() : initialized_(glfwInit() == GLFW_TRUE) {}

    ~GlfwRuntime() {
        if (initialized_) glfwTerminate();
    }

    GlfwRuntime(const GlfwRuntime&) = delete;
    GlfwRuntime& operator=(const GlfwRuntime&) = delete;

    bool initialized() const { return initialized_; }

private:
    bool initialized_ = false;
};

void setMenuTitle(GLFWwindow* window) {
    glfwSetWindowTitle(window, "ASCENDENDO | MENU | A/D navegar  ESPACO confirmar  E editor  Q sair");
}

void setPlayingTitle(GLFWwindow* window) {
    glfwSetWindowTitle(window, "ASCENDENDO | E editor  Q voltar ao menu  ESC pausa");
}

void setEditorTitle(GLFWwindow* window) {
    glfwSetWindowTitle(window, "ASCENDENDO | EDITOR | G STAMP/DRAG  [/] tamanho  ESC voltar");
}

} // namespace

int main(int argc, char** argv) {
    std::cout << "[ASCENDENDO] A iniciar motor...\n";

    GlfwRuntime glfw;
    if (!glfw.initialized()) {
        std::cerr << "[ERRO] GLFW nao conseguiu inicializar.\n";
        return -1;
    }

    const core::RuntimePaths runtimePaths =
        core::RuntimePaths::fromProcess(argc > 0 ? argv[0] : nullptr);
    if (!runtimePaths.ensureUserDirectories()) {
        std::cerr << "[AVISO] Nao foi possivel preparar completamente o diretorio de dados do utilizador.\n";
    }

    const std::string levelsDir = runtimePaths.levelsRoot().string();
    const std::string runsCsvPath = runtimePaths.runsFile().string();
    const std::string controlsCfgPath = runtimePaths.controlsFile().string();
    const std::string playerSpritePath = runtimePaths.playerSprite().string();

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

        if (textPipeline.init(&ctx, &swapchain, &renderPass) &&
            font.init(&ctx, textPipeline.descriptorSetLayout())) {
            renderer.attachText(&textPipeline, &font);
            std::cout << "[ASCENDENDO] Fonte TTF carregada (texto real em CREDITOS/MENU/PAUSA).\n";
        } else {
            std::cout << "[ASCENDENDO] Fonte TTF nao disponivel -- a usar BitmapFont (fallback).\n";
        }

        if (spritePipeline.init(&ctx, &swapchain, &renderPass) &&
            playerSprite.init(&ctx, spritePipeline.descriptorSetLayout(), playerSpritePath)) {
            renderer.attachSprite(&spritePipeline, &playerSprite);
            std::cout << "[ASCENDENDO] Sprite do jogador carregado ("
                      << playerSprite.width() << "x" << playerSprite.height() << ").\n";
        } else {
            std::cout << "[ASCENDENDO] Sprite do jogador nao disponivel -- a usar rectangulo (fallback).\n";
        }

        input.registerWithWindow(win.handle());

        if (bindings.loadFromFile(controlsCfgPath)) {
            std::cout << "[ASCENDENDO] Controlos carregados de " << controlsCfgPath << ".\n";
        } else {
            std::cout << "[ASCENDENDO] " << controlsCfgPath
                      << " nao encontrado -- a usar controlos por omissao.\n";
        }

        const std::vector<std::filesystem::path> campaign =
            core::CampaignLoader::load(
                runtimePaths.campaignFile(),
                runtimePaths.levelsRoot());
        const CampaignRuntime campaignRuntime(campaign);

        std::string campaignID = core::computeCampaignID(levelsDir);
        std::cout << "[ASCENDENDO] Campaign ID: "
                  << (campaignID.empty() ? "(indisponivel)" : campaignID) << "\n";

        Level level;
        PhysicsWorld world;
        SimulationOrchestrator simulation;
        Camera camera;
        Player player;
        EditorSession editorSession(campaign.size() <= 1);
        renderer.attachEditorSession(&editorSession);
        core::GameStateMachine stateMachine;

        float elapsedTime = 0.0f;

        auto resetGame = [&]() {
            player = logic::Player{};
            player.body.position = {config::LOGICAL_WIDTH / 2.0f, 40.0f};
            camera = gfx::Camera{};
            world = logic::PhysicsWorld{};
            level.clear();
            elapsedTime = 0.0f;

            CampaignRuntime& runtime = const_cast<CampaignRuntime&>(campaignRuntime);
            runtime.loadInitialLevel(level, config::LOGICAL_WIDTH);

            stateMachine.enterPlaying();
            setPlayingTitle(win.handle());
        };

        auto openEditor = [&](GameState returnState) {
            camera = gfx::Camera{};
            editorSession.cancelInteraction();
            stateMachine.enterEditor(returnState);
            setEditorTitle(win.handle());
        };

        auto navigate = [&](int delta, int count) {
            stateMachine.selectRelative(delta, count);
        };

        auto clickedMenuBox = [&](int count) -> int {
            if (!input.isMouseButtonJustPressed(MouseButton::LEFT)) return -1;
            core::LogicalPoint pt = core::windowToLogical(
                input.cursorX(), input.cursorY(),
                (int32_t)win.width(), (int32_t)win.height(),
                config::LOGICAL_WIDTH, config::LOGICAL_HEIGHT);
            return core::hitTestMenuBox(pt.x, pt.y, count, config::LOGICAL_WIDTH);
        };

        setMenuTitle(win.handle());

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
            const GameState state = stateMachine.state();

            if (state == GameState::PLAYING) {
                elapsedTime += dt;

                if (openEditorPressed) {
                    openEditor(GameState::PLAYING);
                } else if (quitPressed) {
                    editorSession.cancelInteraction();
                    stateMachine.returnToMenu();
                    setMenuTitle(win.handle());
                } else if (pausePressed) {
                    stateMachine.pause();
                    glfwSetWindowTitle(win.handle(),
                        "ASCENDENDO | PAUSA | A/D navegar  ESPACO confirmar  Q menu  ESC continuar");
                } else {
                    simulation.advance(dt, input, player, world, level);

                    camera.follow(player.position(), dt);

                    CampaignRuntime& runtime = const_cast<CampaignRuntime&>(campaignRuntime);
                    if (player.position().y > runtime.currentSpawnY() - config::LOGICAL_HEIGHT) {
                        runtime.streamNextLevel(level, config::LOGICAL_WIDTH);
                    }

                    if (level.hasFlag &&
                        PhysicsWorld::collides(player.body.bounds(), level.flagBounds)) {
                        const bool recorded = logic::recordRun(
                            runsCsvPath, CAMPAIGN_NAME, campaignID, elapsedTime);

                        std::cout
                            << "\n============================================\n"
                            << "  ASCENDENDO -- FIM DA CAMPANHA\n"
                            << "  Tempo:         " << logic::formatElapsed(elapsedTime) << "\n"
                            << "  Campaign ID:   " << campaignID << "\n"
                            << "  Registo:       " << (recorded ? "guardado em " + runsCsvPath
                                                                 : "FALHOU (verificar permissoes)") << "\n"
                            << "  Autor:         Rafael Gomes Bernardo\n"
                            << "  Auxiliado por: Claude (Anthropic)\n"
                            << "                 Gemini (Google)\n"
                            << "============================================\n"
                            << "  Pressiona ESPACO para continuar\n\n";

                        stateMachine.enterCredits(GameState::MENU);
                        glfwSetWindowTitle(win.handle(),
                            "ASCENDENDO | Creditos | ESPACO para continuar");
                    }
                }

            } else if (state == GameState::PAUSED) {
                if (pausePressed) {
                    stateMachine.resume();
                    setPlayingTitle(win.handle());
                } else if (quitPressed) {
                    stateMachine.returnToMenu();
                    setMenuTitle(win.handle());
                } else {
                    int clickedPaused = clickedMenuBox(3);
                    if (clickedPaused >= 0) stateMachine.select(clickedPaused, 3);

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UILeft)) navigate(-1, 3);
                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIRight)) navigate(+1, 3);

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) || clickedPaused >= 0) {
                        if (stateMachine.menuSelection() == 0) {
                            stateMachine.resume();
                            setPlayingTitle(win.handle());
                        } else if (stateMachine.menuSelection() == 1) {
                            stateMachine.enterCredits(GameState::PAUSED);
                            glfwSetWindowTitle(win.handle(), "ASCENDENDO | Creditos | ESPACO para continuar");
                        } else {
                            stateMachine.returnToMenu();
                            setMenuTitle(win.handle());
                        }
                    }
                }

            } else if (state == GameState::CREDITS) {
                if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) || pausePressed) {
                    stateMachine.returnFromCredits();
                    if (stateMachine.state() == GameState::MENU) setMenuTitle(win.handle());
                    else if (stateMachine.state() == GameState::PLAYING) setPlayingTitle(win.handle());
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
                    if (clickedMenu >= 0) stateMachine.select(clickedMenu, 4);

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UILeft)) navigate(-1, 4);
                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIRight)) navigate(+1, 4);

                    if (core::isActionJustPressed(bindings, input, core::GameAction::UIConfirm) || clickedMenu >= 0) {
                        if (stateMachine.menuSelection() == 0) {
                            resetGame();
                        } else if (stateMachine.menuSelection() == 1) {
                            openEditor(GameState::MENU);
                        } else if (stateMachine.menuSelection() == 2) {
                            stateMachine.enterCredits(GameState::MENU);
                            glfwSetWindowTitle(win.handle(), "ASCENDENDO | Creditos | ESPACO para continuar");
                        } else {
                            break;
                        }
                    }
                }

            } else if (state == GameState::EDITOR) {
                if (pausePressed) {
                    editorSession.cancelInteraction();
                    stateMachine.returnFromEditor();
                    if (stateMachine.state() == GameState::PLAYING) setPlayingTitle(win.handle());
                    else setMenuTitle(win.handle());
                } else {
                    editorSession.update(input, bindings,
                                        (int32_t)win.width(), (int32_t)win.height());
                }
            }

            if (!renderer.drawFrame(player, camera, &level, stateMachine.state(),
                                    stateMachine.menuSelection(), elapsedTime)) {
                std::cerr << "[ERRO] Renderer falhou ao desenhar o estado atual.\n";
                break;
            }
        }

        vkDeviceWaitIdle(ctx.device());
    }

    std::cout << "[ASCENDENDO] Motor encerrado com sucesso.\n";
    return 0;
}
