// =============================================================================
//  Game/Graphics/FontRenderer.cpp
//
//  @version 7.6
//
//  IMPORTANTE (mesma licao do bug de duplicate symbol do doctest):
//  STB_TRUETYPE_IMPLEMENTATION so pode ser definido AQUI, neste unico .cpp
//  de todo o projecto. Qualquer outro ficheiro que precise de stb_truetype.h
//  deve incluir SEM essa macro.
//
//  Fluxo de upload da textura (padrao standard Vulkan):
//    1. stb_truetype baka o atlas em RAM (greyscale, 1 canal, R8).
//    2. Staging buffer (host-visible) recebe os pixeis via memcpy.
//    3. VkImage (device-local, R8_UNORM) e criada.
//    4. Command buffer de uso unico: layout UNDEFINED->TRANSFER_DST,
//       vkCmdCopyBufferToImage, layout TRANSFER_DST->SHADER_READ_ONLY.
//    5. VkImageView + VkSampler + VkDescriptorSet apontam para a imagem.
// =============================================================================
#include "Graphics/FontRenderer.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/TextPipeline.h"   // TextPushConstants

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include <fstream>
#include <cstring>
#include <iostream>

namespace gfx {

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers Vulkan genericos (auto-contidos — nao dependem de Renderer.cpp)
// ─────────────────────────────────────────────────────────────────────────────

uint32_t FontRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags props) const {
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(m_ctx->physicalDevice(), &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; ++i) {
        if ((typeFilter & (1u << i)) &&
            (memProps.memoryTypes[i].propertyFlags & props) == props) {
            return i;
        }
    }
    return UINT32_MAX; // falha — chamador deve verificar
}

