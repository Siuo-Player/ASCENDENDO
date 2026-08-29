#pragma once

#include <vulkan/vulkan.h>

#include <cstddef>
#include <string>

namespace gfx {

class VulkanContext;

// Small validation-only readback path for the current swapchain format.
// It records a copy into host-visible memory and writes a binary PPM after
// the submission fence has signalled. It is deliberately not a general
// screenshot/asset framework.
class VulkanFrameCapture {
public:
    VulkanFrameCapture() = default;
    ~VulkanFrameCapture();

    VulkanFrameCapture(const VulkanFrameCapture&) = delete;
    VulkanFrameCapture& operator=(const VulkanFrameCapture&) = delete;

    bool init(VulkanContext* context);
    void cleanup();

    bool supports(VkFormat format) const;
    bool prepare(VkExtent2D extent, VkFormat format);
    bool record(VkCommandBuffer commandBuffer, VkImage image,
                VkExtent2D extent, VkFormat format);
    bool writePpm(const std::string& path, VkFormat format, VkExtent2D extent);

private:
    bool recreateBuffer(VkDeviceSize size);
    uint32_t findHostVisibleMemoryType(uint32_t typeBits) const;

    VulkanContext* m_context = nullptr;
    VkBuffer m_stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_stagingMemory = VK_NULL_HANDLE;
    VkDeviceSize m_stagingSize = 0;
};

} // namespace gfx
