#include "Graphics/SpriteRenderer.h"
#include "Graphics/SpriteRendererGpu.h"
#include "Graphics/SpritePipeline.h"
#include "Graphics/VulkanContext.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb/stb_image.h"

#include <iostream>

namespace gfx {

SpriteRenderer::SpriteRenderer() : m_gpu(std::make_unique<SpriteRendererGpu>()) {}

SpriteRenderer::~SpriteRenderer() { cleanup(); }

bool SpriteRenderer::loadPNG(const std::string& path, std::vector<unsigned char>& outPixels) {
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::cerr << "[SpriteRenderer] Falha ao carregar " << path
                  << ": " << stbi_failure_reason() << "\n";
        return false;
    }

    m_width = width;
    m_height = height;
    outPixels.assign(data, data + static_cast<size_t>(width) * height * 4);
    stbi_image_free(data);
    return true;
}

bool SpriteRenderer::init(VulkanContext* ctx, VkDescriptorSetLayout descriptorSetLayout,
                          const std::string& pngPath) {
    if (m_initialized) return true;
    if (!ctx || !ctx->isInitialized()) return false;

    std::vector<unsigned char> pixels;
    if (!loadPNG(pngPath, pixels) ||
        !m_gpu->init(ctx, descriptorSetLayout, pixels,
                     static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height))) {
        cleanup();
        return false;
    }

    m_initialized = true;
    return true;
}

void SpriteRenderer::cleanup() {
    if (m_gpu) m_gpu->cleanup();
    m_initialized = false;
}

void SpriteRenderer::bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const {
    if (!m_initialized) return;
    m_gpu->bind(cmd, pipelineLayout);
}

void SpriteRenderer::draw(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                          float x, float y, float w, float h, bool flipX,
                          float r, float g, float b, float a,
                          float camPosX, float camPosY) const {
    if (!m_initialized) return;

    SpritePushConstants pc{};
    pc.tint[0] = r; pc.tint[1] = g; pc.tint[2] = b; pc.tint[3] = a;
    pc.objPos[0] = x; pc.objPos[1] = y;
    pc.objSize[0] = w; pc.objSize[1] = h;
    pc.camPos[0] = camPosX; pc.camPos[1] = camPosY;
    pc.logicalRes[0] = 640.0f; pc.logicalRes[1] = 360.0f;
    pc.flipX = flipX ? 1.0f : 0.0f;

    vkCmdPushConstants(cmd, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(SpritePushConstants), &pc);
    vkCmdDraw(cmd, 6, 1, 0, 0);
}

} // namespace gfx
