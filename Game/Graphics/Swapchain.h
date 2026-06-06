#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace gfx {

// Forward declarations dentro do namespace correto
class VulkanContext;
class Window;

class Swapchain {
public:
    Swapchain();
    ~Swapchain();

    // Inicializa o swapchain associado a um contexto e a uma janela
    bool init(VulkanContext* ctx, Window* window);
    void cleanup();

    VkSwapchainKHR handle() const { return m_swapchain; }
    VkFormat imageFormat() const { return m_imageFormat; }
    VkExtent2D extent() const { return m_extent; }
    bool isInitialized() const { return m_swapchain != VK_NULL_HANDLE; }

private:
    VulkanContext* m_ctx = nullptr;
    VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
    VkFormat m_imageFormat;
    VkExtent2D m_extent;
    
    std::vector<VkImage> m_images;
};

} // namespace gfx