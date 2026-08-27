#include "GraphicsRuntime.h"

#include <vector>

namespace gfx {

bool GraphicsRuntime::init(uint32_t width, uint32_t height, const char* title) {
    if (isInitialized()) return true;

    // A previous failed initialization may have left owned resources alive.
    // Always normalize the object before a retry so every successful init
    // starts from the same neutral state.
    cleanup();

    auto fail = [this]() {
        cleanup();
        return false;
    };

    if (!window_.create(width, height, title)) return fail();

    std::vector<const char*> extensions;
    window_.appendRequiredExtensions(extensions);
    if (!context_.init(false, extensions)) return fail();

    const VkSurfaceKHR surface = window_.createVulkanSurface(context_.instance());
    if (surface == VK_NULL_HANDLE) return fail();
    if (!context_.createSurface(surface)) return fail();

    if (!swapchain_.init(&context_, &window_)) return fail();
    if (!renderPass_.init(&context_, &swapchain_)) return fail();
    if (!pipeline_.init(&context_, &swapchain_, &renderPass_)) return fail();
    if (!renderer_.init(&context_, &swapchain_, &renderPass_, &pipeline_)) return fail();

    return true;
}

void GraphicsRuntime::cleanup() {
    // Tear down in the reverse dependency order of the owned graphics stack.
    renderer_.cleanup();
    pipeline_.cleanup();
    renderPass_.cleanup();
    swapchain_.cleanup();
    context_.shutdown();
    window_.destroy();
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
