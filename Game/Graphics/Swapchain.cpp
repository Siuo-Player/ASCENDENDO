// =============================================================================
//  Game/Graphics/Swapchain.cpp
//
//  @version 2.40
//  @history
//    v2.4  — criado
//    v2.40 — adiciona criacao de VkImage + VkImageView (necessario para Renderer)
// =============================================================================

#include "Graphics/Swapchain.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/Window.h"

namespace gfx {

void Swapchain::cleanup() {
    if (m_ctx == nullptr) return;

    VkDevice device = m_ctx->device();

    for (auto iv : m_imageViews) {
        if (iv != VK_NULL_HANDLE) vkDestroyImageView(device, iv, nullptr);
    }
    m_imageViews.clear();
    m_images.clear();

    if (m_swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, m_swapchain, nullptr);
        m_swapchain = VK_NULL_HANDLE;
    }
    m_ctx = nullptr;
}

bool Swapchain::init(VulkanContext* ctx, Window* window) {
    if (!ctx || !window || !ctx->isInitialized() || !window->isCreated()) return false;
    m_ctx = ctx;

    // ── Criar Swapchain ───────────────────────────────────────────────────────
    VkSwapchainCreateInfoKHR ci{};
    ci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface          = ctx->surface();
    ci.minImageCount    = 2;                                // double buffering
    ci.imageFormat      = VK_FORMAT_B8G8R8A8_SRGB;
    ci.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    ci.imageExtent      = {window->width(), window->height()};
    ci.imageArrayLayers = 1;
    ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.preTransform     = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    ci.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode      = VK_PRESENT_MODE_FIFO_KHR; // V-Sync garantido
    ci.clipped          = VK_TRUE;
    ci.oldSwapchain     = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(ctx->device(), &ci, nullptr, &m_swapchain) != VK_SUCCESS) {
        return false;
    }

    m_imageFormat = ci.imageFormat;
    m_extent      = ci.imageExtent;

    // ── Obter imagens geradas pelo driver ─────────────────────────────────────
    uint32_t count = 0;
    vkGetSwapchainImagesKHR(ctx->device(), m_swapchain, &count, nullptr);
    m_images.resize(count);
    vkGetSwapchainImagesKHR(ctx->device(), m_swapchain, &count, m_images.data());

    // ── Criar Image Views (uma por imagem) ────────────────────────────────────
    m_imageViews.resize(count, VK_NULL_HANDLE);
    for (uint32_t i = 0; i < count; ++i) {
        VkImageViewCreateInfo viewCI{};
        viewCI.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCI.image                           = m_images[i];
        viewCI.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        viewCI.format                          = m_imageFormat;
        viewCI.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCI.subresourceRange.baseMipLevel   = 0;
        viewCI.subresourceRange.levelCount     = 1;
        viewCI.subresourceRange.baseArrayLayer = 0;
        viewCI.subresourceRange.layerCount     = 1;
        // components: VK_COMPONENT_SWIZZLE_IDENTITY e zero — valor padrao correto

        if (vkCreateImageView(ctx->device(), &viewCI, nullptr, &m_imageViews[i]) != VK_SUCCESS) {
            return false;
        }
    }

    return true;
}

} // namespace gfx
