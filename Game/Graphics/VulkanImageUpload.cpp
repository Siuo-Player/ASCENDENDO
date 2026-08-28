#include "Graphics/VulkanImageUpload.h"
#include "Graphics/VulkanContext.h"

#include <cstring>
#include <limits>

namespace gfx {

namespace {

uint32_t findMemoryType(const VulkanContext& ctx,
                        uint32_t typeFilter,
                        VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProps{};
    vkGetPhysicalDeviceMemoryProperties(ctx.physicalDevice(), &memProps);

    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((typeFilter & (1u << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return std::numeric_limits<uint32_t>::max();
}

VkCommandBuffer beginOneTimeCommands(VkDevice device, VkCommandPool pool) {
    VkCommandBufferAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocateInfo.commandPool = pool;
    allocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    if (vkAllocateCommandBuffers(device, &allocateInfo, &commandBuffer) != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
        return VK_NULL_HANDLE;
    }

    return commandBuffer;
}

bool endOneTimeCommands(VkDevice device,
                        VkCommandPool pool,
                        VkQueue queue,
                        VkCommandBuffer commandBuffer) {
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
        return false;
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    if (vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
        return false;
    }

    if (vkQueueWaitIdle(queue) != VK_SUCCESS) {
        vkDeviceWaitIdle(device);
        vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
        return false;
    }

    vkFreeCommandBuffers(device, pool, 1, &commandBuffer);
    return true;
}

void resetResource(VulkanImageResource& resource) {
    resource.image = VK_NULL_HANDLE;
    resource.memory = VK_NULL_HANDLE;
    resource.view = VK_NULL_HANDLE;
    resource.sampler = VK_NULL_HANDLE;
}

} // namespace

bool uploadVulkanImage2D(VulkanContext* ctx,
                         std::span<const uint8_t> pixels,
                         uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkFilter filter,
                         VulkanImageResource& out) {
    resetResource(out);

    if (!ctx || !ctx->isInitialized() || pixels.empty() || width == 0 || height == 0) {
        return false;
    }

    const VkDevice device = ctx->device();
    const VkDeviceSize byteSize = static_cast<VkDeviceSize>(pixels.size());
    VulkanImageResource resource{};

    VkBuffer stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    auto cleanupStaging = [&]() {
        if (stagingBuffer) vkDestroyBuffer(device, stagingBuffer, nullptr);
        if (stagingMemory) vkFreeMemory(device, stagingMemory, nullptr);
        stagingBuffer = VK_NULL_HANDLE;
        stagingMemory = VK_NULL_HANDLE;
    };

    auto cleanupResource = [&]() {
        destroyVulkanImage2D(ctx, resource);
    };

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = byteSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
        return false;
    }

    VkMemoryRequirements stagingRequirements{};
    vkGetBufferMemoryRequirements(device, stagingBuffer, &stagingRequirements);

    VkMemoryAllocateInfo stagingAllocation{};
    stagingAllocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    stagingAllocation.allocationSize = stagingRequirements.size;
    stagingAllocation.memoryTypeIndex = findMemoryType(
        *ctx,
        stagingRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (stagingAllocation.memoryTypeIndex == std::numeric_limits<uint32_t>::max() ||
        vkAllocateMemory(device, &stagingAllocation, nullptr, &stagingMemory) != VK_SUCCESS) {
        cleanupStaging();
        return false;
    }

    if (vkBindBufferMemory(device, stagingBuffer, stagingMemory, 0) != VK_SUCCESS) {
        cleanupStaging();
        return false;
    }

    void* mapped = nullptr;
    if (vkMapMemory(device, stagingMemory, 0, byteSize, 0, &mapped) != VK_SUCCESS) {
        cleanupStaging();
        return false;
    }
    std::memcpy(mapped, pixels.data(), static_cast<size_t>(byteSize));
    vkUnmapMemory(device, stagingMemory);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent = {width, height, 1};
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(device, &imageInfo, nullptr, &resource.image) != VK_SUCCESS) {
        cleanupStaging();
        return false;
    }

    VkMemoryRequirements imageRequirements{};
    vkGetImageMemoryRequirements(device, resource.image, &imageRequirements);

    VkMemoryAllocateInfo imageAllocation{};
    imageAllocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imageAllocation.allocationSize = imageRequirements.size;
    imageAllocation.memoryTypeIndex = findMemoryType(
        *ctx,
        imageRequirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (imageAllocation.memoryTypeIndex == std::numeric_limits<uint32_t>::max() ||
        vkAllocateMemory(device, &imageAllocation, nullptr, &resource.memory) != VK_SUCCESS) {
        cleanupResource();
        cleanupStaging();
        return false;
    }

    if (vkBindImageMemory(device, resource.image, resource.memory, 0) != VK_SUCCESS) {
        cleanupResource();
        cleanupStaging();
        return false;
    }

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolInfo.queueFamilyIndex = ctx->graphicsFamily();

    VkCommandPool pool = VK_NULL_HANDLE;
    if (vkCreateCommandPool(device, &poolInfo, nullptr, &pool) != VK_SUCCESS) {
        cleanupResource();
        cleanupStaging();
        return false;
    }

    VkCommandBuffer commandBuffer = beginOneTimeCommands(device, pool);
    if (!commandBuffer) {
        vkDestroyCommandPool(device, pool, nullptr);
        cleanupResource();
        cleanupStaging();
        return false;
    }

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = resource.image;
    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    VkBufferImageCopy region{};
    region.imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
    region.imageExtent = {width, height, 1};
    vkCmdCopyBufferToImage(commandBuffer,
                           stagingBuffer,
                           resource.image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region);

    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier);

    const bool uploaded = endOneTimeCommands(
        device, pool, ctx->graphicsQueue(), commandBuffer);
    vkDestroyCommandPool(device, pool, nullptr);
    cleanupStaging();

    if (!uploaded) {
        cleanupResource();
        return false;
    }

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = resource.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    if (vkCreateImageView(device, &viewInfo, nullptr, &resource.view) != VK_SUCCESS) {
        cleanupResource();
        return false;
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = filter;
    samplerInfo.minFilter = filter;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.maxAnisotropy = 1.0f;
    if (vkCreateSampler(device, &samplerInfo, nullptr, &resource.sampler) != VK_SUCCESS) {
        cleanupResource();
        return false;
    }

    out = resource;
    return true;
}

void destroyVulkanImage2D(VulkanContext* ctx, VulkanImageResource& resource) {
    if (!ctx || !ctx->isInitialized()) {
        resetResource(resource);
        return;
    }

    const VkDevice device = ctx->device();
    if (resource.sampler) vkDestroySampler(device, resource.sampler, nullptr);
    if (resource.view) vkDestroyImageView(device, resource.view, nullptr);
    if (resource.image) vkDestroyImage(device, resource.image, nullptr);
    if (resource.memory) vkFreeMemory(device, resource.memory, nullptr);
    resetResource(resource);
}

} // namespace gfx
