// =============================================================================
//  Game/Logic/InputManager.cpp
//
//  @version 3.1
//  @history
//    v3.1 — criado
// =============================================================================

#include "Logic/InputManager.h"

#ifdef GLFW_AVAILABLE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif

namespace logic {

InputManager* InputManager::s_instance = nullptr;

// =============================================================================

void InputManager::beginFrame() {
    m_justDown.clear();
    m_justUp.clear();
}

void InputManager::onKeyEvent(int key, int action) {
    if (action == Action::PRESS) {
        if (!isKeyDown(key)) m_justDown[key] = true;
        m_current[key] = true;
    } else if (action == Action::RELEASE) {
        if (isKeyDown(key)) m_justUp[key] = true;
        m_current[key] = false;
    }
    // REPEAT (2) ignorado — usar isKeyDown() para teclas mantidas premidas
}

void InputManager::registerWithWindow(GLFWwindow* window) {
#ifdef GLFW_AVAILABLE
    s_instance = this;
    glfwSetKeyCallback(window, s_glfwKeyCallback);
#else
    (void)window;
#endif
}

// =============================================================================

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

// =============================================================================

void InputManager::s_glfwKeyCallback(GLFWwindow*, int key, int, int action, int) {
    if (s_instance) s_instance->onKeyEvent(key, action);
}

} // namespace logic
