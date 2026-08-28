// =============================================================================
//  ASCENDENDO — Entry Point
//
//  Process/bootstrap and frame composition. Runtime session policy lives in
//  logic::GameSession; graphics/presentation ownership lives in dedicated
//  runtimes. Runtime data bootstrap lives in core::RuntimeBootstrap.
// =============================================================================
#include "Game/Graphics/GraphicsRuntime.h"
#include "Game/Graphics/PresentationRuntime.h"
#include "Game/Graphics/Camera.h"
#include "Game/Graphics/RenderState.h"
#include "Game/Graphics/RenderSnapshot.h"
#include "Game/Graphics/RenderSnapshotBuilder.h"
#include "Game/Logic/EditorRenderSnapshot.h"
#include "Game/Logic/GameSession.h"
#include "Game/Logic/InputManager.h"
#include "Game/Logic/RunHistory.h"
#include "Game/Core/RuntimeBootstrap.h"
#include "Game/Core/Config.h"
#include "Game/Core/KeyBindings.h"

#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>

using namespace gfx;
using namespace logic;

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
    glfwSetWindowTitle(window,
        "ASCENDENDO | MENU | A/D navegar  ESPACO confirmar  E editor  Q sair");
}

void setPlayingTitle(GLFWwindow* window) {
    glfwSetWindowTitle(window,
        "ASCENDENDO | E editor  Q voltar ao menu  ESC pausa");
}

void setEditorTitle(GLFWwindow* window) {
    glfwSetWindowTitle(window,
        "ASCENDENDO | EDITOR | G STAMP/DRAG  [/] tamanho  ESC voltar");
}

void setCreditsTitle(GLFWwindow* window) {
    glfwSetWindowTitle(window,
        "ASCENDENDO | Creditos | ESPACO para continuar");
}

void setPausedTitle(GLFWwindow* window) {
    glfwSetWindowTitle(window,
        "ASCENDENDO | PAUSA | A/D navegar  ESPACO confirmar  Q menu  ESC continuar");
}

void applyStatePresentation(GLFWwindow* window,
                           GameState state,
                           GameState previousState,
                           Camera& camera) {
    if (state == GameState::EDITOR && previousState != GameState::EDITOR) {
        camera = gfx::Camera{};
        setEditorTitle(window);
    } else if (state == GameState::PLAYING && previousState == GameState::MENU) {
        camera = gfx::Camera{};
        setPlayingTitle(window);
    } else if (state == GameState::MENU) {
        setMenuTitle(window);
    } else if (state == GameState::PAUSED) {
        setPausedTitle(window);
    } else if (state == GameState::CREDITS) {
        setCreditsTitle(window);
    } else if (state == GameState::PLAYING) {
        setPlayingTitle(window);
    }
}

RenderState toRenderState(GameState state) {
    switch (state) {
        case GameState::PLAYING: return RenderState::PLAYING;
        case GameState::PAUSED:  return RenderState::PAUSED;
        case GameState::CREDITS: return RenderState::CREDITS;
        case GameState::MENU:    return RenderState::MENU;
        case GameState::EDITOR:  return RenderState::EDITOR;
    }
    return RenderState::MENU;
}

} // namespace

