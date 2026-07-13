// =============================================================================
//  Game/Logic/InputManager.cpp
//
//  @version 9.2
//  @history
//    v3.1 — criado
//    v3.3 — implementação de injectRawState
//    v9.2 — Rato: onMouseButtonEvent/onCursorPosEvent + callbacks GLFW
// =============================================================================

#include "Logic/InputManager.h"

#ifdef GLFW_AVAILABLE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

namespace logic {

InputManager* InputManager::s_instance = nullptr;

void InputManager::beginFrame() {
    m_justDown.clear();
    m_justUp.clear();
    m_mouseJustDown.clear();
    m_mouseJustUp.clear();
}

void InputManager::onKeyEvent(int key, int action) {
    if (action == Action::PRESS) {
        if (!isKeyDown(key)) m_justDown[key] = true;
        m_current[key] = true;
    } else if (action == Action::RELEASE) {
        if (isKeyDown(key)) m_justUp[key] = true;
        m_current[key] = false;
    }
}

void InputManager::registerWithWindow(GLFWwindow* window) {
#ifdef GLFW_AVAILABLE
    s_instance = this;
    glfwSetKeyCallback(window, s_glfwKeyCallback);
    glfwSetMouseButtonCallback(window, s_glfwMouseButtonCallback);
    glfwSetCursorPosCallback(window, s_glfwCursorPosCallback);
#else
    (void)window;
#endif
}

bool InputManager::isKeyDown(int key) const {
    auto it = m_current.find(key);
    return it != m_current.end() && it->second;
}

bool InputManager::isKeyJustPressed(int key) const {
    return m_justDown.count(key) > 0;
}

bool InputManager::isKeyJustReleased(int key) const {
    return m_justUp.count(key) > 0;
}

bool InputManager::isLeft()  const { return isKeyDown(Key::A)     || isKeyDown(Key::LEFT);  }
bool InputManager::isRight() const { return isKeyDown(Key::D)     || isKeyDown(Key::RIGHT); }
bool InputManager::isJump()  const { return isKeyDown(Key::SPACE);                          }

void InputManager::injectRawState(bool left, bool right, bool jumpHeld, bool jumpPressed, bool jumpReleased) {
    m_current.clear();
    m_justDown.clear();
    m_justUp.clear();

    if (left)          m_current[Key::A]      = true;
    if (right)         m_current[Key::D]      = true;
    if (jumpHeld)      m_current[Key::SPACE]  = true;
    if (jumpPressed)   m_justDown[Key::SPACE] = true;
    if (jumpReleased)  m_justUp[Key::SPACE]   = true;
}

void InputManager::s_glfwKeyCallback(GLFWwindow*, int key, int, int action, int) {
    if (s_instance) s_instance->onKeyEvent(key, action);
}

// ── Rato (Fase 9.2) ──────────────────────────────────────────────────────────

void InputManager::onMouseButtonEvent(int button, int action) {
    if (action == Action::PRESS) {
        if (!isMouseButtonDown(button)) m_mouseJustDown[button] = true;
        m_mouseCurrent[button] = true;
    } else if (action == Action::RELEASE) {
        if (isMouseButtonDown(button)) m_mouseJustUp[button] = true;
        m_mouseCurrent[button] = false;
    }
}

void InputManager::onCursorPosEvent(double x, double y) {
    m_cursorX = x;
    m_cursorY = y;
}

bool InputManager::isMouseButtonDown(int button) const {
    auto it = m_mouseCurrent.find(button);
    return it != m_mouseCurrent.end() && it->second;
}

bool InputManager::isMouseButtonJustPressed(int button) const {
    return m_mouseJustDown.count(button) > 0;
}

bool InputManager::isMouseButtonJustReleased(int button) const {
    return m_mouseJustUp.count(button) > 0;
}

void InputManager::injectCursorPos(double x, double y) {
    m_cursorX = x;
    m_cursorY = y;
}

void InputManager::s_glfwMouseButtonCallback(GLFWwindow*, int button, int action, int) {
    if (s_instance) s_instance->onMouseButtonEvent(button, action);
}

void InputManager::s_glfwCursorPosCallback(GLFWwindow*, double x, double y) {
    if (s_instance) s_instance->onCursorPosEvent(x, y);
}

} // namespace logic