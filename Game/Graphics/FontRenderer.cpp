#include "Graphics/FontRenderer.h"
#include "Graphics/FontRendererGpu.h"
#include "Graphics/VulkanContext.h"
#include "Graphics/TextPipeline.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include <fstream>
#include <iostream>

namespace gfx {

FontRenderer::FontRenderer() : m_gpu(std::make_unique<FontRendererGpu>()) {}

FontRenderer::~FontRenderer() { cleanup(); }

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
    if (!file) {
        std::cerr << "[FontRenderer] Nao foi possivel ler: " << ttfPath << "\n";
        return false;
    }

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

bool FontRenderer::init(VulkanContext* ctx, VkDescriptorSetLayout descriptorSetLayout,
                        const std::string& ttfPath, float bakePixelHeight) {
    if (m_initialized) return true;
    if (!ctx || !ctx->isInitialized()) return false;

    m_ctx = ctx;
    std::vector<uint8_t> atlasPixels;
    if (!loadAndBakeFont(ttfPath, bakePixelHeight, atlasPixels) ||
        !m_gpu->init(ctx, descriptorSetLayout, atlasPixels, ATLAS_W, ATLAS_H)) {
        cleanup();
        return false;
    }

    m_initialized = true;
    return true;
}

void FontRenderer::cleanup() {
    if (m_gpu) m_gpu->cleanup();
    m_initialized = false;
    m_ctx = nullptr;
}

void FontRenderer::bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const {
    if (!m_initialized) return;
    m_gpu->bind(cmd, pipelineLayout);
}

void FontRenderer::drawText(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                            const char* text, float x, float baseline_y, float scale,
                            float r, float g, float b, float a) const {
    if (!m_initialized || !text) return;

    const float invAtlasW = 1.0f / static_cast<float>(ATLAS_W);
    const float invAtlasH = 1.0f / static_cast<float>(ATLAS_H);
    float cursorX = x;

    for (const char* p = text; *p; ++p) {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c < FIRST_CHAR || c >= FIRST_CHAR + NUM_CHARS) {
            cursorX += scale * m_bakePixelHeight * 0.3f;
            continue;
        }

        const BakedChar& glyph = m_chars[c - FIRST_CHAR];
        const float width = static_cast<float>(glyph.x1 - glyph.x0);
        const float height = static_cast<float>(glyph.y1 - glyph.y0);
        if (width > 0.0f && height > 0.0f) {
            const float left = cursorX + glyph.xoff * scale;
            const float top = baseline_y - glyph.yoff * scale;
            const float bottom = top - height * scale;

            TextPushConstants pc{};
            pc.color[0] = r; pc.color[1] = g; pc.color[2] = b; pc.color[3] = a;
            pc.objPos[0] = left; pc.objPos[1] = bottom;
            pc.objSize[0] = width * scale; pc.objSize[1] = height * scale;
            pc.uv0[0] = glyph.x0 * invAtlasW; pc.uv0[1] = glyph.y0 * invAtlasH;
            pc.uv1[0] = glyph.x1 * invAtlasW; pc.uv1[1] = glyph.y1 * invAtlasH;
            pc.logicalRes[0] = 640.0f; pc.logicalRes[1] = 360.0f;

            vkCmdPushConstants(cmd, pipelineLayout,
                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0, sizeof(TextPushConstants), &pc);
            vkCmdDraw(cmd, 6, 1, 0, 0);
        }
        cursorX += glyph.xadvance * scale;
    }
}

void FontRenderer::drawTextCentered(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                                    const char* text, float cx, float baseline_y, float scale,
                                    float r, float g, float b, float a) const {
    const float width = textWidth(text, scale);
    drawText(cmd, pipelineLayout, text, cx - width * 0.5f, baseline_y,
             scale, r, g, b, a);
}

float FontRenderer::textWidth(const char* text, float scale) const {
    if (!text) return 0.0f;

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

bool FontRenderer::isInitialized() const {
    return m_initialized && m_gpu && m_gpu->isInitialized();
}

} // namespace gfx
