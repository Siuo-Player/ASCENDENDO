#pragma once
// =============================================================================
//  Game/Graphics/RenderPass.h
//
//  @version 4.2
//  @history
//    v4.2 — layout final alterado para TRANSFER_SRC_OPTIMAL para permitir Blit
// =============================================================================
#include <vulkan/vulkan.h>

namespace gfx {

class VulkanContext;
class Swapchain;

class RenderPass {
public:
    RenderPass() = default;
    ~RenderPass();

    bool init(VulkanContext* ctx, Swapchain* swapchain);
    void cleanup();

    VkRenderPass handle() const { return m_renderPass; }
    bool isInitialized() const { return m_renderPass != VK_NULL_HANDLE; }

private:
    VulkanContext* m_ctx = nullptr;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
};

} // namespace gfx