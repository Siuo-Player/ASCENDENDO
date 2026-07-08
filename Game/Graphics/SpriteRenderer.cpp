// =============================================================================
//  Game/Graphics/SpriteRenderer.cpp
//
//  @version 8.2
//
//  IMPORTANTE (mesma licao do doctest/FontRenderer): STB_IMAGE_IMPLEMENTATION
//  so pode ser definido AQUI, neste unico .cpp de todo o projecto.
//
//  Upload da textura: identico ao padrao usado em FontRenderer.cpp (staging
//  buffer -> VkImage -> barriers -> copy -> barrier final), reaproveitando
//  o mesmo desenho ja validado. Unica diferenca de fundo: formato
//  VK_FORMAT_R8G8B8A8_UNORM (4 canais, imagem a cores) em vez de R8_UNORM
//  (1 canal, atlas de fonte), e sampler NEAREST em vez de LINEAR.
// =============================================================================
#include "Graphics/SpriteRenderer.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/SpritePipeline.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb/stb_image.h"

#include <cstring>
#include <iostream>

namespace gfx {

uint32_t SpriteRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) const {
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
    if (vkAllocateCommandBuffers(device, &allocInfo, &cmd) != VK_SUCCESS) return VK_NULL_HANDLE;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);
    return cmd;
}

void endOneTimeCommands(VkDevice device, VkCommandPool pool, VkQueue queue, VkCommandBuffer cmd) {
    vkEndCommandBuffer(cmd);
    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &cmd;
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
    vkFreeCommandBuffers(device, pool, 1, &cmd);
}

} // namespace anonimo

bool SpriteRenderer::loadPNG(const std::string& path, std::vector<unsigned char>& outPixels) {
    int w, h, channels;
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4); // forcar RGBA
    if (!data) {
        std::cerr << "[SpriteRenderer] Falha ao carregar " << path
                  << ": " << stbi_failure_reason() << "\n";
        return false;
    }
    m_width  = w;
    m_height = h;
    outPixels.assign(data, data + (size_t)w * h * 4);
    stbi_image_free(data);
    return true;
}

bool SpriteRenderer::createTextureImage(const std::vector<unsigned char>& pixels) {
    VkDevice device = m_ctx->device();
    VkDeviceSize imageSize = (VkDeviceSize)pixels.size();

    // ── Staging buffer ────────────────────────────────────────────────────
    VkBuffer       stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory stagingMemory = VK_NULL_HANDLE;

    VkBufferCreateInfo bufInfo{};
    bufInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufInfo.size        = imageSize;
    bufInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(device, &bufInfo, nullptr, &stagingBuffer) != VK_SUCCESS) return false;

    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memReq);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits,
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
    vkMapMemory(device, stagingMemory, 0, imageSize, 0, &mapped);
    std::memcpy(mapped, pixels.data(), (size_t)imageSize);
    vkUnmapMemory(device, stagingMemory);

    // ── VkImage (RGBA8, device-local) ─────────────────────────────────────
    VkImageCreateInfo imgInfo{};
    imgInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType     = VK_IMAGE_TYPE_2D;
    imgInfo.extent        = { (uint32_t)m_width, (uint32_t)m_height, 1 };
    imgInfo.mipLevels     = 1;
    imgInfo.arrayLayers   = 1;
    imgInfo.format        = VK_FORMAT_R8G8B8A8_UNORM;
    imgInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imgInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imgInfo.samples       = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(device, &imgInfo, nullptr, &m_image) != VK_SUCCESS) {
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        return false;
    }

    VkMemoryRequirements imgMemReq{};
    vkGetImageMemoryRequirements(device, m_image, &imgMemReq);
    VkMemoryAllocateInfo imgAllocInfo{};
    imgAllocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imgAllocInfo.allocationSize  = imgMemReq.size;
    imgAllocInfo.memoryTypeIndex = findMemoryType(imgMemReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (imgAllocInfo.memoryTypeIndex == UINT32_MAX ||
        vkAllocateMemory(device, &imgAllocInfo, nullptr, &m_imageMemory) != VK_SUCCESS) {
        vkDestroyImage(device, m_image, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        return false;
    }
    vkBindImageMemory(device, m_image, m_imageMemory, 0);

    // ── Comando de uso unico: transicoes de layout + copia ────────────────
    VkCommandPoolCreateInfo poolCI{};
    poolCI.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolCI.flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    poolCI.queueFamilyIndex = m_ctx->graphicsFamily();
    VkCommandPool tempPool = VK_NULL_HANDLE;
    vkCreateCommandPool(device, &poolCI, nullptr, &tempPool);

    VkCommandBuffer cmd = beginOneTimeCommands(device, tempPool);
    if (cmd == VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, tempPool, nullptr);
        vkDestroyBuffer(device, stagingBuffer, nullptr);
        vkFreeMemory(device, stagingMemory, nullptr);
        return false;
    }

    VkImageMemoryBarrier toTransfer{};
    toTransfer.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toTransfer.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    toTransfer.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.image               = m_image;
    toTransfer.subresourceRange    = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    toTransfer.srcAccessMask       = 0;
    toTransfer.dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &toTransfer);

    VkBufferImageCopy region{};
    region.bufferOffset      = 0;
    region.bufferRowLength   = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource  = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageOffset       = { 0, 0, 0 };
    region.imageExtent       = { (uint32_t)m_width, (uint32_t)m_height, 1 };
    vkCmdCopyBufferToImage(cmd, stagingBuffer, m_image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    VkImageMemoryBarrier toShaderRead = toTransfer;
    toShaderRead.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toShaderRead.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    toShaderRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    toShaderRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, nullptr, 0, nullptr, 1, &toShaderRead);

    endOneTimeCommands(device, tempPool, m_ctx->graphicsQueue(), cmd);
    vkDestroyCommandPool(device, tempPool, nullptr);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingMemory, nullptr);

    // ── VkImageView ────────────────────────────────────────────────────────
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image    = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format   = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) return false;

    // ── VkSampler: NEAREST — CRITICO para pixel art. LINEAR (usado no
    // FontRenderer) borraria as arestas nitidas dos pixeis do sprite. ─────
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter    = VK_FILTER_NEAREST;
    samplerInfo.minFilter    = VK_FILTER_NEAREST;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.borderColor  = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.maxAnisotropy = 1.0f;
    if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) return false;

    return true;
}

