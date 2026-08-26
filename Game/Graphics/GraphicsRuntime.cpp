#include "GraphicsRuntime.h"

#include <vector>

namespace gfx {

bool GraphicsRuntime::init(uint32_t width, uint32_t height, const char* title) {
    if (!window_.create(width, height, title)) return false;

    std::vector<const char*> extensions;
    window_.appendRequiredExtensions(extensions);
    if (!context_.init(false, extensions)) return false;

    const VkSurfaceKHR surface = window_.createVulkanSurface(context_.instance());
    if (surface == VK_NULL_HANDLE) return false;
    if (!context_.createSurface(surface)) return false;

    if (!swapchain_.init(&context_, &window_)) return false;
    if (!renderPass_.init(&context_, &swapchain_)) return false;
    if (!pipeline_.init(&context_, &swapchain_, &renderPass_)) return false;
    if (!renderer_.init(&context_, &swapchain_, &renderPass_, &pipeline_)) return false;

    return true;
}

bool GraphicsRuntime::isInitialized() const {
    return window_.isCreated() &&
           context_.isInitialized() &&
           swapchain_.isInitialized() &&
           renderPass_.isInitialized() &&
           pipeline_.isInitialized() &&
           renderer_.isInitialized();
}

} // namespace gfx
