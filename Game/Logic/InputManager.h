#pragma once
// =============================================================================
//  Game/Logic/InputManager.h
// =============================================================================
#include "Logic/TickInput.h"

#include <cstddef>
#include <unordered_map>

struct GLFWwindow;

namespace core { class KeyBindings; }

namespace logic {

namespace Key {
    constexpr int LEFT    = 263;
    constexpr int RIGHT   = 262;
    constexpr int UP      = 265;
    constexpr int DOWN    = 264;
    constexpr int A       = 65;
    constexpr int C       = 67;
    constexpr int D       = 68;
    constexpr int W       = 87;
    constexpr int S       = 83;
    constexpr int E       = 69;
    constexpr int Q       = 81;
    constexpr int G       = 71;
    constexpr int NUM0    = 48;
    constexpr int NUM1    = 49;
    constexpr int NUM2    = 50;
    constexpr int NUM3    = 51;
    constexpr int F2      = 291; // legacy compatibility only
    constexpr int F5      = 294; // legacy compatibility only
    constexpr int F6      = 295; // legacy compatibility only
    constexpr int LBRACKET  = 91;
    constexpr int RBRACKET  = 93;
    constexpr int DELETE_KEY = 261;
    constexpr int BACKSPACE  = 259;
    constexpr int SPACE   = 32;
    constexpr int ESCAPE  = 256;
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

    bool isKeyDown(int key) const;
    bool isKeyJustPressed(int key) const;
    bool isKeyJustReleased(int key) const;

    bool isLeft() const;
    bool isRight() const;
    bool isJump() const;

    // Converts the frame sample into the semantic input consumed by one
    // simulation tick. Edge events belong only to tick zero of that frame;
    // continuous actions remain valid for every fixed step derived from it.
    TickInput tickInput(const core::KeyBindings& bindings, std::size_t tickInFrame) const;

    void injectRawState(bool left, bool right, bool jumpHeld, bool jumpPressed, bool jumpReleased);

    void onMouseButtonEvent(int button, int action);
    void onCursorPosEvent(double x, double y);
    bool isMouseButtonDown(int button) const;
    bool isMouseButtonJustPressed(int button) const;
    bool isMouseButtonJustReleased(int button) const;

    double cursorX() const { return m_cursorX; }
    double cursorY() const { return m_cursorY; }
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
