#include "Graphics/SpriteRendererGpu.h"
#include "Graphics/VulkanImageUpload.h"
#include "Graphics/VulkanContext.h"

namespace gfx {

SpriteRendererGpu::~SpriteRendererGpu() { cleanup(); }

bool SpriteRendererGpu::init(VulkanContext* ctx,
                             VkDescriptorSetLayout layout,
                             const std::vector<unsigned char>& pixels,
                             uint32_t width,
                             uint32_t height) {
    if (m_initialized) return true;
    if (!ctx || !ctx->isInitialized() || pixels.empty() || width == 0 || height == 0) return false;

    m_ctx = ctx;
    const std::span<const uint8_t> pixelSpan(
        reinterpret_cast<const uint8_t*>(pixels.data()), pixels.size());
    if (!uploadVulkanImage2D(ctx, pixelSpan,
                             width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_FILTER_NEAREST, m_texture) ||
        !createDescriptorSet(layout)) {
        cleanup();
        return false;
    }

    m_initialized = true;
    return true;
}

void SpriteRendererGpu::cleanup() {
    if (!m_ctx) return;

    VkDevice device = m_ctx->device();
    vkDeviceWaitIdle(device);

    if (m_descPool) vkDestroyDescriptorPool(device, m_descPool, nullptr);
    m_descPool = VK_NULL_HANDLE;
    m_descSet = VK_NULL_HANDLE;

    destroyVulkanImage2D(m_ctx, m_texture);
    m_initialized = false;
    m_ctx = nullptr;
}

void SpriteRendererGpu::bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const {
    if (!m_initialized) return;
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &m_descSet, 0, nullptr);
}

bool SpriteRendererGpu::createDescriptorSet(VkDescriptorSetLayout layout) {
    VkDevice device = m_ctx->device();

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descPool) != VK_SUCCESS) return false;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;
    if (vkAllocateDescriptorSets(device, &allocInfo, &m_descSet) != VK_SUCCESS) {
        vkDestroyDescriptorPool(device, m_descPool, nullptr);
        m_descPool = VK_NULL_HANDLE;
        return false;
    }

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_texture.view;
    imageInfo.sampler = m_texture.sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_descSet;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
    return true;
}

} // namespace gfx
