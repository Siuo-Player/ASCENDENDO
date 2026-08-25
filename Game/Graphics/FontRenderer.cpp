// =============================================================================
//  Game/Graphics/FontRenderer.cpp
//
//  Font baking, descriptor setup and text drawing. Vulkan atlas upload lives in
//  FontRendererGpu.cpp so the class stays below the source-size gate without
//  splitting unrelated responsibilities artificially.
// =============================================================================
#include "Graphics/FontRenderer.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/TextPipeline.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include <fstream>
#include <iostream>

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

bool FontRenderer::loadAndBakeFont(const std::string& ttfPath, float pixelHeight,
                                   std::vector<uint8_t>& outAtlasPixels) {
    std::ifstream file(ttfPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "[FontRenderer] Nao foi possivel abrir: " << ttfPath << "\n";
        return false;
    }

    const size_t size = static_cast<size_t>(file.tellg());
    file.seekg(0);
    std::vector<uint8_t> ttfBuffer(size);
    file.read(reinterpret_cast<char*>(ttfBuffer.data()), static_cast<std::streamsize>(size));
    file.close();

    outAtlasPixels.assign(static_cast<size_t>(ATLAS_W) * ATLAS_H, 0);

    std::vector<stbtt_bakedchar> baked(NUM_CHARS);
    const int ret = stbtt_BakeFontBitmap(ttfBuffer.data(), 0, pixelHeight,
                                          outAtlasPixels.data(), ATLAS_W, ATLAS_H,
                                          FIRST_CHAR, NUM_CHARS, baked.data());
    if (ret == 0) {
        std::cerr << "[FontRenderer] stbtt_BakeFontBitmap falhou (atlas demasiado pequeno?)\n";
        return false;
    }

    for (int i = 0; i < NUM_CHARS; ++i) {
        m_chars[i].x0 = baked[i].x0;
        m_chars[i].y0 = baked[i].y0;
        m_chars[i].x1 = baked[i].x1;
        m_chars[i].y1 = baked[i].y1;
        m_chars[i].xoff = baked[i].xoff;
        m_chars[i].yoff = baked[i].yoff;
        m_chars[i].xadvance = baked[i].xadvance;
    }
    m_bakePixelHeight = pixelHeight;
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
    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descPool) != VK_SUCCESS) return false;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;
    if (vkAllocateDescriptorSets(device, &allocInfo, &m_descSet) != VK_SUCCESS) return false;

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

    if (m_descPool) vkDestroyDescriptorPool(device, m_descPool, nullptr);
    if (m_sampler) vkDestroySampler(device, m_sampler, nullptr);
    if (m_imageView) vkDestroyImageView(device, m_imageView, nullptr);
    if (m_image) vkDestroyImage(device, m_image, nullptr);
    if (m_imageMemory) vkFreeMemory(device, m_imageMemory, nullptr);

    m_descPool = VK_NULL_HANDLE;
    m_sampler = VK_NULL_HANDLE;
    m_imageView = VK_NULL_HANDLE;
    m_image = VK_NULL_HANDLE;
    m_imageMemory = VK_NULL_HANDLE;
    m_initialized = false;
    m_ctx = nullptr;
}

void FontRenderer::bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const {
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
                            0, 1, &m_descSet, 0, nullptr);
}

void FontRenderer::drawText(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                            const char* text, float x, float baseline_y, float scale,
                            float r, float g, float b, float a) const {
    if (!m_initialized) return;

    const float invAtlasW = 1.0f / static_cast<float>(ATLAS_W);
    const float invAtlasH = 1.0f / static_cast<float>(ATLAS_H);
    float cursorX = x;

    for (const char* p = text; *p; ++p) {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c < FIRST_CHAR || c >= FIRST_CHAR + NUM_CHARS) {
            cursorX += scale * m_bakePixelHeight * 0.3f;
            continue;
        }

        const BakedChar& bc = m_chars[c - FIRST_CHAR];
        const float glyphW = static_cast<float>(bc.x1 - bc.x0);
        const float glyphH = static_cast<float>(bc.y1 - bc.y0);

        if (glyphW > 0.0f && glyphH > 0.0f) {
            const float left = cursorX + bc.xoff * scale;
            const float top = baseline_y - bc.yoff * scale;
            const float bottom = top - glyphH * scale;
            const float width = glyphW * scale;
            const float height = glyphH * scale;

            const float u0 = bc.x0 * invAtlasW;
            const float v0 = bc.y0 * invAtlasH;
            const float u1 = bc.x1 * invAtlasW;
            const float v1 = bc.y1 * invAtlasH;

            TextPushConstants pc{};
            pc.color[0] = r;
            pc.color[1] = g;
            pc.color[2] = b;
            pc.color[3] = a;
            pc.objPos[0] = left;
            pc.objPos[1] = bottom;
            pc.objSize[0] = width;
            pc.objSize[1] = height;
            pc.uv0[0] = u0;
            pc.uv0[1] = v0;
            pc.uv1[0] = u1;
            pc.uv1[1] = v1;
            pc.logicalRes[0] = 640.0f;
            pc.logicalRes[1] = 360.0f;

            vkCmdPushConstants(cmd, pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0, sizeof(TextPushConstants), &pc);
            vkCmdDraw(cmd, 6, 1, 0, 0);
        }

        cursorX += bc.xadvance * scale;
    }
}

void FontRenderer::drawTextCentered(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                                    const char* text, float cx, float baseline_y, float scale,
                                    float r, float g, float b, float a) const {
    const float w = textWidth(text, scale);
    drawText(cmd, pipelineLayout, text, cx - w * 0.5f, baseline_y, scale, r, g, b, a);
}

float FontRenderer::textWidth(const char* text, float scale) const {
    float width = 0.0f;
    for (const char* p = text; *p; ++p) {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c < FIRST_CHAR || c >= FIRST_CHAR + NUM_CHARS) {
            width += scale * m_bakePixelHeight * 0.3f;
            continue;
        }
        width += m_chars[c - FIRST_CHAR].xadvance * scale;
    }
    return width;
}

} // namespace gfx
