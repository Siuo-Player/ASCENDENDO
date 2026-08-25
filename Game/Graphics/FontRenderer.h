#pragma once

#include <vulkan/vulkan.h>

#include "Graphics/VulkanContext.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace gfx {

class FontRendererGpu;

class FontRenderer {
public:
    FontRenderer();
    ~FontRenderer();

    FontRenderer(const FontRenderer&) = delete;
    FontRenderer& operator=(const FontRenderer&) = delete;

    bool init(VulkanContext* ctx, VkDescriptorSetLayout descriptorSetLayout,
              const std::string& ttfPath = "Game/Assets/Fonts/UIFont.ttf",
              float bakePixelHeight = 48.0f);
    void cleanup();

    void bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const;

    void drawText(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                  const char* text, float x, float baseline_y, float scale,
                  float r, float g, float b, float a) const;

    void drawTextCentered(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
                          const char* text, float cx, float baseline_y, float scale,
                          float r, float g, float b, float a) const;

    float textWidth(const char* text, float scale) const;
    bool isInitialized() const;

private:
    struct BakedChar {
        uint16_t x0, y0, x1, y1;
        float xoff, yoff, xadvance;
    };

    bool loadAndBakeFont(const std::string& ttfPath, float pixelHeight,
                         std::vector<uint8_t>& outAtlasPixels);

    static constexpr uint32_t ATLAS_W = 512;
    static constexpr uint32_t ATLAS_H = 512;
    static constexpr int FIRST_CHAR = 32;
    static constexpr int NUM_CHARS = 95;

    VulkanContext* m_ctx = nullptr;
    std::unique_ptr<FontRendererGpu> m_gpu;
    BakedChar m_chars[NUM_CHARS]{};
    float m_bakePixelHeight = 48.0f;
    bool m_initialized = false;
};

} // namespace gfx
