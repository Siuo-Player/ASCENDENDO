// =============================================================================
//  Game/Graphics/Window.cpp
// =============================================================================

#include "Graphics/Window.h"

// =============================================================================
//  Implementacao Real (requer GLFW instalado em external/glfw/)
// =============================================================================
#ifdef GLFW_AVAILABLE

#define GLFW_INCLUDE_VULKAN   // faz GLFW incluir vulkan.h automaticamente
#include <GLFW/glfw3.h>

namespace gfx {

bool Window::create(uint32_t width, uint32_t height, const char* title) {
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // sem contexto OpenGL
    glfwWindowHint(GLFW_RESIZABLE,  GLFW_FALSE);    // fixo por agora (Fase 4: redimensionavel)

    m_handle = glfwCreateWindow(static_cast<int>(width),
                                static_cast<int>(height),
                                title, nullptr, nullptr);
    if (!m_handle) {
        glfwTerminate();
        return false;
    }

    m_width  = width;
    m_height = height;
    return true;
}

void Window::destroy() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
        glfwTerminate();
        m_handle = nullptr;
        m_width  = 0;
        m_height = 0;
    }
}

bool Window::shouldClose() const {
    return m_handle && glfwWindowShouldClose(m_handle);
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::appendRequiredExtensions(std::vector<const char*>& out) const {
    uint32_t     count = 0;
    const char** exts  = glfwGetRequiredInstanceExtensions(&count);
    for (uint32_t i = 0; i < count; ++i) out.push_back(exts[i]);
}

VkSurfaceKHR Window::createVulkanSurface(VkInstance instance) const {
    if (!m_handle || instance == VK_NULL_HANDLE) return VK_NULL_HANDLE;

    VkSurfaceKHR surface = VK_NULL_HANDLE;
    if (glfwCreateWindowSurface(instance, m_handle, nullptr, &surface) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return surface;
}

} // namespace gfx

// =============================================================================
//  Implementacao Stub (GLFW nao disponivel — funcoes safe no-op)
// =============================================================================
#else

namespace gfx {

bool         Window::create(uint32_t, uint32_t, const char*) { return false;  }
void         Window::destroy()                               {}
bool         Window::shouldClose()                     const { return true;    }
void         Window::pollEvents()                            {}
void         Window::appendRequiredExtensions(std::vector<const char*>&) const {}
VkSurfaceKHR Window::createVulkanSurface(VkInstance)   const { return VK_NULL_HANDLE; }

} // namespace gfx

#endif // GLFW_AVAILABLE
