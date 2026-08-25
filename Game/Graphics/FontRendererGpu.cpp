// =============================================================================
// Game/Graphics/FontRendererGpu.cpp
//
// Responsabilidade: gestão GPU do atlas de fontes do FontRenderer.
// A preparação CPU/baking continua em FontRenderer.cpp.
// =============================================================================
#include "Graphics/FontRenderer.h"
#include "Graphics/VulkanContext.h"

#include <cstring>

namespace gfx {

uint32_t FontRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) const {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(m_ctx->physicalDevice(), &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((typeFilter & (1u << i)) &&
            (memProps.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }
    return UINT32_MAX;
}

namespace {

VkCommandBuffer beginOneTimeCommands(VkDevice device, VkCommandPool pool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(device, &allocInfo, &cmd) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, pool, 1, &cmd);
        return VK_NULL_HANDLE;
    }
    return cmd;
}

bool endOneTimeCommands(VkDevice device, VkCommandPool pool, VkQueue queue,
                        VkCommandBuffer cmd) {
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, pool, 1, &cmd);
        return false;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, pool, 1, &cmd);
        return false;
    }

    if (vkQueueWaitIdle(queue) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, pool, 1, &cmd);
        return false;
    }

    vkFreeCommandBuffers(device, pool, 1, &cmd);
    return true;
}

} // namespace

bool FontRenderer::createAtlasImage(const std::vector<uint8_t>& pixels) {
    VkDevice device = m_ctx->device();
    VkDeviceSize imageSize = static_cast<VkDeviceSize>(pixels.size());

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    VkBufferCreateInfo bufInfo{};
    bufInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size = imageSize;
    bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &bufInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        memReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (allocInfo.memoryTypeIndex == UINT32_MAX) {
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        return false;
    }

    if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingMemory) != VK_SUCCESS) {
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        return false;
    }
    vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0);

    void* mapped = nullptr;
    if (vkMapMemory(device, stagingMemory, 0, imageSize, 0, &mapped) != VK_SUCCESS) {
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        return false;
    }
    std::memcpy(mapped, pixels.data(), static_cast<size_t>(imageSize));
    vkUnmapMemory(device, stagingMemory);

    VkImageCreateInfo imgInfo{};
    imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType = VK_IMAGE_TYPE_2D;
    imgInfo.extent = { static_cast<uint32_t>(ATLAS_W), static_cast<uint32_t>(ATLAS_H), 1 };
    imgInfo.mipLevels = 1;
    imgInfo.arrayLayers = 1;
    imgInfo.format = VK_FORMAT_R8_UNORM;
    imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imgInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(device, &imgInfo, nullptr, &m_image) != VK_SUCCESS) {
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        return false;
    }

    VkMemoryRequirements imgMemReq{};
    vkGetImageMemoryRequirements(device, m_image, &imgMemReq);

    VkMemoryAllocateInfo imgAllocInfo{};
    imgAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imgAllocInfo.allocationSize = imgMemReq.size;
    imgAllocInfo.memoryTypeIndex = findMemoryType(
        imgMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (imgAllocInfo.memoryTypeIndex == UINT32_MAX ||
        vkAllocateMemory(device, &imgAllocInfo, nullptr, &m_imageMemory) != VK_SUCCESS) {
        vkDestroyImage(device, m_image, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        m_image = VK_NULL_HANDLE;
        return false;
    }
    vkBindImageMemory(device, m_image, m_imageMemory, 0);

    VkCommandPoolCreateInfo poolCI{};
    poolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCI.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolCI.queueFamilyIndex = m_ctx->graphicsFamily();

    VkCommandPool tempPool = VK_NULL_HANDLE;
    if (vkCreateCommandPool(device, &poolCI, nullptr, &tempPool) != VK_SUCCESS) {
        vkDestroyImage(device, m_image, nullptr);
        vkFreeMemory(device, m_imageMemory, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        m_image = VK_NULL_HANDLE;
        m_imageMemory = VK_NULL_HANDLE;
        return false;
    }

    VkCommandBuffer cmd = beginOneTimeCommands(device, tempPool);
    if (cmd == VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, tempPool, nullptr);
        vkDestroyImage(device, m_image, nullptr);
        vkFreeMemory(device, m_imageMemory, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        m_image = VK_NULL_HANDLE;
        m_imageMemory = VK_NULL_HANDLE;
        return false;
    }

    VkImageMemoryBarrier toTransfer{};
    toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.image = m_image;
    toTransfer.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cmd,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &toTransfer);

    VkBufferImageCopy region{};
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageExtent = { static_cast<uint32_t>(ATLAS_W), static_cast<uint32_t>(ATLAS_H), 1 };
    vkCmdCopyBufferToImage(cmd, stagingBuffer, m_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VkImageMemoryBarrier toShaderRead = toTransfer;
    toShaderRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    toShaderRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    toShaderRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &toShaderRead);

    const bool submitted = endOneTimeCommands(
        device, tempPool, m_ctx->graphicsQueue(), cmd);
    vkDestroyCommandPool(device, tempPool, nullptr);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);

    if (!submitted) {
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
    viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) {
        vkDestroyImage(device, m_image, nullptr);
        vkFreeMemory(device, m_imageMemory, nullptr);
        m_image = VK_NULL_HANDLE;
        m_imageMemory = VK_NULL_HANDLE;
        return false;
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.maxAnisotropy = 1.0f;
    if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) {
        vkDestroyImageView(device, m_imageView, nullptr);
        vkDestroyImage(device, m_image, nullptr);
        vkFreeMemory(device, m_imageMemory, nullptr);
        m_imageView = VK_NULL_HANDLE;
        m_image = VK_NULL_HANDLE;
        m_imageMemory = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

bool FontRenderer::createDescriptorSet(VkDescriptorSetLayout layout) {
    VkDevice device = m_ctx->device();

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = 1;
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descPool) != VK_SUCCESS) {
        return false;
    }

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

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imgInfo.imageView = m_imageView;
    imgInfo.sampler = m_sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_descSet;
    write.dstBinding = 0;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &imgInfo;
    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

    return true;
}

} // namespace gfx
