#pragma once
// =============================================================================
//  Game/Graphics/Window.h
//
//  Wrapper da janela da aplicacao.
//  Implementacao real: GLFW (quando GLFW_AVAILABLE esta definido pelo Makefile).
//  Implementacao stub: retorna false/no-op (sem GLFW instalado).
//
//  Ownership:
//    Window owns the GLFWwindow resource only.
//    GLFW library initialization/termination is owned by the process boundary.
//
//  Precondition:
//    The process must initialize GLFW before calling create().
// =============================================================================

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>

struct GLFWwindow;

namespace gfx {

class Window {
public:
    Window() = default;
    ~Window() { destroy(); }

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool create(uint32_t width, uint32_t height, const char* title);
    void destroy();

    bool shouldClose() const;
    void pollEvents();

    void appendRequiredExtensions(std::vector<const char*>& out) const;
    VkSurfaceKHR createVulkanSurface(VkInstance instance) const;

    GLFWwindow* handle() const { return m_handle; }
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    bool isCreated() const { return m_handle != nullptr; }

private:
    GLFWwindow* m_handle = nullptr;
    uint32_t m_width = 0;
    uint32_t m_height = 0;
};

} // namespace gfx
