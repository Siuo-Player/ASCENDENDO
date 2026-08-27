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

#include <algorithm>
#include <cmath>

namespace gfx {

bool Window::create(uint32_t width, uint32_t height, const char* title) {
    // GLFW process lifetime is owned by the application/process boundary.
    // Window owns only the GLFWwindow resource.
    if (!glfwGetVersionString()) return false;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);   // sem contexto OpenGL
    glfwWindowHint(GLFW.RESIZABLE, GLFW_FALSE);     // fixo por agora (Fase 4: redimensionavel)

    // O caller pode passar a resolucao total do monitor. Uma janela normal
    // acrescenta decoracao fora da area cliente, portanto uma janela com
    // exactamente a resolucao do monitor pode ficar cortada no lado direito.
    // Limitar a 90% da area do monitor garante que o client area e os borders
    // conseguem caber mantendo o mesmo rácio pedido.
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = monitor ? glfwGetVideoMode(monitor) : nullptr;
    if (mode && width > 0 && height > 0) {
        const double maxWidth = static_cast<double>(mode->width) * 0.90;
        const double maxHeight = static_cast<double>(mode->height) * 0.90;
        const double scale = std::min({
            1.0,
            maxWidth / static_cast<double>(width),
            maxHeight / static_cast<double>(height)
        });

        if (scale < 1.0) {
            width = static_cast<uint32_t>(std::floor(width * scale));
            height = static_cast<uint32_t>(std::floor(height * scale));
        }
    }

    m_handle = glfwCreateWindow(static_cast<int>(width),
                                static_cast<int>(height),
                                title, nullptr, nullptr);
    if (!m_handle) return false;

    m_width  = width;
    m_height = height;
    return true;
}

void Window::destroy() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
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
