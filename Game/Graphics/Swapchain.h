#pragma once
// =============================================================================
//  Game/Graphics/Swapchain.h
//
//  Owned swapchain resources plus explicit recreation support after
//  VK_ERROR_OUT_OF_DATE_KHR / VK_SUBOPTIMAL_KHR.
// =============================================================================

#include <vulkan/vulkan.h>
#include <vector>

namespace gfx {

class VulkanContext;
class Window;

class Swapchain {
public:
    Swapchain() = default;
    ~Swapchain() { cleanup(); }

    Swapchain(const Swapchain&) = delete;
    Swapchain& operator=(const Swapchain&) = delete;

    bool init(VulkanContext* ctx, Window* window);
    bool recreate();
    void cleanup();

    VkSwapchainKHR handle() const { return m_swapchain; }
    VkFormat imageFormat() const { return m_imageFormat; }
    VkExtent2D extent() const { return m_extent; }
    const std::vector<VkImageView>& imageViews() const { return m_imageViews; }
    const std::vector<VkImage>& images() const { return m_images; }
    uint32_t imageCount() const {
        return static_cast<uint32_t>(m_imageViews.size());
    }
    bool supportsTransferSrc() const { return m_supportsTransferSrc; }
    bool isInitialized() const { return m_swapchain != VK_NULL_HANDLE; }

private:
    bool createResources(VkSwapchainKHR oldSwapchain);
    void destroyImageResources();

    VulkanContext* m_ctx = nullptr;
    Window* m_window = nullptr;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_imageFormat{};
    VkExtent2D m_extent{};
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    bool m_supportsTransferSrc = false;
};

} // namespace gfx
