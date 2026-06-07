#pragma once
// =============================================================================
//  Game/Logic/InputManager.h
//
//  @version 3.1
//  @history
//    v3.1 — criado (key states: down, justPressed, justReleased; callbacks GLFW)
//
//  Sistema de input desacoplado do GLFW:
//  - onKeyEvent() pode ser chamado diretamente nos testes (sem janela real)
//  - registerWithWindow() liga ao callback GLFW em produção
//  - beginFrame() deve ser chamado no início de cada frame (limpa "just" states)
//
//  Uso em jogo:
//    input.registerWithWindow(win.handle());
//    while (!win.shouldClose()) {
//        input.beginFrame();
//        win.pollEvents();         // GLFW chama onKeyEvent() automaticamente
//        if (input.isLeft()) { ... }
//    }
//
//  Uso nos testes (sem GLFW):
//    input.onKeyEvent(Key::SPACE, 1);  // 1 = PRESS
//    CHECK(input.isJump() == true);
// =============================================================================

#include <unordered_map>

struct GLFWwindow;

namespace logic {

// Constantes para as teclas mais usadas (valores == GLFW_KEY_*)
namespace Key {
    constexpr int LEFT   = 263;
    constexpr int RIGHT  = 262;
    constexpr int UP     = 265;
    constexpr int DOWN   = 264;
    constexpr int A      = 65;
    constexpr int D      = 68;
    constexpr int W      = 87;
    constexpr int S      = 83;
    constexpr int SPACE  = 32;
    constexpr int ESCAPE = 256;
}

// Constantes de action GLFW (replicadas para evitar incluir GLFW no header)
namespace Action {
    constexpr int RELEASE = 0;
    constexpr int PRESS   = 1;
    constexpr int REPEAT  = 2;
}

class InputManager {
public:
    InputManager()  = default;
    ~InputManager() = default;

    // ── Ciclo de Frame ────────────────────────────────────────────────────────
    // Chamar NO INICIO de cada frame, ANTES de pollEvents().
    // Limpa isKeyJustPressed e isKeyJustReleased.
    void beginFrame();

    // ── Injecção de Eventos ───────────────────────────────────────────────────
    // Chamado pelo callback GLFW OU diretamente pelos testes.
    void onKeyEvent(int key, int action);

    // Ligar esta instância ao callback de teclado de uma janela GLFW.
    void registerWithWindow(GLFWwindow* window);

    // ── Consultas de Estado ───────────────────────────────────────────────────
    bool isKeyDown(int key)         const;
    bool isKeyJustPressed(int key)  const;
    bool isKeyJustReleased(int key) const;

    // ── Conveniência para o Jogo ──────────────────────────────────────────────
    bool isLeft()  const;  // A ou LEFT_ARROW
    bool isRight() const;  // D ou RIGHT_ARROW
    bool isJump()  const;  // SPACE (premido = a carregar o salto)

private:
    static void s_glfwKeyCallback(GLFWwindow*, int key, int, int action, int);
    static InputManager* s_instance; // singleton para o callback C do GLFW

    std::unordered_map<int, bool> m_current;  // true = tecla premida agora
    std::unordered_map<int, bool> m_justDown; // true = premida NESTE frame
    std::unordered_map<int, bool> m_justUp;   // true = solta NESTE frame
};

} // namespace logic
