#pragma once
// =============================================================================
//  Game/Logic/InputManager.h
//
//  @version 3.3
//  @history
//    v3.1 — criado (key states: down, justPressed, justReleased; callbacks GLFW)
//    v3.3 — injectRawState() adicionado para simulação e reprodução de replays
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
    constexpr int SPACE  = 32;
    constexpr int ESCAPE = 256;
}

namespace Action {
    constexpr int RELEASE = 0;
    constexpr int PRESS   = 1;
    constexpr int REPEAT  = 2;
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

private:
    static void s_glfwKeyCallback(GLFWwindow*, int key, int, int action, int);
    static InputManager* s_instance;

    std::unordered_map<int, bool> m_current;
    std::unordered_map<int, bool> m_justDown;
    std::unordered_map<int, bool> m_justUp;
};

} // namespace logic