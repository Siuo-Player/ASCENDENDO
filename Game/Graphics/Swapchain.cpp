// =============================================================================
//  Game/Graphics/Swapchain.cpp
// =============================================================================

#include "Graphics/Swapchain.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/Window.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

namespace gfx {

namespace {

bool chooseSurfaceFormat(VkPhysicalDevice device,
                         VkSurfaceKHR surface,
                         VkFormat& outFormat,
                         VkColorSpaceKHR& outColorSpace) {
    uint32_t count = 0;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, nullptr) != VK_SUCCESS || count == 0) {
        return false;
    }

    std::vector<VkSurfaceFormatKHR> formats(count);
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &count, formats.data()) != VK_SUCCESS) {
        return false;
    }

    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            outFormat = format.format;
            outColorSpace = format.colorSpace;
            return true;
        }
    }

    // RenderPass currently uses the swapchain format directly at initialization
    // and is not recreated during a frame. Reject a different format rather than
    // silently creating an incompatible render pass/framebuffer combination.
    return false;
}

VkPresentModeKHR choosePresentMode(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t count = 0;
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, nullptr) != VK_SUCCESS || count == 0) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    std::vector<VkPresentModeKHR> modes(count);
    if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &count, modes.data()) != VK_SUCCESS) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    if (std::find(modes.begin(), modes.end(), VK_PRESENT_MODE_FIFO_KHR) != modes.end()) {
        return VK_PRESENT_MODE_FIFO_KHR;
    }
    return modes.front();
}

VkCompositeAlphaFlagBitsKHR chooseCompositeAlpha(const VkSurfaceCapabilitiesKHR& capabilities) {
    constexpr VkCompositeAlphaFlagBitsKHR candidates[] = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
    };

    for (const auto candidate : candidates) {
        if (capabilities.supportedCompositeAlpha & candidate) return candidate;
    }
    return static_cast<VkCompositeAlphaFlagBitsKHR>(0);
}

VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                        uint32_t requestedWidth,
                        uint32_t requestedHeight) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    VkExtent2D extent{requestedWidth, requestedHeight};
    extent.width = std::clamp(extent.width,
                              capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height,
                               capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height);
    return extent;
}

} // namespace

void Swapchain::destroyImageResources() {
    if (!m_ctx) return;
    VkDevice device = m_ctx->device();

    for (auto view : m_imageViews) {
        if (view != VK_NULL_HANDLE) vkDestroyImageView(device, view, nullptr);
    }
    m_imageViews.clear();
    m_images.clear();
}

void Swapchain::cleanup() {
    if (m_ctx == nullptr) return;

    VkDevice device = m_ctx->device();
    destroyImageResources();

    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }

    m_extent = {};
    m_imageFormat = {};
    m_ctx = nullptr;
    m_window = nullptr;
}

bool Swapchain::init(VulkanContext* ctx, Window* window) {
    if (!ctx || !window || !ctx->isInitialized() || !window->isCreated()) return false;

    if (m_swapchain != VK_NULL_HANDLE || m_ctx != nullptr) cleanup();

    m_ctx = ctx;
    m_window = window;
    return createResources(VK_NULL_HANDLE);
}

bool Swapchain::recreate() {
    if (!m_ctx || !m_window || !m_ctx->isInitialized() || !m_window->isCreated()) {
        return false;
    }

    VkDevice device = m_ctx->device();
    if (vkDeviceWaitIdle(device) != VK_SUCCESS) return false;

    // Swapchain image views must be destroyed before their parent swapchain.
    destroyImageResources();
    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
    m_extent = {};
    m_imageFormat = {};

    return createResources(VK_NULL_HANDLE);
}

bool Swapchain::createResources(VkSwapchainKHR oldSwapchain) {
    if (!m_ctx || !m_window) return false;

    VkPhysicalDevice physicalDevice = m_ctx->physicalDevice();
    VkSurfaceKHR surface = m_ctx->surface();
    VkDevice device = m_ctx->device();
    if (physicalDevice == VK_NULL_HANDLE || surface == VK_NULL_HANDLE || device == VK_NULL_HANDLE) {
        return false;
    }

    // The current device exposes a single graphics queue. Before allocating
    // swapchain resources, fail explicitly if that queue cannot present to the
    // selected surface instead of discovering it later during vkQueuePresentKHR.
    VkBool32 graphicsCanPresent = VK_FALSE;
    if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,
                                             m_ctx->graphicsFamily(),
                                             surface,
                                             &graphicsCanPresent) != VK_SUCCESS ||
        graphicsCanPresent != VK_TRUE) {
        std::cerr << "[ERRO] A queue de graphics selecionada nao suporta presentation nesta surface.\n";
        return false;
    }

    VkSurfaceCapabilitiesKHR capabilities{};
    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities) != VK_SUCCESS) {
        return false;
    }

    VkFormat format = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    if (!chooseSurfaceFormat(physicalDevice, surface, format, colorSpace)) return false;

    const VkPresentModeKHR presentMode = choosePresentMode(physicalDevice, surface);
    const VkCompositeAlphaFlagBitsKHR compositeAlpha = chooseCompositeAlpha(capabilities);
    if (compositeAlpha == 0) return false;

    const VkExtent2D extent = chooseExtent(capabilities, m_window->width(), m_window->height());
    if (extent.width == 0 || extent.height == 0) return false;

    uint32_t imageCount = std::max(2u, capabilities.minImageCount);
    if (capabilities.maxImageCount > 0) {
        imageCount = std::min(imageCount, capabilities.maxImageCount);
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format;
    createInfo.imageColorSpace = colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = (capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
        ? VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
        : capabilities.currentTransform;
    createInfo.compositeAlpha = compositeAlpha;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapchain;

    VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
    VkResult result = vkCreateSwapchainKHR(device, &createInfo, nullptr, &newSwapchain);
    if (result != VK_SUCCESS) {
        std::cerr << "[ERRO] vkCreateSwapchainKHR falhou: " << result << "\n";
        return false;
    }

    uint32_t count = 0;
    if (vkGetSwapchainImagesKHR(device, newSwapchain, &count, nullptr) != VK_SUCCESS || count == 0) {
        vkDestroySwapchainKHR(device, newSwapchain, nullptr);
        return false;
    }

    std::vector<VkImage> images(count);
    if (vkGetSwapchainImagesKHR(device, newSwapchain, &count, images.data()) != VK_SUCCESS) {
        vkDestroySwapchainKHR(device, newSwapchain, nullptr);
        return false;
    }

    std::vector<VkImageView> imageViews(count, VK_NULL_HANDLE);
    for (uint32_t i = 0; i < count; ++i) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = images[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(device, &viewInfo, nullptr, &imageViews[i]);
        if (result != VK_SUCCESS) {
            for (auto view : imageViews) {
                if (view != VK_NULL_HANDLE) vkDestroyImageView(device, view, nullptr);
            }
            vkDestroySwapchainKHR(device, newSwapchain, nullptr);
            return false;
        }
    }

    m_swapchain = newSwapchain;
    m_imageFormat = format;
    m_extent = extent;
    m_images = std::move(images);
    m_imageViews = std::move(imageViews);
    return true;
}

} // namespace gfx
