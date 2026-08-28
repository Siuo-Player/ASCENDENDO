#pragma once

#include "Graphics/VulkanImageUpload.h"

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
    bool createDescriptorSet(VkDescriptorSetLayout layout);

    VulkanContext* m_ctx = nullptr;
    VulkanImageResource m_atlas{};
    VkDescriptorPool m_descPool = VK_NULL_HANDLE;
    VkDescriptorSet m_descSet = VK_NULL_HANDLE;
    bool m_initialized = false;
};

} // namespace gfx
