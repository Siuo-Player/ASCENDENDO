#pragma once

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

namespace gfx {

class VulkanContext;
class SpriteRendererGpu;

class SpriteRenderer {
public:
    SpriteRenderer();
    ~SpriteRenderer();

    SpriteRenderer(const SpriteRenderer&) = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;

    bool init(VulkanContext* ctx, VkDescriptorSetLayout descriptorSetLayout,
              const std::string& pngPath = "Game/Assets/Sprites/personagem.png");
    void cleanup();

    void bind(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout) const;

    void draw(VkCommandBuffer cmd, VkPipelineLayout pipelineLayout,
              float x, float y, float w, float h, bool flipX,
              float r, float g, float b, float a,
              float camPosX, float camPosY) const;

    int width() const { return m_width; }
    int height() const { return m_height; }
    bool isInitialized() const { return m_initialized; }

private:
    bool loadPNG(const std::string& path, std::vector<unsigned char>& outPixels);

    int m_width = 0;
    int m_height = 0;
    bool m_initialized = false;
    std::unique_ptr<SpriteRendererGpu> m_gpu;
};

} // namespace gfx
