#pragma once
// =============================================================================
//  Game/Graphics/Window.h
//
//  Wrapper da janela da aplicacao.
//  Implementacao real: GLFW (quando GLFW_AVAILABLE esta definido pelo Makefile).
//  Implementacao stub: retorna false/no-op (sem GLFW instalado).
//
//  Utilizacao normal:
//    gfx::Window win;
//    if (!win.create(1280, 720, "ASCENDENDO")) { /* erro */ }
//    while (!win.shouldClose()) { win.pollEvents(); /* render */ }
//    win.destroy();
//
//  Integracao com Vulkan:
//    win.appendRequiredExtensions(extensions);  // adiciona VK_KHR_surface etc.
//    VkSurfaceKHR surface = win.createVulkanSurface(ctx.instance());
// =============================================================================

#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>

// Forward-declare GLFWwindow para nao expor GLFW no header publico.
// O utilizador de Window.h nunca precisa de incluir GLFW diretamente.
struct GLFWwindow;

namespace gfx {

class Window {
public:
    Window()  = default;
    ~Window() { destroy(); }

    Window(const Window&)            = delete;
    Window& operator=(const Window&) = delete;

    // ── Ciclo de Vida ─────────────────────────────────────────────────────────
    bool create(uint32_t width, uint32_t height, const char* title);
    void destroy();

    // ── Loop Principal ────────────────────────────────────────────────────────
    bool shouldClose()  const;
    void pollEvents();

    // ── Integracao Vulkan ─────────────────────────────────────────────────────

    // Preenche 'out' com as extensoes de VkInstance necessarias para esta janela.
    // Chamar ANTES de VulkanContext::init().
    void appendRequiredExtensions(std::vector<const char*>& out) const;

    // Cria uma VkSurfaceKHR a partir desta janela e da VkInstance fornecida.
    // Retorna VK_NULL_HANDLE em caso de erro ou se GLFW nao estiver disponivel.
    VkSurfaceKHR createVulkanSurface(VkInstance instance) const;

    // ── Acessores ─────────────────────────────────────────────────────────────
    GLFWwindow* handle()    const { return m_handle; }
    uint32_t    width()     const { return m_width;  }
    uint32_t    height()    const { return m_height; }
    bool        isCreated() const { return m_handle != nullptr; }

private:
    GLFWwindow* m_handle = nullptr;
    uint32_t    m_width  = 0;
    uint32_t    m_height = 0;
};

} // namespace gfx
