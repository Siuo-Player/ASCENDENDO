// =============================================================================
//  ASCENDENDO — Window implementation
//
//  GLFW library lifetime is owned by the process boundary. Window owns only
//  the GLFWwindow resource and therefore assumes GLFW has already been
//  initialized before create().
// =============================================================================
#include "Graphics/Window.h"

#ifdef GLFW_AVAILABLE

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

namespace gfx {

namespace {

bool readCaptureDimension(const char* name, uint32_t& value) {
    const char* text = std::getenv(name);
    if (!text || *text == '\0') return false;

    char* end = nullptr;
    const unsigned long long parsed = std::strtoull(text, &end, 10);
    if (end == text || *end != '\0' || parsed == 0 ||
        parsed > static_cast<unsigned long long>(std::numeric_limits<uint32_t>::max())) {
        return false;
    }

    value = static_cast<uint32_t>(parsed);
    return true;
}

void applyCaptureWindowDimensions(uint32_t& width, uint32_t& height) {
    uint32_t captureWidth = 0;
    uint32_t captureHeight = 0;
    if (readCaptureDimension("ASCENDENDO_CAPTURE_WINDOW_WIDTH", captureWidth) &&
        readCaptureDimension("ASCENDENDO_CAPTURE_WINDOW_HEIGHT", captureHeight)) {
        width = captureWidth;
        height = captureHeight;
    }
}

} // namespace

bool Window::create(uint32_t width, uint32_t height, const char* title) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    applyCaptureWindowDimensions(width, height);

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

    m_width = width;
    m_height = height;
    return true;
}

void Window::destroy() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
        m_handle = nullptr;
        m_width = 0;
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
    uint32_t count = 0;
    const char** extensions = glfwGetRequiredInstanceExtensions(&count);
    for (uint32_t i = 0; i < count; ++i) out.push_back(extensions[i]);
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

#else

namespace gfx {

bool Window::create(uint32_t, uint32_t, const char*) { return false; }
void Window::destroy() {}
bool Window::shouldClose() const { return true; }
void Window::pollEvents() {}
void Window::appendRequiredExtensions(std::vector<const char*>&) const {}
VkSurfaceKHR Window::createVulkanSurface(VkInstance) const { return VK_NULL_HANDLE; }

} // namespace gfx

#endif // GLFW_AVAILABLE
