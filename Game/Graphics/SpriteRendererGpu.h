#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <vector>

namespace gfx {

class VulkanContext;

class SpriteRendererGpu {
public:
    SpriteRendererGpu() = default;
    ~SpriteRendererGpu();

    SpriteRendererGpu(const SpriteRendererGpu&) = delete;
    SpriteRendererGpu& operator=(const SpriteRendererGpu&) = delete;

    bool init(VulkanContext* ctx,
              VkDescriptorSetLayout descriptorSetLayout,
              const std::vector<unsigned char>& pixels,
              uint32_t width,
              uint32_t height);

    void cleanup();
    void bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const;

    bool isInitialized() const { return m_initialized; }

private:
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) const;
    bool createTextureImage(const std::vector<unsigned char>& pixels,
                            uint32_t width,
                            uint32_t height);
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