namespace {

// Comando de uso unico (aloca, grava, submete, espera, liberta). Usado apenas
// durante o upload do atlas (uma vez no arranque) — nao esta no caminho quente.
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

// ─────────────────────────────────────────────────────────────────────────────
//  Baking (CPU, stb_truetype)
// ─────────────────────────────────────────────────────────────────────────────

bool FontRenderer::loadAndBakeFont(const std::string& ttfPath, float pixelHeight,
                                    std::vector<uint8_t>& outAtlasPixels) {
    std::ifstream file(ttfPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "[FontRenderer] Nao foi possivel abrir: " << ttfPath << "\n";
        return false;
    }
    size_t size = (size_t)file.tellg();
    file.seekg(0);
    std::vector<uint8_t> ttfBuffer(size);
    file.read(reinterpret_cast<char*>(ttfBuffer.data()), (std::streamsize)size);
    file.close();

    outAtlasPixels.assign((size_t)ATLAS_W * ATLAS_H, 0);

    std::vector<stbtt_bakedchar> baked(NUM_CHARS);
    int ret = stbtt_BakeFontBitmap(ttfBuffer.data(), 0, pixelHeight,
                                    outAtlasPixels.data(), ATLAS_W, ATLAS_H,
                                    FIRST_CHAR, NUM_CHARS, baked.data());
    if (ret == 0) {
        std::cerr << "[FontRenderer] stbtt_BakeFontBitmap falhou (atlas demasiado pequeno?)\n";
        return false;
    }

    for (int i = 0; i < NUM_CHARS; ++i) {
        m_chars[i].x0 = baked[i].x0; m_chars[i].y0 = baked[i].y0;
        m_chars[i].x1 = baked[i].x1; m_chars[i].y1 = baked[i].y1;
        m_chars[i].xoff = baked[i].xoff; m_chars[i].yoff = baked[i].yoff;
        m_chars[i].xadvance = baked[i].xadvance;
    }
    m_bakePixelHeight = pixelHeight;
    return true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Upload GPU (imagem + view + sampler)
// ─────────────────────────────────────────────────────────────────────────────

bool FontRenderer::createAtlasImage(const std::vector<uint8_t>& pixels) {
    VkDevice device = m_ctx->device();
    VkDeviceSize imageSize = (VkDeviceSize)pixels.size();

    // ── Staging buffer (host-visible) ─────────────────────────────────────
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

    // ── VkImage (device-local, R8_UNORM) ──────────────────────────────────
    VkImageCreateInfo imgInfo{};
    imgInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgInfo.imageType     = VK_IMAGE_TYPE_2D;
    imgInfo.extent        = { (uint32_t)ATLAS_W, (uint32_t)ATLAS_H, 1 };
    imgInfo.mipLevels     = 1;
    imgInfo.arrayLayers   = 1;
    imgInfo.format        = VK_FORMAT_R8_UNORM;
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
    region.imageExtent       = { (uint32_t)ATLAS_W, (uint32_t)ATLAS_H, 1 };
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
    viewInfo.format   = VK_FORMAT_R8_UNORM;
    viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    if (vkCreateImageView(device, &viewInfo, nullptr, &m_imageView) != VK_SUCCESS) return false;

    // ── VkSampler (linear — suaviza a amostragem em qualquer escala) ──────
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter    = VK_FILTER_LINEAR;
    samplerInfo.minFilter    = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.borderColor  = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.maxAnisotropy = 1.0f;
    if (vkCreateSampler(device, &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS) return false;

    return true;
}

bool FontRenderer::createDescriptorSet(VkDescriptorSetLayout layout) {
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

// ─────────────────────────────────────────────────────────────────────────────
//  Ciclo de vida
// ─────────────────────────────────────────────────────────────────────────────

bool FontRenderer::init(VulkanContext* ctx, VkDescriptorSetLayout descriptorSetLayout,
                        const std::string& ttfPath, float bakePixelHeight) {
    if (m_initialized) return true;
    if (!ctx || !ctx->isInitialized()) return false;
    m_ctx = ctx;

    std::vector<uint8_t> atlasPixels;
    if (!loadAndBakeFont(ttfPath, bakePixelHeight, atlasPixels)) return false;
    if (!createAtlasImage(atlasPixels)) return false;
    if (!createDescriptorSet(descriptorSetLayout)) return false;

    m_initialized = true;
    return true;
}

void FontRenderer::cleanup() {
    if (!m_initialized || !m_ctx) return;
    VkDevice device = m_ctx->device();
    vkDeviceWaitIdle(device);

    if (m_descPool)   vkDestroyDescriptorPool(device, m_descPool, nullptr);
    if (m_sampler)    vkDestroySampler(device, m_sampler, nullptr);
    if (m_imageView)  vkDestroyImageView(device, m_imageView, nullptr);
    if (m_image)      vkDestroyImage(device, m_image, nullptr);
    if (m_imageMemory) vkFreeMemory(device, m_imageMemory, nullptr);

    m_descPool = VK_NULL_HANDLE; m_sampler = VK_NULL_HANDLE;
    m_imageView = VK_NULL_HANDLE; m_image = VK_NULL_HANDLE; m_imageMemory = VK_NULL_HANDLE;
    m_initialized = false;
    m_ctx = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Desenho
// ─────────────────────────────────────────────────────────────────────────────

void FontRenderer::bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const {
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &m_descSet, 0, nullptr);
}

void FontRenderer::drawText(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                            const char* text, float x, float baseline_y, float scale,
                            float r, float g, float b, float a) const {
    if (!m_initialized) return;

    float invAtlasW = 1.0f / (float)ATLAS_W;
    float invAtlasH = 1.0f / (float)ATLAS_H;
    float cx = x;

    for (const char* p = text; *p; ++p) {
        unsigned char c = (unsigned char)*p;
        if (c < FIRST_CHAR || c >= FIRST_CHAR + NUM_CHARS) {
            cx += scale * m_bakePixelHeight * 0.3f; // espaco/caractere desconhecido
            continue;
        }
        const BakedChar& bc = m_chars[c - FIRST_CHAR];

        float glyphW = (float)(bc.x1 - bc.x0);
        float glyphH = (float)(bc.y1 - bc.y0);

        if (glyphW > 0.0f && glyphH > 0.0f) {
            // stb: yoff e a distancia (Y-baixo) do baseline ao TOPO do glyph
            //      (negativa para a maioria das letras). xoff = deslocamento
            //      lateral do "cursor" ate ao canto esq do glyph.
            // Y-cima (mundo do jogo): topo_world = baseline_y - yoff*scale
            //                          fundo_world = topo_world - glyphH*scale
            float left = cx + bc.xoff * scale;
            float top  = baseline_y - bc.yoff * scale;
            float bottom = top - glyphH * scale;
            float width  = glyphW * scale;
            float height = glyphH * scale;

            float u0 = bc.x0 * invAtlasW, v0 = bc.y0 * invAtlasH; // topo-esq
            float u1 = bc.x1 * invAtlasW, v1 = bc.y1 * invAtlasH; // fundo-dir

            TextPushConstants pc{};
            pc.color[0] = r; pc.color[1] = g; pc.color[2] = b; pc.color[3] = a;
            pc.objPos[0]  = left;  pc.objPos[1]  = bottom;
            pc.objSize[0] = width; pc.objSize[1] = height;
            pc.uv0[0] = u0; pc.uv0[1] = v0;
            pc.uv1[0] = u1; pc.uv1[1] = v1;
            pc.logicalRes[0] = 640.0f; pc.logicalRes[1] = 360.0f; // config::LOGICAL_*

            vkCmdPushConstants(cmd, pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0, sizeof(TextPushConstants), &pc);
            vkCmdDraw(cmd, 6, 1, 0, 0);
        }

        cx += bc.xadvance * scale;
    }
}

void FontRenderer::drawTextCentered(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                                    const char* text, float cx, float baseline_y, float scale,
                                    float r, float g, float b, float a) const {
    float w = textWidth(text, scale);
    drawText(cmd, pipelineLayout, text, cx - w * 0.5f, baseline_y, scale, r, g, b, a);
}

float FontRenderer::textWidth(const char* text, float scale) const {
    float w = 0.0f;
    for (const char* p = text; *p; ++p) {
        unsigned char c = (unsigned char)*p;
        if (c < FIRST_CHAR || c >= FIRST_CHAR + NUM_CHARS) {
            w += scale * m_bakePixelHeight * 0.3f;
            continue;
        }
        w += m_chars[c - FIRST_CHAR].xadvance * scale;
    }
    return w;
}

} // namespace gfx