int main(int argc, char** argv) {
    std::cout << "[ASCENDENDO] A iniciar motor...\n";

    GlfwRuntime glfw;
    if (!glfw.initialized()) {
        std::cerr << "[ERRO] GLFW nao conseguiu inicializar.\n";
        return -1;
    }

    const core::RuntimeBootstrapResult bootstrap =
        core::RuntimeBootstrap::fromProcess(
            argc > 0 ? argv[0] : nullptr);

    if (!bootstrap.userDirectoriesReady) {
        std::cerr << "[AVISO] Nao foi possivel preparar completamente o diretorio de dados do utilizador.\n";
    }

    {
        GraphicsRuntime graphics;
        Window& win = graphics.window();
        VulkanContext& ctx = graphics.context();
        Swapchain& swapchain = graphics.swapchain();
        RenderPass& renderPass = graphics.renderPass();
        RendererFacade& renderer = graphics.renderer();
        PresentationRuntime presentation;
        InputManager input;
        core::KeyBindings bindings;

        const auto primaryMonitor = glfwGetPrimaryMonitor();
        const auto mode = primaryMonitor ? glfwGetVideoMode(primaryMonitor) : nullptr;
        const int screenWidth = mode ? mode->width : 1280;
        const int screenHeight = mode ? mode->height : 720;
        if (!mode) {
            std::cerr << "[AVISO] Nao foi possivel obter o monitor primario; a usar 1280x720.\n";
        }

        if (!graphics.init(screenWidth, screenHeight, "ASCENDENDO")) {
            std::cerr << "[ERRO] Nao foi possivel inicializar o subsistema grafico.\n";
            return -1;
        }

        if (!presentation.init(&ctx, &swapchain, &renderPass,
                               &renderer, bootstrap.playerSprite())) {
            std::cerr << "[ERRO] Nao foi possivel inicializar o subsistema de apresentacao.\n";
            return -1;
        }

        if (presentation.textReady()) {
            std::cout << "[ASCENDENDO] Fonte TTF carregada (texto real em CREDITOS/MENU/PAUSA).\n";
        } else {
            std::cout << "[ASCENDENDO] Fonte TTF nao disponivel -- a usar BitmapFont (fallback).\n";
        }

        if (presentation.spriteReady()) {
            std::cout << "[ASCENDENDO] Sprite do jogador carregado ("
                      << presentation.playerSprite().width() << "x"
                      << presentation.playerSprite().height() << ").\n";
        } else {
            std::cout << "[ASCENDENDO] Sprite do jogador nao disponivel -- a usar rectangulo (fallback).\n";
        }

        input.registerWithWindow(win.handle());

        if (bindings.loadFromFile(bootstrap.controlsFile().string())) {
            std::cout << "[ASCENDENDO] Controlos carregados de "
                      << bootstrap.controlsFile().string() << ".\n";
        } else {
            std::cout << "[ASCENDENDO] " << bootstrap.controlsFile().string()
                      << " nao encontrado -- a usar controlos por omissao.\n";
        }

        std::cout << "[ASCENDENDO] Campaign ID: "
                  << (bootstrap.campaignID.empty() ? "(indisponivel)" : bootstrap.campaignID)
                  << "\n";

        GameSession session(
            bootstrap.campaign,
            bootstrap.campaignID,
            bootstrap.runsFile().string());
        Camera camera;

        setMenuTitle(win.handle());
        auto lastTime = std::chrono::high_resolution_clock::now();
        std::cout << "[ASCENDENDO] MENU: A/D navegar | ESPACO confirmar | E editor | Q sair\n";

        while (!win.shouldClose()) {
            const auto now = std::chrono::high_resolution_clock::now();
            const float dt = std::chrono::duration<float>(now - lastTime).count();
            lastTime = now;

            input.beginFrame();
            win.pollEvents();

            const GameState previousState = session.state();
            const GameSessionUpdateResult result = session.update(
                dt, input, bindings,
                static_cast<int32_t>(win.width()),
                static_cast<int32_t>(win.height()),
                static_cast<float>(config::LOGICAL_WIDTH),
                static_cast<float>(config::LOGICAL_HEIGHT));
            const GameState currentState = session.state();

            if (previousState == GameState::PLAYING && currentState == GameState::PLAYING) {
                camera.follow(session.player().position(), dt);
            }

            if (result.campaignCompleted) {
                std::cout
                    << "\n============================================\n"
                    << "  ASCENDENDO -- FIM DA CAMPANHA\n"
                    << "  Tempo:         " << logic::formatElapsed(result.completionElapsedSeconds) << "\n"
                    << "  Campaign ID:   " << session.campaignID() << "\n"
                    << "  Registo:       " << (result.runRecorded ? "guardado em " + bootstrap.runsFile().string()
                                                         : "FALHOU (verificar permissoes)") << "\n"
                    << "  Autor:         Rafael Gomes Bernardo\n"
                    << "  Auxiliado por: Claude (Anthropic)\n"
                    << "                 Gemini (Google)\n"
                    << "============================================\n"
                    << "  Pressiona ESPACO para continuar\n\n";
            }

            if (result.stateChanged) {
                applyStatePresentation(win.handle(), currentState, previousState, camera);
            }

            if (result.quitRequested) {
                break;
            }

            RenderSnapshot renderSnapshot;
            logic::EditorRenderSnapshot editorSnapshot;
            if (currentState == GameState::PLAYING || currentState == GameState::PAUSED) {
                renderSnapshot = buildRenderSnapshot(session.player(), session.level());
                renderer.attachEditorSnapshot(nullptr);
            } else if (currentState == GameState::EDITOR) {
                editorSnapshot = session.editorSession().renderSnapshot();
                renderer.attachEditorSnapshot(&editorSnapshot);
            } else {
                renderer.attachEditorSnapshot(nullptr);
            }

            if (!renderer.drawFrame(renderSnapshot, camera, toRenderState(currentState),
                                    session.menuSelection(), session.elapsedTime())) {
                std::cerr << "[ERRO] Renderer falhou ao desenhar o estado atual.\n";
                break;
            }
        }

        vkDeviceWaitIdle(ctx.device());
    }

    std::cout << "[ASCENDENDO] Motor encerrado com sucesso.\n";
    return 0;
}
