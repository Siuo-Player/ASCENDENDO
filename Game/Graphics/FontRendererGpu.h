#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace gfx {

class VulkanContext;

class FontRendererGpu {
public:
    FontRendererGpu() = default;
    ~FontRendererGpu();

    FontRendererGpu(const FontRendererGpu&) = delete;
    FontRendererGpu& operator=(const FontRendererGpu&) = delete;

    bool init(VulkanContext* ctx,
              VkDescriptorSetLayout descriptorSetLayout,
              const std::vector<uint8_t>& atlasPixels,
              uint32_t atlasWidth,
              uint32_t atlasHeight);

    void cleanup();
    void bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const;

    bool isInitialized() const { return m_initialized; }

private:
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) const;
    bool createAtlasImage(const std::vector<uint8_t>& pixels,
                          uint32_t atlasWidth,
                          uint32_t atlasHeight);
    bool createDescriptorSet(VkDescriptorSetLayout layout);

    VulkanContext* m_ctx = nullptr;
    VkImage m_image = VK_NULL_HANDLE;
    VkDeviceMemory m_imageMemory = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;
    VkDescriptorPool m_descPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descSet = VK_NULL_HANDLE;
    bool m_initialized = false;
};

} // namespace gfx
