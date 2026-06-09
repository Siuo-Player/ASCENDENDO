#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace gfx {

class VulkanContext;
class Swapchain;
class RenderPass;

struct PushConstants {
    float color[4];
    float camPos[2];
    float objPos[2];
    float objSize[2];
    float logicalRes[2]; // INJETADO DAQUI PARA O SHADER
};

class Pipeline {
public:
    Pipeline() = default;
    ~Pipeline() { cleanup(); }

    bool init(VulkanContext* ctx, Swapchain* swapchain, RenderPass* renderPass);
    void cleanup();

    VkPipeline       handle() const { return m_pipeline; }
    VkPipelineLayout layout() const { return m_layout;   }
    bool             isInitialized() const { return m_initialized; }

private:
    VkShaderModule createShaderModule(const std::vector<char>& code);
    std::vector<char> readFile(const std::string& filename);

    VulkanContext* m_ctx         = nullptr;
    VkPipelineLayout m_layout      = VK_NULL_HANDLE;
    VkPipeline       m_pipeline    = VK_NULL_HANDLE;
    bool             m_initialized = false;
};

} // namespace gfx