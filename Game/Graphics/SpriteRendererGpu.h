#pragma once

#include "Graphics/VulkanImageUpload.h"

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
    bool createDescriptorSet(VkDescriptorSetLayout layout);

    VulkanContext* m_ctx = nullptr;
    VulkanImageResource m_texture{};
    VkDescriptorPool m_descPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descSet = VK_NULL_HANDLE;
    bool m_initialized = false;
};

} // namespace gfx