bool SpriteRenderer::createDescriptorSet(VkDescriptorSetLayout layout) {
    VkDevice device = m_ctx->device();

    VkDescriptorPoolSize poolSize{};
    poolSize.type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = &poolSize;
    poolInfo.maxSets       = 1;
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descPool) != VK_SUCCESS) return false;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool     = m_descPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts        = &layout;
    if (vkAllocateDescriptorSets(device, &allocInfo, &m_descSet) != VK_SUCCESS) return false;

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imgInfo.imageView   = m_imageView;
    imgInfo.sampler     = m_sampler;

    VkWriteDescriptorSet write{};
    write.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet          = m_descSet;
    write.dstBinding      = 0;
    write.descriptorCount = 1;
    write.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo      = &imgInfo;
    vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);

    return true;
}

bool SpriteRenderer::init(VulkanContext* ctx, VkDescriptorSetLayout descriptorSetLayout,
                          const std::string& pngPath) {
    if (m_initialized) return true;
    if (!ctx || !ctx->isInitialized()) return false;
    m_ctx = ctx;

    std::vector<unsigned char> pixels;
    if (!loadPNG(pngPath, pixels)) return false;
    if (!createTextureImage(pixels)) return false;
    if (!createDescriptorSet(descriptorSetLayout)) return false;

    m_initialized = true;
    return true;
}

void SpriteRenderer::cleanup() {
    if (!m_initialized || !m_ctx) return;
    VkDevice device = m_ctx->device();
    vkDeviceWaitIdle(device);

    if (m_descPool)    vkDestroyDescriptorPool(device, m_descPool, nullptr);
    if (m_sampler)     vkDestroySampler(device, m_sampler, nullptr);
    if (m_imageView)   vkDestroyImageView(device, m_imageView, nullptr);
    if (m_image)       vkDestroyImage(device, m_image, nullptr);
    if (m_imageMemory) vkFreeMemory(device, m_imageMemory, nullptr);

    m_descPool = VK_NULL_HANDLE; m_sampler = VK_NULL_HANDLE;
    m_imageView = VK_NULL_HANDLE; m_image = VK_NULL_HANDLE; m_imageMemory = VK_NULL_HANDLE;
    m_initialized = false;
    m_ctx = nullptr;
}

void SpriteRenderer::bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const {
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &m_descSet, 0, nullptr);
}

void SpriteRenderer::draw(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                          float x, float y, float w, float h, bool flipX,
                          float r, float g, float b, float a,
                          float camPosX, float camPosY) const {
    if (!m_initialized) return;

    SpritePushConstants pc{};
    pc.tint[0] = r; pc.tint[1] = g; pc.tint[2] = b; pc.tint[3] = a;
    pc.objPos[0]  = x; pc.objPos[1]  = y;
    pc.objSize[0] = w; pc.objSize[1] = h;
    pc.camPos[0]  = camPosX; pc.camPos[1] = camPosY;
    pc.logicalRes[0] = 640.0f; pc.logicalRes[1] = 360.0f; // config::LOGICAL_*
    pc.flipX = flipX ? 1.0f : 0.0f;

    vkCmdPushConstants(cmd, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(SpritePushConstants), &pc);
    vkCmdDraw(cmd, 6, 1, 0, 0);
}

} // namespace gfx
