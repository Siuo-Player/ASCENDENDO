#pragma once
// =============================================================================
//  Game/Graphics/TextPipeline.h
//
//  @version 7.6
//  Pipeline Vulkan DEDICADA para texto (glyphs TTF via FontRenderer).
//  Deliberadamente SEPARADA de Pipeline.h/.cpp (que continua a servir os
//  retangulos solidos: jogador, plataformas, FLAG, decoracoes de UI).
//  Razao: zero risco para a pipeline existente/testada. Ambas partilham
//  o mesmo RenderPass e podem alternar dentro do mesmo command buffer
//  (vkCmdBindPipeline muda a pipeline activa; viewport/scissor dinamicos
//  mantem-se validos entre pipelines compativeis).
// =============================================================================
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace gfx {

class VulkanContext;
class Swapchain;
class RenderPass;

// Push constants do shader text.vert/text.frag — TODOS os campos sao float
// ou float[N], para que o layout std430-like do Vulkan corresponda
// exactamente ao struct GLSL sem padding escondido pelo compilador C++.
struct TextPushConstants {
    float color[4];       // offset  0..16
    float objPos[2];      // offset 16..24
    float objSize[2];     // offset 24..32
    float uv0[2];          // offset 32..40  (s0, t0)
    float uv1[2];          // offset 40..48  (s1, t1)
    float logicalRes[2];  // offset 48..56
};                         // total 56 bytes

class TextPipeline {
public:
    TextPipeline()  = default;
    ~TextPipeline() { cleanup(); }

    bool init(VulkanContext* ctx, Swapchain* swapchain, RenderPass* renderPass);
    void cleanup();

    VkPipeline            handle()              const { return m_pipeline; }
    VkPipelineLayout      layout()              const { return m_layout; }
    VkDescriptorSetLayout descriptorSetLayout()  const { return m_descriptorSetLayout; }
    bool                  isInitialized()        const { return m_initialized; }

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
