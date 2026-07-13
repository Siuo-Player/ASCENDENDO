#pragma once
// =============================================================================
//  Game/Logic/InputManager.h
//
//  @version 9.2
//  @history
//    v3.1 — criado (key states: down, justPressed, justReleased; callbacks GLFW)
//    v3.3 — injectRawState() adicionado para simulação e reprodução de replays
//    v9.1 — Key::E, Key::Q adicionados (defaults de GameAction::OpenEditor e
//            GameAction::Quit em KeyBindings.h). Puramente aditivo -- nenhum
//            valor/assinatura existente foi alterado; isLeft()/isRight()/
//            isJump() continuam identicos.
//    v9.2 — Rato: posicao do cursor + botoes (MouseButton::LEFT/RIGHT/MIDDLE),
//            mesmo padrao current/justDown/justUp ja usado para teclado, em
//            mapas SEPARADOS (m_mouseCurrent/...) -- nao ha risco de colisao
//            de codigos entre tecla e botao de rato. beginFrame() passa a
//            limpar tambem os buffers de rato. Puramente aditivo -- nenhum
//            metodo/campo existente foi alterado.
// =============================================================================

#include <unordered_map>

struct GLFWwindow;

namespace logic {

namespace Key {
    constexpr int LEFT   = 263;
    constexpr int RIGHT  = 262;
    constexpr int UP     = 265;
    constexpr int DOWN   = 264;
    constexpr int A      = 65;
    constexpr int D      = 68;
    constexpr int W      = 87;
    constexpr int S      = 83;
    constexpr int E      = 69;   // v9.1: default de GameAction::OpenEditor
    constexpr int Q      = 81;   // v9.1: default de GameAction::Quit
    constexpr int SPACE  = 32;
    constexpr int ESCAPE = 256;
}

namespace Action {
    constexpr int RELEASE = 0;
    constexpr int PRESS   = 1;
    constexpr int REPEAT  = 2;
}

namespace MouseButton {
    constexpr int LEFT   = 0;
    constexpr int RIGHT  = 1;
    constexpr int MIDDLE = 2;
}

class InputManager {
public:
    InputManager()  = default;
    ~InputManager() = default;

    void beginFrame();
    void onKeyEvent(int key, int action);
    void registerWithWindow(GLFWwindow* window);

    bool isKeyDown(int key)         const;
    bool isKeyJustPressed(int key)  const;
    bool isKeyJustReleased(int key) const;

    bool isLeft()  const;
    bool isRight() const;
    bool isJump()  const;

    // Injeta estados lógicos puros diretamente nos buffers (essencial para Replay/Mocks)
    void injectRawState(bool left, bool right, bool jumpHeld, bool jumpPressed, bool jumpReleased);

    // ── Rato (Fase 9.2) ──────────────────────────────────────────────────────
    void onMouseButtonEvent(int button, int action);
    void onCursorPosEvent(double x, double y);

    bool isMouseButtonDown(int button)         const;
    bool isMouseButtonJustPressed(int button)  const;
    bool isMouseButtonJustReleased(int button) const;

    // Posicao do cursor em coordenadas de JANELA (pixeis, Y para baixo,
    // origem topo-esquerdo -- convencao GLFW crua). Conversao para espaco
    // logico do motor fica em Core/Viewport.h (windowToLogical), chamada
    // pelo utilizador com o tamanho actual da janela.
    double cursorX() const { return m_cursorX; }
    double cursorY() const { return m_cursorY; }

    // Injecta posicao do cursor directamente (testes/mocks -- mesmo espirito
    // de injectRawState).
    void injectCursorPos(double x, double y);

private:
    static void s_glfwKeyCallback(GLFWwindow*, int key, int, int action, int);
    static void s_glfwMouseButtonCallback(GLFWwindow*, int button, int action, int mods);
    static void s_glfwCursorPosCallback(GLFWwindow*, double x, double y);
    static InputManager* s_instance;

    std::unordered_map<int, bool> m_current;
    std::unordered_map<int, bool> m_justDown;
    std::unordered_map<int, bool> m_justUp;

    std::unordered_map<int, bool> m_mouseCurrent;
    std::unordered_map<int, bool> m_mouseJustDown;
    std::unordered_map<int, bool> m_mouseJustUp;
    double m_cursorX = 0.0;
    double m_cursorY = 0.0;
};

} // namespace logic