#include "Swapchain.h"
#include "VulkanContext.h"
#include "Window.h"
#include <stdexcept>

namespace gfx {

Swapchain::Swapchain() {}

Swapchain::~Swapchain() {
    cleanup();
}

void Swapchain::cleanup() {
    if (m_swapchain != VK_NULL_HANDLE && m_ctx != nullptr) {
        vkDestroySwapchainKHR(m_ctx->device(), m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
}

bool Swapchain::init(VulkanContext* ctx, Window* window) {
    if (!ctx || !window || !ctx->isInitialized() || !window->isCreated()) return false;
    m_ctx = ctx;
    
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = ctx->surface();
    createInfo.minImageCount = 2; // Double buffering
    createInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageExtent = { static_cast<uint32_t>(window->width()), static_cast<uint32_t>(window->height()) };
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // V-Sync ativado por defeito
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_ctx->device(), &createInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        return false;
    }

    m_imageFormat = createInfo.imageFormat;
    m_extent = createInfo.imageExtent;

    return true;
}

} // namespace gfx