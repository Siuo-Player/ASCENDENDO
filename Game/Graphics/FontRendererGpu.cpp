#include "Graphics/FontRendererGpu.h"
#include "Graphics/VulkanContext.h"

#include <cstring>

namespace gfx {

FontRendererGpu::~FontRendererGpu() { cleanup(); }

uint32_t FontRendererGpu::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) const {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(m_ctx->physicalDevice(), &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((typeFilter & (1u << i)) &&
            (memProps.memoryTypes[i].propertyFlags & props) == props) return i;
    }
    return UINT32_MAX;
}

namespace {
VkCommandBuffer beginOneTimeCommands(VkDevice device, VkCommandPool pool) {
    VkCommandBufferAllocateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    info.commandPool = pool;
    info.commandBufferCount = 1;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(device, &info, &cmd) != VK_SUCCESS) return VK_NULL_HANDLE;
    VkCommandBufferBeginInfo begin{};
    begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(cmd, &begin) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, pool, 1, &cmd);
        return VK_NULL_HANDLE;
    }
    return cmd;
}

bool endOneTimeCommands(VkDevice device, VkCommandPool pool, VkQueue queue, VkCommandBuffer cmd) {
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, pool, 1, &cmd);
        return false;
    }
    VkSubmitInfo submit{};
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &cmd;
    if (vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE) != VK_SUCCESS ||
        vkQueueWaitIdle(queue) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, pool, 1, &cmd);
        return false;
    }
    vkFreeCommandBuffers(device, pool, 1, &cmd);
    return true;
}
} // namespace

bool FontRendererGpu::init(VulkanContext* ctx, VkDescriptorSetLayout layout,
                           const std::vector<uint8_t>& pixels, uint32_t width, uint32_t height) {
    if (m_initialized) return true;
    if (!ctx || !ctx->isInitialized() || pixels.empty()) return false;
    m_ctx = ctx;
    if (!createAtlasImage(pixels, width, height) || !createDescriptorSet(layout)) {
        cleanup();
        return false;
    }
    m_initialized = true;
    return true;
}

void FontRendererGpu::cleanup() {
    if (!m_ctx) return;
    VkDevice device = m_ctx->device();
    vkDeviceWaitIdle(device);
    if (m_descPool) vkDestroyDescriptorPool(device, m_descPool, nullptr);
    if (m_sampler) vkDestroySampler(device, m_sampler, nullptr);
    if (m_imageView) vkDestroyImageView(device, m_imageView, nullptr);
    if (m_image) vkDestroyImage(device, m_image, nullptr);
    if (m_imageMemory) vkFreeMemory(device, m_imageMemory, nullptr);
    m_descPool = VK_NULL_HANDLE;
    m_descSet = VK_NULL_HANDLE;
    m_sampler = VK_NULL_HANDLE;
    m_imageView = VK_NULL_HANDLE;
    m_image = VK_NULL_HANDLE;
    m_imageMemory = VK_NULL_HANDLE;
    m_initialized = false;
    m_ctx = nullptr;
}

void FontRendererGpu::bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const {
    if (!m_initialized) return;
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &m_descSet, 0, nullptr);
}

bool FontRendererGpu::createAtlasImage(const std::vector<uint8_t>& pixels,
                                       uint32_t width, uint32_t height) {
    VkDevice device = m_ctx->device();
    const VkDeviceSize size = static_cast<VkDeviceSize>(pixels.size());
    VkBuffer staging = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &staging) != VK_SUCCESS) return false;

    VkMemoryRequirements req{};
    vkGetBufferMemoryRequirements(device, staging, &req);
    VkMemoryAllocateInfo alloc{};
    alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc.allocationSize = req.size;
    alloc.memoryTypeIndex = findMemoryType(
        req.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (alloc.memoryTypeIndex == UINT32_MAX ||
        vkAllocateMemory(device, &alloc, nullptr, &stagingMemory) != VK_SUCCESS) {
        vkDestroyBuffer(device, staging, nullptr);
        return false;
    }
    vkBindBufferMemory(device, staging, stagingMemory, 0);

    void* mapped = nullptr;
    if (vkMapMemory(device, stagingMemory, 0, size, 0, &mapped) != VK_SUCCESS) {
        vkDestroyBuffer(device, staging, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        return false;
    }
    std::memcpy(mapped, pixels.data(), static_cast<size_t>(size));
    vkUnmapMemory(device, stagingMemory);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = {width, height, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    if (vkCreateImage(device, &imageInfo, nullptr, &m_image) != VK_SUCCESS) {
        vkDestroyBuffer(device, staging, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        return false;
    }

    VkMemoryRequirements imageReq{};
    vkGetImageMemoryRequirements(device, m_image, &imageReq);
    VkMemoryAllocateInfo imageAlloc{};
    imageAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imageAlloc.allocationSize = imageReq.size;
    imageAlloc.memoryTypeIndex = findMemoryType(imageReq.memoryTypeBits,
                                                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (imageAlloc.memoryTypeIndex == UINT32_MAX ||
        vkAllocateMemory(device, &imageAlloc, nullptr, &m_imageMemory) != VK_SUCCESS) {
        vkDestroyImage(device, m_image, nullptr);
        m_image = VK_NULL_HANDLE;
        vkDestroyBuffer(device, staging, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        return false;
    }
    vkBindImageMemory(device, m_image, m_imageMemory, 0);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = m_ctx->graphicsFamily();
    VkCommandPool pool = VK_NULL_HANDLE;
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        vkDestroyImage(device, m_image, nullptr);
        vkFreeMemory(device, m_imageMemory, nullptr);
        m_image = VK_NULL_HANDLE;
        m_imageMemory = VK_NULL_HANDLE;
        vkDestroyBuffer(device, staging, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        return false;
    }

    VkCommandBuffer cmd = beginOneTimeCommands(device, pool);
    if (!cmd) {
        vkDestroyCommandPool(device, pool, nullptr);
        vkDestroyImage(device, m_image, nullptr);
        vkFreeMemory(device, m_imageMemory, nullptr);
        m_image = VK_NULL_HANDLE;
        m_imageMemory = VK_NULL_HANDLE;
        vkDestroyBuffer(device, staging, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        return false;
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr,
                         1, &barrier);

    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {width, height, 1};
    vkCmdCopyBufferToImage(cmd, staging, m_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr,
                         0, nullptr, 1, &barrier);

    const bool ok = endOneTimeCommands(device, pool, m_ctx->graphicsQueue(), cmd);
    vkDestroyCommandPool(device, pool, nullptr);
    vkDestroyBuffer(device, staging, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);
    if (!ok) {
        vkDestroyImage(device, m_image, nullptr);
        vkFreeMemory(device, m_imageMemory, nullptr);
        m_image = VK_NULL_HANDLE;
        m_imageMemory = VK_NULL_HANDLE;
        return false;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8_UNORM;
    viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) return false;

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.maxAnisotropy = 1.0f;
    if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) return false;
    return true;
}

bool FontRendererGpu::createDescriptorSet(VkDescriptorSetLayout layout) {
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
    imageInfo.imageView = m_imageView;
    imageInfo.sampler = m_sampler;
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
