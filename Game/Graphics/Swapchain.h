#pragma once
// =============================================================================
//  Game/Graphics/Swapchain.h
//
//  @version 2.40
//  @history
//    v2.4  — criado (VkSwapchainKHR, formato B8G8R8A8_SRGB, V-Sync FIFO)
//    v2.40 — adiciona imagens e image views (necessario para Framebuffers)
//
//  Gere o double-buffering entre GPU e ecra.
//  Expoe imageViews() para o Renderer criar os Framebuffers.
// =============================================================================

#include <vulkan/vulkan.h>
#include <vector>

namespace gfx {

class VulkanContext;
class Window;

class Swapchain {
public:
    Swapchain()  = default;
    ~Swapchain() { cleanup(); }

    bool init(VulkanContext* ctx, Window* window);
    void cleanup();

    // ── Acessores ─────────────────────────────────────────────────────────────
    VkSwapchainKHR                    handle()      const { return m_swapchain;   }
    VkFormat                          imageFormat() const { return m_imageFormat; }
    VkExtent2D                        extent()      const { return m_extent;      }
    const std::vector<VkImageView>&   imageViews()  const { return m_imageViews;  }
    uint32_t                          imageCount()  const { return static_cast<uint32_t>(m_imageViews.size()); }
    bool                              isInitialized() const { return m_swapchain != VK_NULL_HANDLE; }

private:
    VulkanContext*           m_ctx        = nullptr;
    VkSwapchainKHR           m_swapchain  = VK_NULL_HANDLE;
    VkFormat                 m_imageFormat{};
    VkExtent2D               m_extent{};
    std::vector<VkImage>     m_images;
    std::vector<VkImageView> m_imageViews;
};

} // namespace gfx
