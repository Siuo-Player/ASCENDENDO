#pragma once
// =============================================================================
//  Game/Graphics/SpritePipeline.h
//
//  @version 8.2
//  Pipeline Vulkan dedicada para sprites (pixel art) — estrutura identica
//  a TextPipeline (descriptor set + push constants), mas com sampler
//  NEAREST (preserva arestas nitidas dos pixeis) em vez de LINEAR.
// =============================================================================
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace gfx {

class VulkanContext;
class Swapchain;
class RenderPass;

// Push constants do shader sprite.vert/sprite.frag. Todos os campos
// float/float[N] — sem padding escondido pelo compilador C++.
struct SpritePushConstants {
    float tint[4];        // offset  0..16
    float objPos[2];       // offset 16..24
    float objSize[2];      // offset 24..32
    float camPos[2];       // offset 32..40
    float logicalRes[2];  // offset 40..48
    float flipX;           // offset 48..52
    float _pad;            // offset 52..56
};                          // total 56 bytes

class SpritePipeline {
public:
    SpritePipeline()  = default;
    ~SpritePipeline() { cleanup(); }

    bool init(VulkanContext* ctx, Swapchain* swapchain, RenderPass* renderPass);
    void cleanup();

    VkPipeline            handle()             const { return m_pipeline; }
    VkPipelineLayout      layout()             const { return m_layout; }
    VkDescriptorSetLayout descriptorSetLayout() const { return m_descriptorSetLayout; }
    bool                  isInitialized()       const { return m_initialized; }

private:
    VkShaderModule createShaderModule(const std::vector<char>& code);
    std::vector<char> readFile(const std::string& filename);

    VulkanContext*         m_ctx                 = nullptr;
    VkDescriptorSetLayout  m_descriptorSetLayout = VK_NULL_HANDLE;
    VkPipelineLayout       m_layout              = VK_NULL_HANDLE;
    VkPipeline             m_pipeline            = VK_NULL_HANDLE;
    bool                   m_initialized         = false;
};

} // namespace gfx
