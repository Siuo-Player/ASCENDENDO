#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <span>

namespace gfx {

class VulkanContext;

struct VulkanImageResource {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
};

// Creates and fully uploads a sampled 2D image.
// The caller owns the returned handles and must call destroyVulkanImage2D().
// Format/filter remain explicit because they are consumer-specific policy.
bool uploadVulkanImage2D(VulkanContext* ctx,
                         std::span<const uint8_t> pixels,
                         uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkFilter filter,
                         VulkanImageResource& out);

// Destroys all handles owned by the resource. The resource is reset to null
// handles. The caller is responsible for ensuring the device is idle when
// destroying resources that may still be referenced by submitted work.
void destroyVulkanImage2D(VulkanContext* ctx, VulkanImageResource& resource);

} // namespace gfx
