#include "Graphics/VulkanFrameCapture.h"
#include "Graphics/VulkanContext.h"

#include <array>
#include <fstream>

namespace gfx {

VulkanFrameCapture::~VulkanFrameCapture() { cleanup(); }

bool VulkanFrameCapture::init(VulkanContext* context) {
    if (!context || !context->isInitialized()) return false;
    m_context = context;
    return true;
}

void VulkanFrameCapture::cleanup() {
    if (!m_context) return;
    VkDevice device = m_context->device();
    if (m_stagingBuffer) vkDestroyBuffer(device, m_stagingBuffer, nullptr);
    if (m_stagingMemory) vkFreeMemory(device, m_stagingMemory, nullptr);
    m_stagingBuffer = VK_NULL_HANDLE;
    m_stagingMemory = VK_NULL_HANDLE;
    m_stagingSize = 0;
    m_context = nullptr;
}

bool VulkanFrameCapture::supports(VkFormat format) const {
    return format == VK_FORMAT_B8G8R8A8_SRGB;
}

uint32_t VulkanFrameCapture::findHostVisibleMemoryType(uint32_t typeBits) const {
    if (!m_context) return UINT32_MAX;
    VkPhysicalDeviceMemoryProperties properties{};
    vkGetPhysicalDeviceMemoryProperties(m_context->physicalDevice(), &properties);
    for (uint32_t i = 0; i < properties.memoryTypeCount; ++i) {
        if ((typeBits & (1u << i)) == 0) continue;
        const auto flags = properties.memoryTypes[i].propertyFlags;
        if ((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            (flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) return i;
    }
    return UINT32_MAX;
}

bool VulkanFrameCapture::recreateBuffer(VkDeviceSize size) {
    if (!m_context || size == 0) return false;
    VkDevice device = m_context->device();
    if (m_stagingBuffer) vkDestroyBuffer(device, m_stagingBuffer, nullptr);
    if (m_stagingMemory) vkFreeMemory(device, m_stagingMemory, nullptr);
    m_stagingBuffer = VK_NULL_HANDLE;
    m_stagingMemory = VK_NULL_HANDLE;
    m_stagingSize = 0;

    VkBufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size = size;
    info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &info, nullptr, &m_stagingBuffer) != VK_SUCCESS) return false;

    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(device, m_stagingBuffer, &requirements);
    const uint32_t memoryType = findHostVisibleMemoryType(requirements.memoryTypeBits);
    if (memoryType == UINT32_MAX) {
        vkDestroyBuffer(device, m_stagingBuffer, nullptr);
        m_stagingBuffer = VK_NULL_HANDLE;
        return false;
    }

    VkMemoryAllocateInfo allocation{};
    allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocation.allocationSize = requirements.size;
    allocation.memoryTypeIndex = memoryType;
    if (vkAllocateMemory(device, &allocation, nullptr, &m_stagingMemory) != VK_SUCCESS) {
        vkDestroyBuffer(device, m_stagingBuffer, nullptr);
        m_stagingBuffer = VK_NULL_HANDLE;
        return false;
    }
    if (vkBindBufferMemory(device, m_stagingBuffer, m_stagingMemory, 0) != VK_SUCCESS) {
        vkFreeMemory(device, m_stagingMemory, nullptr);
        vkDestroyBuffer(device, m_stagingBuffer, nullptr);
        m_stagingMemory = VK_NULL_HANDLE;
        m_stagingBuffer = VK_NULL_HANDLE;
        return false;
    }
    m_stagingSize = size;
    return true;
}

bool VulkanFrameCapture::prepare(VkExtent2D extent, VkFormat format) {
    if (!m_context || extent.width == 0 || extent.height == 0 || !supports(format)) return false;
    const VkDeviceSize size = static_cast<VkDeviceSize>(extent.width) *
                               static_cast<VkDeviceSize>(extent.height) * 4u;
    return (m_stagingBuffer && m_stagingSize >= size) || recreateBuffer(size);
}

bool VulkanFrameCapture::record(VkCommandBuffer commandBuffer, VkImage image,
                                VkExtent2D extent, VkFormat format) {
    if (commandBuffer == VK_NULL_HANDLE || image == VK_NULL_HANDLE || !prepare(extent, format)) return false;

    VkImageMemoryBarrier toTransfer{};
    toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toTransfer.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    toTransfer.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    toTransfer.image = image;
    toTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toTransfer.subresourceRange.levelCount = 1;
    toTransfer.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &toTransfer);

    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {extent.width, extent.height, 1};
    vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           m_stagingBuffer, 1, &region);

    VkImageMemoryBarrier toPresent{};
    toPresent.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toPresent.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    toPresent.dstAccessMask = 0;
    toPresent.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    toPresent.image = image;
    toPresent.subresourceRange = toTransfer.subresourceRange;

    vkCmdPipelineBarrier(commandBuffer,
                         VK_PIPELINE_STAGE_TRANSFER_BIT,
                         VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &toPresent);
    return true;
}

bool VulkanFrameCapture::writePpm(const std::string& path, VkFormat format, VkExtent2D extent) {
    if (!m_context || !m_stagingMemory || !supports(format) || extent.width == 0 || extent.height == 0) {
        return false;
    }
    const VkDeviceSize required = static_cast<VkDeviceSize>(extent.width) *
                                  static_cast<VkDeviceSize>(extent.height) * 4u;
    if (m_stagingSize < required) return false;

    void* mapped = nullptr;
    if (vkMapMemory(m_context->device(), m_stagingMemory, 0, required, 0, &mapped) != VK_SUCCESS) return false;

    std::ofstream output(path, std::ios::binary);
    if (!output) {
        vkUnmapMemory(m_context->device(), m_stagingMemory);
        return false;
    }

    output << "P6\n" << extent.width << ' ' << extent.height << "\n255\n";
    const auto* pixels = static_cast<const unsigned char*>(mapped);
    for (uint32_t y = 0; y < extent.height; ++y) {
        const uint32_t sourceY = extent.height - 1 - y;
        for (uint32_t x = 0; x < extent.width; ++x) {
            const size_t index = (static_cast<size_t>(sourceY) * extent.width + x) * 4u;
            const std::array<unsigned char, 3> rgb = {pixels[index + 2], pixels[index + 1], pixels[index]};
            output.write(reinterpret_cast<const char*>(rgb.data()), 3);
        }
    }
    const bool ok = output.good();
    output.close();
    vkUnmapMemory(m_context->device(), m_stagingMemory);
    return ok;
}

} // namespace gfx
